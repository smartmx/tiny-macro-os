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

/* 所有任务的时间变量值*/
volatile TINY_MACRO_OS_TIME_t os_task_timers[TINY_MACRO_OS_TASKS_MAX_NUM];

/* 所有任务的函数运行标记值*/
volatile TINY_MACRO_OS_LINE_t os_task_linenums[TINY_MACRO_OS_TASKS_MAX_NUM];

/*
void SysTick_Handler(void)
{
    //必须要在定时器函数中更新系统任务时钟timer，提供心跳
    OS_UPDATE_TIMERS();
}
*/

/* 下面的例子可以通过更改#if 1启用，然后在程序main函数中调用tmos_test_main查看例子运行效果 */

#if 0 /*普通任务OS_TASK_WAITX例子 */

/*void参数表示函数无参数，可以不写该void，但是定义不标准，所以写上void最好 */
OS_TASK(OS_TASK_TEST1, void)
{
    OS_TASK_START(OS_TASK_TEST1);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        printf("OS_TASK_TEST1\n");
        OS_TASK_WAITX(OS_TASK_TEST1, OS_SEC_TICKS * 6 / 10);
    }
    OS_TASK_END(OS_TASK_TEST1);
}

/* 带参数任务编写格式函数参数直接作为宏定义中的成员写进去即可 */
OS_TASK(OS_TASK_TEST2, unsigned char params, ...)
{
    OS_TASK_START(OS_TASK_TEST2);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        printf("OS_TASK_TEST2:%d\n", params);
        OS_TASK_WAITX(OS_TASK_TEST2, OS_SEC_TICKS * 6 / 10);
    }
    OS_TASK_END(OS_TASK_TEST2);
}

void tmos_test_main(void)
{
    OS_INIT_TASKS();
    unsigned char i = 0;
    while (1)
    {
        /* 所有的主任务都需要手动在main函数的while(1)中调用 */
        OS_RUN_TASK(OS_TASK_TEST1);
        OS_RUN_TASK(OS_TASK_TEST2, i++);
    }
}

#elif 0 /* OS_TASK_CALL_ANOTHER，OS_TASK_SUSPEND_ANOTHER，OS_TASK_EXIT_ANOTHER，OS_TASK_RESTART_ANOTHER运行例子 */

OS_TASK(OS_TASK_TEST2)
{
    OS_TASK_START(OS_TASK_TEST2);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        printf("task2 1\n");
        OS_TASK_WAITX(OS_TASK_TEST2, OS_SEC_TICKS * 3 / 10);
        printf("task2 2\n");
        OS_TASK_WAITX(OS_TASK_TEST2, OS_SEC_TICKS * 4 / 10);
        printf("task2 3\n");
        OS_TASK_WAITX(OS_TASK_TEST2, OS_SEC_TICKS * 5 / 10);
    }
    OS_TASK_END(OS_TASK_TEST2);
}

OS_TASK(OS_TASK_TEST1, void)
{
    OS_TASK_START(OS_TASK_TEST1);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        printf("task1 start\n");
        OS_TASK_WAITX(OS_TASK_TEST1, OS_SEC_TICKS * 3);

        printf("suspend task2\n");
        /*挂起任务，保留任务状态*/
        OS_TASK_SUSPEND_ANOTHER(OS_TASK_TEST2);
        OS_TASK_WAITX(OS_TASK_TEST1, OS_SEC_TICKS * 3);
        printf("restart task2\n");
        /* 必须已经停止的任务才可以重启，不然无效 */
        OS_TASK_RESTART_ANOTHER(OS_TASK_TEST2, 0);
        OS_TASK_WAITX(OS_TASK_TEST1, OS_SEC_TICKS * 3);

        printf("exit task2\n");
        /*退出任务，清除任务状态*/
        OS_TASK_EXIT_ANOTHER(OS_TASK_TEST2);
        OS_TASK_WAITX(OS_TASK_TEST1, OS_SEC_TICKS * 3);
        printf("call task2\n");
        OS_TASK_CALL_ANOTHER(OS_TASK_TEST2);
        OS_TASK_WAITX(OS_TASK_TEST1, OS_SEC_TICKS * 3);
    }
    OS_TASK_END(OS_TASK_TEST1);
}

void tmos_test_main(void)
{
    OS_INIT_TASKS();
    while (1)
    {
        /* 所有的主任务都需要手动在main函数的while(1)中调用 */
        OS_RUN_TASK(OS_TASK_TEST1);
        OS_RUN_TASK(OS_TASK_TEST2);
    }
}

#elif 0 /* OS_CALL_SUB子任务运行例子， */

/* 子任务不退出，查看效果 */
OS_TASK(OS_TASK_TEST2)
{
    OS_TASK_START(OS_TASK_TEST2);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        printf("child task 1\n");
        OS_TASK_WAITX(OS_TASK_TEST2, OS_SEC_TICKS * 3 / 10);
        printf("child task 2\n");
        OS_TASK_WAITX(OS_TASK_TEST2, OS_SEC_TICKS * 4 / 10);
        printf("child task 3\n");
        OS_TASK_WAITX(OS_TASK_TEST2, OS_SEC_TICKS * 5 / 10);
    }
    OS_TASK_END(OS_TASK_TEST2);
}

OS_TASK(OS_TASK_TEST1, void)
{
    OS_TASK_START(OS_TASK_TEST1);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        printf("father task\n");
        OS_TASK_WAITX(OS_TASK_TEST1, OS_SEC_TICKS * 2 / 10);
        /* 如果需要每次开始执行子任务时都需要子任务从头开始执行，在OS_CALL_SUB之前使用OS_TASK_RESET_ANOTHER复位子任务 */
        OS_TASK_RESET_ANOTHER(OS_TASK_TEST2);
#if 1
        /* 没有超时时间 */
        OS_CALL_SUB(OS_TASK_TEST1, OS_TASK_TEST2);
#else
        /* 有超时时间 */
        OS_CALL_SUBX(OS_TASK_TEST1, OS_SEC_TICKS * 5, OS_TASK_TEST2);
#endif
    }
    OS_TASK_END(OS_TASK_TEST1);
}

void tmos_test_main(void)
{
    OS_INIT_TASKS();
    while (1)
    {
        /* 所有的主任务都需要手动在main函数的while(1)中调用 */
        OS_RUN_TASK(OS_TASK_TEST1);
    }
}

#elif 0     /* 信号量例子 */

OS_SEM_t sem_test;

OS_TASK(OS_TASK_TEST2)
{
    OS_TASK_START(OS_TASK_TEST2);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        OS_WAIT_SEM(OS_TASK_TEST2, sem_test, 1);
        printf("OS_WAIT_SEM get\n");
        OS_WAIT_SEMX(OS_TASK_TEST2, sem_test, 1, 20);
        if (sem_test == OS_SEM_TIMEOUT)
        {
            printf("OS_WAIT_SEMX 1 timeout\n");
        }
        else
        {
            printf("OS_WAIT_SEMX 1 ok\n");
        }
        OS_WAIT_SEMX(OS_TASK_TEST2, sem_test, 1, 300);
        if (sem_test == OS_SEM_TIMEOUT)
        {
            printf("OS_WAIT_SEMX 2 timeout\n");
        }
        else
        {
            printf("OS_WAIT_SEMX 2 ok\n");
        }
    }
    OS_TASK_END(OS_TASK_TEST2);
}

OS_TASK(OS_TASK_TEST1, void)
{
    OS_TASK_START(OS_TASK_TEST1);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        printf("send sem\n");
        OS_SEND_SEM(sem_test);
        OS_TASK_WAITX(OS_TASK_TEST1, OS_SEC_TICKS * 2 / 10);
    }
    OS_TASK_END(OS_TASK_TEST1);
}

void tmos_test_main(void)
{
    OS_INIT_TASKS();
    OS_INIT_SEM(sem_test);
    while (1)
    {
        /* 所有的主任务都需要手动在main函数的while(1)中调用 */
        OS_RUN_TASK(OS_TASK_TEST1);
        OS_RUN_TASK(OS_TASK_TEST2);
    }
}

#endif

