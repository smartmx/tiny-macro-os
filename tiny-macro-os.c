/*
 * Copyright (c) 2022, smartmx - smartmx@qq.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * This file is part of the tiny-macro-os Library.
 * 
 */
#include "tiny-macro-os.h"

volatile TINY_MACRO_OS_TIME_t os_task_timers[TINY_MACRO_OS_TASKS_NUM];      //所有任务的时间变量值。

/*
 * 该调度器思想借鉴了protothread，通过精简整理后形成该os，其实可以算作另类的状态机。
 * 参考了阿莫论坛https://www.amobbs.com/thread-5508720-2-1.html中的smset提出的小小调度器。
 * 目的只为极度精简，并且可为51所用，所以不采用链表管理任务，全靠手动。
 */
/*
 * 普通任务编写格式
os_task os_task_test1(void){
  os_task_boot();
  os_task_start();
  static uint8_t i = 0;//任务中定义的会在任务切换前后都使用的局部变量需要使用static定义，不然变量会丢失
  //禁止在os_task_start和os_task_end中使用switch和return;
  while(1){
    printf("os test1\n");
    os_task_WaitX(1000);
  }
  os_task_end();
}
*/

/*
 * 带参数任务编写格式
os_task os_task_test2(uint8_t params){
  os_task_boot();
  //os_task_boot到os_task_start之间的代码，每次执行任务都会运行，可以在此增加任务复位等功能退出下面正在等待中的小任务
  if(params){
      os_task_Reset();
  }
  os_task_start();
  //禁止使用switch
  while(1){
    printf("os test2:%d\n",params);
    os_task_WaitX(600);
  }
  os_task_end();
}
*/


/*
 * 所有的主任务都需要手动在main函数的while(1)中调用
void main()
{
  os_InitTasks();
  while(1){
    os_RunTask(os_task_test1,0);
    os_RunTaskWithParam(os_task_test2,1,1);
//    os_RunHpTask(os_task_test1,0);
//    os_RunHpTaskWithParam(os_task_test2,1,1);
  }
}
*/

/*
 * 在定时器中断中更新系统任务时钟timer
void SysTick_Handler(void)
{
  os_UpdateTimers();
}
*/

/*
 * 信号量例子
os_SEM_t test;

os_task os_sem_test(void){
  os_task_boot();
  os_task_start();
  os_InitSem(test);
  //禁止使用switch
  while(1){
    os_WaitSem(test,0);
    printf("uart rec\n");
    os_WaitSemX(test,10,1);//第三个参数最好不要为0，不然无法计算时间，变成看这个任务运行几次了。
    if(test == os_TIMEOUT){
      printf("uart timeout\n");
    } else {
      printf("uart rec\n");
    }
  }
  os_task_end();
}

void UART0_IRQHandler()
{
  if(RX){
    os_SendSem(test);
  }
}
*/

/*
 * os_CallSub子任务运行例子
os_task os_child_task(void){
  os_task_boot();
  os_task_start();
  //禁止使用switch
  os_task_WaitX(500);
  printf("os_child_task 1\n");
  os_task_WaitX(500);
  printf("os_child_task 2\n");
  os_task_WaitX(500);
  printf("os_child_task 3\n");
  os_task_WaitX(500);
  os_task_end();
}

os_task os_father_task(void){
  os_task_boot();
  os_task_start();
  //禁止使用switch
  while(1){
    printf("os_father_task\n");
    os_CallSub(os_child_task);
  }
  os_task_end();
}
*/



