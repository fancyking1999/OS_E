/**
 * @file PandC.c
 * @author FancyKings (YellyHornby@qq.com)
 * @brief Using semaphore method to solve producers and consumer problem
 * @version 1.0
 * @date 2019-05-18 - 2019-05-19
 *
 * @copyright Copyright (c) 2019
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define N 10

union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};

void initialize_value_of_sem(int semid, int init_value) {
  union semun sem_union_temp;
  sem_union_temp.val = init_value;
  if (semctl(semid, 0, SETVAL, sem_union_temp) == -1) {
    printf("Initialize the value of semaphore ERROR\n");
    exit(1);
  }
}

void delete_sem(int semid) {
  union semun sem_union_temp;
  if (semctl(semid, 0, IPC_RMID, sem_union_temp) == -1) {
    printf("Delete the semaphore ERROR\n");
    exit(1);
  }
}

void signal(int semid) {
  struct sembuf sembuf_temp;
  sembuf_temp.sem_flg = SEM_UNDO;
  sembuf_temp.sem_num = 0;
  sembuf_temp.sem_op = 1;
  if (semop(semid, &sembuf_temp, 1) == -1) {
    printf("Signal ERROR\n");
    exit(1);
  }
}

void wait(int semid) {
  struct sembuf sembuf_temp;
  sembuf_temp.sem_flg = SEM_UNDO;
  sembuf_temp.sem_num = 0;
  sembuf_temp.sem_op = -1;
  if (semop(semid, &sembuf_temp, 1) == -1) {
    printf("Wait ERROR\n");
  }
}

struct shared_data {
  int in, out;
  char buff[N];
  FILE *fp_in;
  FILE *fp_out;
};
struct shared_data *shared;

int empty, full, mutex;
int shared_memory_id;
void *shared_memory_addr;

int producer() {
  wait(empty);
  wait(mutex);

  char ch = fgetc(shared->fp_in);
  // 将文件内容逐个打印在屏幕上
  printf("%c", ch);
  // 已经读到文件尾部
  if (ch == EOF) {
    signal(mutex);
    return 0;
  }
  shared->buff[shared->in] = ch;
  shared->in = (shared->in + 1) % N;

  signal(mutex);
  signal(full);

  return 1;
}

int consumer() {
  wait(full);
  wait(mutex);
  // 进入临界区

  printf("outid: %d", shared->out);
  char out_char = shared->buff[shared->out];
  if (out_char == EOF) {
    signal(mutex);
    return 0;
  }
  shared->out = (shared->out + 1) % N;
  fprintf(shared->fp_out, "%c", out_char);
  fflush(shared->fp_out);
  printf("%c+\t", out_char);

  // 退出临界区
  signal(mutex);
  signal(empty);

  return 1;
}

int main(int argc, char const *argv[]) {
  shared_memory_id =
      shmget(12345, sizeof(struct shared_data), 0666 | IPC_CREAT);
  shared_memory_addr = shmat(shared_memory_id, 0, 0);
  shared = (struct shared_data *)shared_memory_addr;
  shared->fp_in = fopen("in", "r");
  shared->fp_out = fopen("out", "w");
  if (shared->fp_in == NULL) {
    printf("Read input file ERROR");
    return 0;
  }
  shared->in = 0;
  shared->out = 0;
  empty = semget(3000, 1, 0666 | IPC_CREAT);
  full = semget(3001, 1, 0666 | IPC_CREAT);
  mutex = semget(3002, 1, 0666 | IPC_CREAT);
  initialize_value_of_sem(empty, N);
  initialize_value_of_sem(full, 0);
  initialize_value_of_sem(mutex, 1);

  pid_t pidone, pidtwo;
  while ((pidone = fork()) == -1);
  if (pidone > 0) {
    while ((pidtwo = fork()) == -1);
    while (producer());
  } else {
    while (consumer());
    printf("\n");
  }

  fclose(shared->fp_out);
  fclose(shared->fp_in);

  return 0;
}
