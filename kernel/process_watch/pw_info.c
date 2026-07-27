#include "pw_internal.h"

#include <linux/seq_file.h>
#include <linux/sched/cputime.h>
#include <linux/mm.h>

static int pi_open(struct inode *, struct file *);
static int pi_show(struct seq_file *, void *);
static int pi_release(struct inode *, struct file *);
static const char *task_state_name(char state);
static const char *pi_policy_name(unsigned int policy);
static unsigned long pages_to_kib(unsigned long pages);

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

    if (!tsk)
        return -ESRCH;

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
    seq_printf(m, "user_time_ms: %lld\n", user_tims_ns);
    seq_printf(m, "system_time_ms: %lld\n", system_time_ns);
    seq_printf(m, "total_time_ms: %lld\n", total_time_ns);
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
        seq_printf(m, "rss_kb: %lu\n", pages_to_kib(get_mm_rss(mm)));
        seq_printf(m, "shared_rss_kb: %lu\n", pages_to_kib(get_mm_counter(mm, MM_SHMEMPAGES)));
        seq_printf(m, "data_kb: \n");
        seq_printf(m, "stack_kb: \n");
        seq_printf(m, "code_kb: \n");
        seq_printf(m, "page_faults_minor: \n");
        seq_printf(m, "page_faults_major: \n");
        mmap_read_unlock(mm);
    }

    seq_printf(m, "\n[io]\n");
    seq_printf(m, "read_bytes: \n");
    seq_printf(m, "write_bytes: \n");
    seq_printf(m, "cancelled_write_bytes: \n");
    seq_printf(m, "read_syscalls: \n");
    seq_printf(m, "write_syscalls: \n");

    seq_printf(m, "\n[signals]\n");
    seq_printf(m, "pending: \n");
    seq_printf(m, "blocked: \n");
    seq_printf(m, "ignored: \n");
    seq_printf(m, "caught: \n");

    seq_printf(m, "\n[security]\n");
    seq_printf(m, "uid: \n");
    seq_printf(m, "effective_uid: \n");
    seq_printf(m, "saved_uid: \n");
    seq_printf(m, "fs_uid: \n");
    seq_printf(m, "gid: \n");
    seq_printf(m, "effective_gid: \n");
    seq_printf(m, "no_new_privs: \n");
    seq_printf(m, "seccomp_mode: \n");

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