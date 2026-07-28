#include "pw_internal.h"

#include <linux/seq_file.h>
#include <linux/sched/cputime.h>
#include <linux/mm.h>
// #include <linux/task_io_accounting.h>
#include <linux/task_io_accounting_ops.h>

static int pi_open(struct inode *, struct file *);
static int pi_show(struct seq_file *, void *);
static int pi_release(struct inode *, struct file *);
static const char *task_state_name(char state);
static const char *pi_policy_name(unsigned int policy);
static unsigned long pages_to_kib(unsigned long pages);
static void show_task_vmas(struct seq_file *m, struct mm_struct *mm);
static char *get_task_cmdline(struct task_struct *tsk);
static char *get_task_envs(struct task_struct *tsk);
static int collect_process_io(struct task_struct *tsk, struct task_io_accounting *result);

const struct proc_ops pi_ops = {
    .proc_open = pi_open,
    .proc_read_iter = seq_read_iter,
    .proc_lseek = seq_lseek,
    .proc_release = pi_release,
};

static int pi_open(struct inode *inode, struct file *filp)
{
    int err;
    struct task_struct *tsk;

    mutex_lock(&watched_pid_lock);
    if (!watched_pid || !(tsk = get_pid_task(watched_pid, PIDTYPE_PID))) {
        mutex_unlock(&watched_pid_lock);
        pr_err("The watched_pid was not found\n");
        return -ESRCH;
    }
    mutex_unlock(&watched_pid_lock);

    err = single_open(filp, pi_show, tsk);
    if (err)
        put_task_struct(tsk);

    return err;
}

static int pi_show(struct seq_file *m, void *data)
{
    char comm[TASK_COMM_LEN];
    struct task_struct *tsk = m->private;
    const struct cred *cred = get_task_cred(tsk);
    struct user_namespace *user_ns = seq_user_ns(m);
    u64 start_ns = tsk->start_boottime;
    u64 now_ns = ktime_get_boottime_ns();
    u64 elapsed_ns = now_ns >= start_ns ? now_ns - start_ns : 0;
    char state = task_state_to_char(tsk);
    u64 user_tims_ns, system_time_ns, total_time_ns;
    struct mm_struct *mm;
    char *cmdline, *envs;
    struct task_io_accounting ioac;

    if (!tsk)
        return -ESRCH;

    cmdline = get_task_cmdline(tsk);
    envs = get_task_envs(tsk);

    get_task_comm(comm, tsk);
    
    seq_printf(m, "[identity]\n");
    seq_printf(m, "pid: %d\n", task_pid_nr(tsk));
    seq_printf(m, "tgid: %d\n", task_tgid_nr(tsk));
    seq_printf(m, "comm: %s\n", comm);
    seq_printf(m, "uid: %d\n", from_kuid_munged(user_ns, cred->uid));
    seq_printf(m, "gid: %d\n", from_kgid_munged(user_ns, cred->gid));
    seq_printf(m, "suid: %d\n", from_kuid_munged(user_ns, cred->suid));
    seq_printf(m, "sgid: %d\n", from_kgid_munged(user_ns, cred->sgid));
    seq_printf(m, "euid: %d\n", from_kuid_munged(user_ns, cred->euid));
    seq_printf(m, "egid: %d\n", from_kgid_munged(user_ns, cred->egid));
    // seq_printf(m, "start_time_ns: %lld\n", tsk->start_time);
    seq_printf(m, "start_boottime_ns(): %lld\n", tsk->start_boottime);
    seq_printf(m, "elapsed_ms: %lld\n", div_u64(elapsed_ns, NSEC_PER_MSEC));
    put_cred(cred);

    seq_printf(m, "\n[state]\n");
    seq_printf(m, "state: %c\n", state);
    seq_printf(m, "state_name: %s\n", task_state_name(state));
    seq_printf(m, "exit_state: %d\n", tsk->exit_state);
    seq_printf(m, "exit_code: %d\n", tsk->exit_code);
    seq_printf(m, "exit_signal: %d\n", tsk->exit_signal);
    seq_printf(m, "threads: %d\n", tsk->signal->nr_threads);
    seq_printf(m, "on_cpu: %d\n", tsk->on_cpu);

    seq_printf(m, "\n[relations]\n");
    seq_printf(m, "parent_pid: %d\n", task_tgid_nr(tsk->parent));
    seq_printf(m, "real_parent_pid: %d\n", task_ppid_nr(tsk));
    seq_printf(m, "group_leader_pid: %d\n", task_tgid_nr(tsk->group_leader));
    seq_printf(m, "process_group_id: %d\n", task_pgrp_nr(tsk));
    seq_printf(m, "session_id: %d\n", task_session_nr_ns(tsk, &init_pid_ns));

    task_cputime_adjusted(tsk, &user_tims_ns, &system_time_ns);
    total_time_ns = user_tims_ns + system_time_ns;
    seq_printf(m, "\n[cpu]\n");
    seq_printf(m, "user_time_ns: %lld\n", user_tims_ns);
    seq_printf(m, "system_time_ns: %lld\n", system_time_ns);
    seq_printf(m, "total_time_ns: %lld\n", total_time_ns);
    seq_printf(m, "last_cpu: %d\n", task_cpu(tsk));
    seq_printf(m, "voluntary_context_switches: %ld\n", READ_ONCE(tsk->nvcsw));
    seq_printf(m, "nonvoluntary_context_switches: %ld\n", READ_ONCE(tsk->nivcsw));

    seq_printf(m, "\n[scheduler]\n");
    seq_printf(m, "policy_id: %d\n", tsk->policy);
    seq_printf(m, "policy: %s\n", pi_policy_name(tsk->policy));
    seq_printf(m, "priority: %d\n", tsk->prio);
    seq_printf(m, "static_priority: %d\n", tsk->static_prio);
    seq_printf(m, "normal_priority: %d\n", tsk->normal_prio);
    seq_printf(m, "rt_priority: %d\n", tsk->rt_priority);
    seq_printf(m, "nice: %d\n", task_nice(tsk));

    mm = get_task_mm(tsk);
    if (mm) {
        mmap_read_lock(mm);
        seq_printf(m, "\n[memory]\n");
        seq_printf(m, "total_vm_kb: %lu\n", pages_to_kib(mm->total_vm));
        seq_printf(m, "data_vm_kb: %lu\n", pages_to_kib(mm->data_vm));
        seq_printf(m, "exec_vm_kb: %lu\n", pages_to_kib(mm->exec_vm));
        seq_printf(m, "stack_vm_kb: %lu\n", pages_to_kib(mm->stack_vm));
        seq_printf(m, "rss_kb: %lu\n", pages_to_kib(get_mm_rss(mm)));
        seq_printf(m, "file_rss_kb: %lu\n", pages_to_kib(get_mm_counter(mm, MM_FILEPAGES)));
        seq_printf(m, "anon_rss_kb: %lu\n", pages_to_kib(get_mm_counter(mm, MM_ANONPAGES)));
        seq_printf(m, "shared_rss_kb: %lu\n", pages_to_kib(get_mm_counter(mm, MM_SHMEMPAGES)));
        seq_printf(m, "start_code: %lx, end_code: %lx, size: %lu\n", mm->start_code, mm->end_code, mm->end_code - mm->start_code);
        seq_printf(m, "start_data: %lx, end_data: %lx, size: %lu\n", mm->start_data, mm->end_data, mm->end_data - mm->start_data);
        seq_printf(m, "start_brk: %lx, brk: %lx, size: %lu\n", mm->start_brk, mm->brk, mm->brk - mm->start_brk);
        seq_printf(m, "start stack: %lx\n", mm->start_stack);
        seq_printf(m, "arg_start: %lx, arg_end: %lx, size: %lu\n", mm->arg_start, mm->arg_end, mm->arg_end - mm->arg_start);
        seq_printf(m, "env_start: %lx, env_end: %lx, size: %lu\n", mm->env_start, mm->env_end, mm->env_end - mm->env_start);
        if (cmdline) {
            seq_printf(m, "cmdline: %s\n", cmdline);
            kvfree(cmdline);
        }
        if (envs) {
            seq_printf(m, "environs: %s\n", envs);
            kvfree(envs);
        }
        show_task_vmas(m, mm);

        mmap_read_unlock(mm);
        mmput(mm);
    }
    
    if (!collect_process_io(tsk, &ioac)) {
        seq_printf(m, "\n[io]\n");
        seq_printf(m, "read_bytes: %llu\n", ioac.read_bytes);
        seq_printf(m, "write_bytes: %llu\n", ioac.write_bytes);
        seq_printf(m, "cancelled_write_bytes: %llu\n", ioac.cancelled_write_bytes);
        seq_printf(m, "read_syscalls: %llu\n", ioac.syscr);
        seq_printf(m, "write_syscalls: %llu\n", ioac.syscw);
    }

    seq_printf(m, "\n[signals]\n");
    seq_printf(m, "pending: %016llx\n", *(u64 *)&tsk->pending.signal.sig[0]);
    seq_printf(m, "blocked: %016llx\n", *(u64 *)&tsk->blocked.sig[0]);
    // seq_printf(m, "ignored: \n");
    // seq_printf(m, "caught: \n");

    return 0;
}

static int pi_release(struct inode *inode, struct file *filp)
{
    struct seq_file *m = filp->private_data;
    struct task_struct *tsk = m->private;

    if (tsk)
        put_task_struct(tsk);

    return single_release(inode, filp);
}

static const char *task_state_name(char state)
{
    switch (state) {
    case 'R': return "running";
    case 'S': return "sleeping";
    case 'D': return "uninterruptible";
    case 'T': return "stopped";
    case 't': return "tracing";
    case 'X': return "dead";
    case 'Z': return "zombie";
    case 'P': return "parked";
    case 'I': return "idle";
    default: return "unknown";
    }
}

static const char *pi_policy_name(unsigned int policy)
{
    switch (policy) {
    case SCHED_NORMAL: return "normal";
    case SCHED_FIFO: return "fifo";
    case SCHED_RR: return "round_robin";
    case SCHED_BATCH: return "batch";
    case SCHED_IDLE: return "idle";
    case SCHED_DEADLINE: return "deadline";
    default: return "unknown";
    }
}

static unsigned long pages_to_kib(unsigned long pages)
{
    return pages << (PAGE_SHIFT  - 10);
}

static void show_task_vmas(struct seq_file *m, struct mm_struct *mm)
{
    struct vm_area_struct *vma;
    struct vma_iterator vmi;

    vma_iter_init(&vmi, mm, 0);
    for_each_vma(vmi, vma) {
        seq_printf(m, "%016lx - %016lx %c%c%c%c %8lu KB ",
            vma->vm_start, vma->vm_end, 
            vma->vm_flags & VM_READ ? 'r' : '-',
            vma->vm_flags & VM_WRITE ? 'w' : '-',
            vma->vm_flags & VM_EXEC ? 'x' : '-',
            vma->vm_flags & VM_SHARED ? 's' : 'p',
            (vma->vm_end - vma->vm_start) >> 10
        );
        if (vma->vm_file) {
            seq_file_path(m, vma->vm_file, "\n\t");
            seq_putc(m, '\n');
        } else
            seq_puts(m, "[anonymous]\n");
    }

}

static void *copy_task_text(struct task_struct *tsk, unsigned long addr, int len)
{
    int copied;
    void *buffer;
    if (len <= 0)
        return NULL;

    buffer = kvmalloc(len + 1, GFP_KERNEL);
    if (!buffer)
        return NULL;
    copied = access_process_vm(tsk, addr, buffer, len, FOLL_FORCE);
    if (copied != len) {
        kvfree(buffer);
        return NULL;
    }
    ((char *)buffer)[len] = '\0';

    return buffer;
}

static void convert_strings(void *buf, size_t size, char delimiter)
{
    size_t i = 0;
    unsigned char *p = (unsigned char *)buf;
    if (!buf || size == 0)
        return;
    while (i < size) {
        if (p[i] == 0)
            p[i] = delimiter;
        i++;
    }
}

static char *get_task_cmdline(struct task_struct *tsk)
{
    struct mm_struct *mm = get_task_mm(tsk);
    void *buf;

    if (!mm)
        return NULL;
    mmap_read_lock(mm);
    buf = copy_task_text(tsk, mm->arg_start, mm->arg_end - mm->arg_start);
    if (!buf) {
        mmap_read_unlock(mm);
        mmput(mm);
        return NULL;
    }
    convert_strings(buf, mm->arg_end - mm->arg_start, ' ');
    mmap_read_unlock(mm);
    mmput(mm);
    return (char *)buf;
}

static char *get_task_envs(struct task_struct *tsk)
{
    struct mm_struct *mm = get_task_mm(tsk);
    void *buf;

    if (!mm)
        return NULL;
    mmap_read_lock(mm);
    buf = copy_task_text(tsk, mm->env_start, mm->env_end - mm->env_start);
    if (!buf) {
        mmap_read_unlock(mm);
        mmput(mm);
        return NULL;
    }
    convert_strings(buf, mm->env_end - mm->env_start, '\n');
    mmap_read_unlock(mm);
    mmput(mm);
    return (char *)buf;
}

static int collect_process_io(struct task_struct *tsk, struct task_io_accounting *result)
{
    struct task_struct *thread;
    memset(result, 0, sizeof(*result));
    // unsigned long flags;
    // if (!lock_task_sighand(tsk, &flags))
    //     return -ESRCH;

    spin_lock_irq(&tsk->sighand->siglock);

    task_io_accounting_add(result, &tsk->signal->ioac); // 已退出线程的IO统计
    thread = tsk;
    do {
        task_io_accounting_add(result, &thread->ioac);
    } while_each_thread(tsk, thread);
    spin_unlock_irq(&tsk->sighand->siglock);

    // unlock_task_sighand(tsk, &flags);
    return 0;
}

