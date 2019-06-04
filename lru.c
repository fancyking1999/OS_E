/**
 * @file lru.c
 * @author FancyKing (YellyHornby@qq.com)
 * @brief 基于最近最久未使用的请求分页虚存内存置换算法模拟
 *        https://imgchr.com/i/Vtb25R
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
    int a = 0, b = 0;
    for (int j = 0; j < num_of_frames; ++j) {
      if (frame[j] == pages[i]) {
        a = b = 1;
        break;
      }
    }
    if (a == 0) {
      for (int j = 0; j < num_of_frames; ++j) {
        if (frame[j] == -1) {
          frame[j] = pages[i];
          b = 1;// 若不计算替换空页面(-1)产生的缺页，请注释下面一行
          ++num_of_faults;
          break;
        }
      }
    }
    if (b == 0) {
      int *tmp = calloc(num_of_frames + 1, sizeof(int)), position = 0;
      for (int j = i - 1, m = 1; m < num_of_frames; ++m, --j) {
        for (int n = 0; n < num_of_frames; ++n) {
          if (frame[n] == pages[j]) {
            tmp[n] = 1;
          }
        }
      }
      for (int j = 0; j < num_of_frames; ++j) {
        if (tmp[j] == 0) {
          position = j;
        }
      }
      frame[position] = pages[i];
      ++num_of_faults;
    }
    printf("%3d-th exchange \t", i + 1);
    for (int j = 0; j < num_of_frames; ++j) {
      printf("%d\t", frame[j]);
    }
    puts("");
  }
  printf("Number of page faults that occur during the run is : %d\n",
         num_of_faults);
  return 0;
}
