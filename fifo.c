/**
 * @file fifo.c
 * @author FancyKing (YellyHornby@qq.com)
 * @brief 基于先进先出的请求分页虚存内存置换算法模拟
 *        https://imgchr.com/i/Vts82t
 * @version 0.1
 * @date 2019-06-04
 *
 * @copyright Copyright (c) 2019
 *
 */

#include <stdio.h>
#include <stdlib.h>

int main() {
  int num_of_frames, num_of_pages, num_of_faults = 0;
  printf("Please input page's num and frame's num:\n");
  scanf("%d %d", &num_of_pages, &num_of_frames);

  printf("Please enter the page invocation order:\n");
  int *pages = calloc(num_of_frames, sizeof(int));
  for (int i = 0; i < num_of_pages; ++i) {
    scanf("%d", &pages[i]);
  }

  int *frame = calloc(num_of_frames, sizeof(int));
  for (int i = 0; i < num_of_frames; ++i) {
    frame[i] = -1;
  }

  for (int i = 0; i < num_of_pages; ++i) {
    printf("%d-th exchange\t", i + 1);
    int exist = 0;
    for (int j = 0; j < num_of_frames; ++j) {
      if (pages[i] == frame[j]) {
        ++exist;
        --num_of_faults;
      }
    }
    ++num_of_faults;
    if ((num_of_faults <= num_of_frames) && (exist == 0)) {
      frame[i] = pages[i];
    } else if (exist == 0) {
      frame[(num_of_faults - 1) % num_of_frames] = pages[i];
    }
    for (int j = 0; j < num_of_frames; ++j) {
      printf("%-5d\t", frame[j]);
    }
    puts("");
  }
  printf("Number of page faults that occur during the run is : %d\n",
         num_of_faults);
  return 0;
}
