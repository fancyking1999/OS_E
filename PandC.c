/**
 * @file PandC.c
 * @author FancyKing (YellyHornby@qq.com)
 * @brief 使用信号量机制与共享内存模拟生产者消费者问题
 *        你可以在这里看到实验结果的截图
 *        https://imgchr.com/i/Ejptx0
 *        https://imgchr.com/i/EXz1OS
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

/**
 * @brief 由于使用了 'semctl' https://linux.die.net/man/2/semctl
 *        所以在此补充定义 semun 类型
 */
union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};

/**
 * @brief 初始化 semid 信号量的值为 init_value
 *
 * @param semid 信号量标识符，使用 'semget' 的返回值得到
 * @param init_value 欲初始化的信号量初始值
 */
void initialize_value_of_sem(int semid, int init_value) {
  union semun sem_union_temp;
  sem_union_temp.val = init_value;
  if (semctl(semid, 0, SETVAL, sem_union_temp) == -1) {
    printf("Initialize the value of semaphore ERROR\n");
    exit(1);
  }
}

/**
 * @brief 依据信号量标识符删除信号量
 *
 * @param semid 信号量标识符，使用 'semget' 的返回值得到
 */
void delete_sem(int semid) {
  union semun sem_union_temp;
  if (semctl(semid, 0, IPC_RMID, sem_union_temp) == -1) {
    printf("Delete the semaphore ERROR\n");
    exit(1);
  }
}

void signal(int semid) {
  struct sembuf sembuf_temp;
  // sem_flg 设置为 SEM_UNDO，表示在进程退出时，撤销生存周期所做的所有操作
  // 避免因为进程异常退出而造成的死锁
  sembuf_temp.sem_flg = SEM_UNDO;
  // 信号量集合中的信号量编号，从 0 开始，所以该处表示有一个信号量
  sembuf_temp.sem_num = 0;
  // 将信号量的值 +sem_op，设置为正数即为增加信号量的值
  // 若执行完毕，存在信号量的值为非负数，则资源可用
  sembuf_temp.sem_op = 1;
  // semop 的参数依次为 信号量标识符， 操作结构体， 操作信号量个数
  if (semop(semid, &sembuf_temp, 1) == -1) {
    printf("Signal ERROR\n");
    exit(1);
  }
}

/**
 * @brief 对 semop 函数进行封装
 *
 * @param semid 欲操作信号量的标识符
 */
void wait(int semid) {
  struct sembuf sembuf_temp;
  // sem_flg 设置为 SEM_UNDO，表示在进程退出时，撤销生存周期所做的所有操作
  // 避免因为进程异常退出而造成的死锁
  sembuf_temp.sem_flg = SEM_UNDO;
  // 信号量集合中的信号量编号，从 0 开始，所以该处表示有一个信号量
  sembuf_temp.sem_num = 0;
  // 将信号量的值 +sem_op，设置为负数即为减少信号量的值
  // 若执行完毕，存在信号量的值为负数，则为进程阻塞，直到资源可用
  sembuf_temp.sem_op = -1;
  // semop 的参数依次为 信号量标识符， 操作结构体， 操作信号量个数
  if (semop(semid, &sembuf_temp, 1) == -1) {
    printf("Wait ERROR\n");
  }
}

// 共享内存中的结构体
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
  // 进入临界区

  // 共享数据读入字符
  char ch = fgetc(shared->fp_in);
  // 已经读到文件尾部
  if (ch == EOF) {
    signal(mutex);
    return 0;
  }
  // 保存读取的资源，记录循环标号
  shared->buff[shared->in] = ch;
  printf("%c\n", ch);
  shared->in = (shared->in + 1) % N;

  signal(mutex);
  signal(full);
  // 退出临界区

  return 1;
}

int consumer() {
  wait(full);
  wait(mutex);
  // 进入临界区

  // 打印循环标号，取出共享数据
  // printf("out_id: %d:", shared->out);
  char out_char = shared->buff[shared->out];
  if (out_char == EOF) {
    signal(mutex);
    return 0;
  }
  // 写入文件，输出流刷新，并打印到终端
  shared->out = (shared->out + 1) % N;
  fprintf(shared->fp_out, "%c", out_char);
  fflush(shared->fp_out);
  printf("%c\n", out_char);

  signal(mutex);
  signal(empty);
  // 退出临界区

  return 1;
}

int main(int argc, char const *argv[]) {
  // 申请共享内存，如果要打开一个已经存在的合法共享内存， shmflg 可为 0
  // 参数分别为 共享内存标识符，相关数据结构体，
  shared_memory_id =
      shmget(12345, sizeof(struct shared_data), 0666 | IPC_CREAT);

  // shmat 三个参数分别为 共享内存标识符，指定连接地址，动作标记，挂接共享内存
  // 当 shmaddr 为 NULL，则系统自动选择一个地址
  shared_memory_addr = shmat(shared_memory_id, 0, 0);
  shared = (struct shared_data *)shared_memory_addr;
  // 打开文件输入输出流
  shared->fp_in = fopen("in", "r");
  shared->fp_out = fopen("out", "w");
  if (shared->fp_in == NULL) {
    printf("Read input file ERROR");
    return 0;
  }
  shared->in = 0;
  shared->out = 0;
  // 得到信号量标识符，便于后续控制
  empty = semget(3000, 1, 0666 | IPC_CREAT);
  full = semget(3001, 1, 0666 | IPC_CREAT);
  mutex = semget(3002, 1, 0666 | IPC_CREAT);
  // 按照实际情况初始化信号量数值
  initialize_value_of_sem(empty, N);
  initialize_value_of_sem(full, 0);
  initialize_value_of_sem(mutex, 1);

  // 进程ID，其实貌似也就是个int
  pid_t pidone, pidtwo;
  while ((pidone = fork()) == -1)
    ;
  if (pidone > 0) {
    while ((pidtwo = fork()) == -1)
      ;
    do {
      printf("Producing below ");
    } while (producer());
  } else {
    do {
      printf("\t\t  Recieveing below ");
    } while (consumer());
    printf("\n");
  }

  fclose(shared->fp_out);
  fclose(shared->fp_in);

  return 0;
}
