/**
 * @file pcbshower.c
 * @author FancyKing (YellyHornby@qq.com)
 * @brief 利用进程控制块显示进程信息
 * @version 0.1
 * @date 2019-05-25 - 2019-05-26
 *
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/sched/task.h>
#include <linux/seq_file.h>
#include <linux/slab.h>

#define FILENAME "pcbshower"
struct task_struct *shower;

int task_nums = 0, num_of_running = 0, num_of_interruptible = 0,
    num_of_uninterruptible = 0, num_of_zombie = 0, num_of_stopped = 0,
    num_of_traced = 0, num_of_dead = 0, num_of_unknown = 0, tmp_exit_state = 0,
    tmp_state = 0;

static void *scull_seq_start(struct seq_file *m, loff_t *pos) {
  if (*pos == 0) {
    shower = &init_task;
    return &shower;
  } else {
    if (shower == &init_task) {
      return NULL;
    } else {
      return (void *)pos;
    }
  }
}

static void *scull_seq_next(struct seq_file *m, void *v, loff_t *pos) {
  (*pos)++;
  shower = next_task(shower);
  return NULL;
}

static void scull_seq_stop(struct seq_file *m, void *v) {
  if (shower == &init_task) {
    seq_printf(m, "[M] total tasks          : %10d\n", task_nums);
    seq_printf(m, "[M] TASK_RUNNING         : %10d\n", num_of_running);
    seq_printf(m, "[M] TASK_INTERRUPTIBLE   : %10d\n", num_of_interruptible);
    seq_printf(m, "[M] TASK_UNINTERRUPTIBLE : %10d\n", num_of_uninterruptible);
    seq_printf(m, "[M] TASK_TRACED          : %10d\n", num_of_stopped);
    seq_printf(m, "[M] TASK_TRACED          : %10d\n", num_of_stopped);
    seq_printf(m, "[M] EXIT_ZOMBIE          : %10d\n", num_of_zombie);
    seq_printf(m, "[M] EXIT_DEAD            : %10d\n", num_of_dead);
    seq_printf(m, "[M] UNKNOWN              : %10d\n", num_of_unknown);
  }
}

int scull_seq_show(struct seq_file *m, void *v) {
  seq_printf(m, "#%-3d ", task_nums++);
  seq_printf(m, "%7d ", shower->pid);
  seq_printf(m, "%5lu ", shower->state);
  seq_printf(m, "%-20s ", shower->comm);
  seq_printf(m, "%-5d ", shower->prio);
  seq_printf(m, "%-5d ", shower->static_prio);
  seq_printf(m, "%-5d ", shower->normal_prio);
  seq_printf(m, "%-4d ", shower->rt_priority);
  seq_printf(m, "%u ", shower->policy);
  seq_printf(m, "%-11lu ", shower->nvcsw);
  seq_printf(m, "%5lu ", shower->nivcsw);
  seq_printf(m, "%13lu ", shower->utime);
  seq_printf(m, "%13lu ", shower->stime);
  // seq_printf( m,  "%lu ", shower->real_start_time.tv_sec);
  seq_puts(m, "\n");
  tmp_state = shower->state;            // put p->state to variable t_state
  tmp_exit_state = shower->exit_state;  // similar to above

  if (tmp_exit_state != 0) {
    switch (tmp_exit_state) {
      case EXIT_ZOMBIE:
        num_of_zombie++;
        break;
      case EXIT_DEAD:
        num_of_dead++;
        break;
      default:
        break;
    }
  } else {
    switch (tmp_state) {
      case TASK_RUNNING:
        num_of_running++;
        break;
      case TASK_INTERRUPTIBLE:
        num_of_interruptible++;
        break;
      case TASK_UNINTERRUPTIBLE:
        num_of_uninterruptible++;
        break;
      case TASK_STOPPED:
        num_of_stopped++;
        break;
      case TASK_TRACED:
        num_of_traced++;
        break;
      default:
        num_of_unknown++;
        break;
    }
  }
  return 0;
}

static struct seq_operations scull_seq_ops = {.start = scull_seq_start,
                                              .next = scull_seq_next,
                                              .stop = scull_seq_stop,
                                              .show = scull_seq_show};

static int scull_proc_open(struct inode *inode, struct file *file) {
  return seq_open(file, &scull_seq_ops);
}

static const struct file_operations scull_proc_ops = {.owner = THIS_MODULE,
                                                      .open = scull_proc_open,
                                                      .read = seq_read,
                                                      .llseek = seq_lseek,
                                                      .release = seq_release};

int __init shower_init(void) {
  struct proc_dir_entry *scull_seq_entry;
  printk("Now installing the module: %s \n", FILENAME);
  scull_seq_entry = proc_create(FILENAME, 0x644, NULL, &scull_proc_ops);
  if (scull_seq_entry == NULL) {
    return -ENOMEM;
  } else {
    return 0;
  }
}

void __exit shower_exit(void) {
  remove_proc_entry(FILENAME, NULL);
  printk("Now removing the module: %s \n", FILENAME);
}

module_init(shower_init);
module_exit(shower_exit);

MODULE_LICENSE("GPL");
