#include "pw_internal.h"

#include <linux/seq_file.h>

static int pi_open(struct inode *, struct file *);
static int pi_show(struct seq_file *, void *);
static int pi_release(struct inode *, struct file *);
static const char *task_state_name(char state);

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

    seq_printf(m, "\n[cpu]\n");
    seq_printf(m, "user_time_ms: \n");
    seq_printf(m, "system_time_ms: \n");
    seq_printf(m, "total_time_ms: \n");
    seq_printf(m, "last_cpu: %d\n", task_cpu(tsk));
    seq_printf(m, "voluntary_context_switches: \n");
    seq_printf(m, "nonvoluntary_context_switches: \n");

    seq_printf(m, "\n[scheduler]\n");
    seq_printf(m, "policy:\n");
    seq_printf(m, "policy_id: \n");
    seq_printf(m, "priority:\n");
    seq_printf(m, "static_priority:\n");
    seq_printf(m, "normal_priority:\n");
    seq_printf(m, "nice:\n");
    seq_printf(m, "rt_priority:\n");

    seq_printf(m, "\n[memory]\n");
    seq_printf(m, "rss_kb:\n");
    seq_printf(m, "shared_rss_kb: \n");
    seq_printf(m, "data_kb: \n");
    seq_printf(m, "stack_kb: \n");
    seq_printf(m, "code_kb: \n");
    seq_printf(m, "page_faults_minor: \n");
    seq_printf(m, "page_faults_major: \n");

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