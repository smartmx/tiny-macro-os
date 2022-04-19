/*
 * Copyright (c) 2022, smartmx - smartmx@qq.com
 * Copyright (c) 2022, smset - <https://github.com/smset028>
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
#include "ctimer.h"

/* 所有任务的时间变量值，时间如果是在中断函数中更新，则时间类型必须中断安全。*/
/* 如果时间类型非中断安全，则可以考虑使用OS_UPDATES_TIMERS(TICKS)代替OS_UPDATE_TIMERS()。*/
volatile TINY_MACRO_OS_TIME_t os_task_timers[TINY_MACRO_OS_TASKS_MAX_NUM];

/* 所有任务的函数运行标记值*/
volatile TINY_MACRO_OS_LINE_t os_task_linenums[TINY_MACRO_OS_TASKS_MAX_NUM];

/*
单片机操作时间变量类型是中断安全的
比如8位单片机操作8位数据
32位单片机操作8、16、32位数据
void SysTick_Handler(void)
{
    //必须要在定时器函数中更新系统任务时钟timer，提供心跳
    OS_UPDATE_TIMERS();
}
*/

/*
单片机操作时间变量类型是非中断安全的
比如8位单片机操作16、32位数据
32位单片机操作64、128位数据

为了中断安全使用OS_UPDATES_TIMERS(TICKS)例子

unsigned int time;
void SysTick_Handler(void)
{
    time++;             //在中断中更新时间计数
}

void tmos_test_main(void)
{
    OS_INIT_TASKS();
    unsigned char i = 0;
    while (1)
    {
        if(time > 0)
        {
            unsigned int stime;         //定义临时变量保存time的值
            disable_interrupt();        //关中断，确保安全
            stime = time;               //保存时间到临时变量中
            time = 0;                   //清零时间计数
            enable_interrupt();         //重新打开中断，开始时间计数
            OS_UPDATES_TIMERS(stime);   //更新任务时间
        }
        OS_RUN_TASK(os_test1);
    }
}
*/

/* 下面的例子可以通过更改#if 1启用，然后在程序main函数中调用tmos_test_main查看例子运行效果 */

#if 0 /*普通任务和带参数任务编写和OS_TASK_WAITX使用例子 */

/*void参数表示函数无参数，可以不写该void，但是定义不标准，所以写上void最好 */
OS_TASK(os_test1, void)
{
    OS_TASK_START(os_test1);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        printf("os_test1\n");
        OS_TASK_WAITX(OS_SEC_TICKS * 6 / 10);
    }
    OS_TASK_END(os_test1);
}

/* 带参数任务编写格式函数参数直接作为宏定义中的成员写进去即可 */
OS_TASK(os_test2, unsigned char params, ...)
{
    OS_TASK_START(os_test2);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        printf("os_test2:%d\n", params);
        OS_TASK_WAITX(OS_SEC_TICKS * 6 / 10);
    }
    OS_TASK_END(os_test2);
}

void tmos_test_main(void)
{
    OS_INIT_TASKS();
    unsigned char i = 0;
    while (1)
    {
        /* 所有的主任务都需要手动在main函数的while(1)中调用 */
        OS_RUN_TASK(os_test1);
        OS_RUN_TASK(os_test2, i++);
    }
}

#elif 0 /* OS_TASK_WAIT_UNTIL，OS_TASK_WAIT_UNTILX运行例子 */

unsigned char params_test;

/*void参数表示函数无参数，可以不写该void，但是定义不标准，所以写上void最好 */
OS_TASK(os_test1, void)
{
    OS_TASK_START(os_test1);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        /* 每0.1秒params_test加1 */
        printf("++\n");
        OS_TASK_WAITX(OS_SEC_TICKS / 10);
        params_test++;
    }
    OS_TASK_END(os_test1);
}

/* 带参数任务编写格式函数参数直接作为宏定义中的成员写进去即可 */
OS_TASK(os_test2, unsigned char params, ...)
{
    /* OS_TASK_WAIT_UNTILX需要一个常驻变量保存时间信息，可以是全局变量或局部静态变量 */
    static unsigned char timecnt;
    OS_TASK_START(os_test2);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        /*每1个tick检测一次params是否可以被10整除*/
        OS_TASK_WAIT_UNTIL((params % 10) == 0, 1);
        printf("os_test2:%d\n", params);
        /*每OS_SEC_TICKS / 10个tick检测一次params是否可以被10整除，检测10次超时*/
        OS_TASK_WAIT_UNTILX((params % 100) == 0, OS_SEC_TICKS / 10, 10, timecnt);
        printf("os_test2:%d,cnt:%d\n", params, timecnt);
        OS_TASK_WAITX(OS_SEC_TICKS / 10);
    }
    OS_TASK_END(os_test2);
}

void tmos_test_main(void)
{
    OS_INIT_TASKS();
    params_test = 0;
    while (1)
    {
        /* 所有的主任务都需要手动在main函数的while(1)中调用 */
        OS_RUN_TASK(os_test1);
        OS_RUN_TASK(os_test2, params_test);
    }
}

#elif 0 /* OS_TASK_CALL_ANOTHER，OS_TASK_SUSPEND_ANOTHER，OS_TASK_EXIT_ANOTHER，OS_TASK_RESTART_ANOTHER运行例子 */

OS_TASK(os_test2)
{
    OS_TASK_START(os_test2);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        printf("task2 1\n");
        OS_TASK_WAITX(OS_SEC_TICKS * 3 / 10);
        printf("task2 2\n");
        OS_TASK_WAITX(OS_SEC_TICKS * 4 / 10);
        printf("task2 3\n");
        OS_TASK_WAITX(OS_SEC_TICKS * 5 / 10);
    }
    OS_TASK_END(os_test2);
}

OS_TASK(os_test1, void)
{
    OS_TASK_START(os_test1);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        printf("task1 start\n");
        OS_TASK_WAITX(OS_SEC_TICKS * 3);

        printf("suspend task2\n");
        /*挂起任务，保留任务状态*/
        OS_TASK_SUSPEND_ANOTHER(os_test2);
        OS_TASK_WAITX(OS_SEC_TICKS * 3);
        printf("restart task2\n");
        /* 必须已经停止的任务才可以重启，不然无效 */
        OS_TASK_RESTART_ANOTHER(os_test2, 0);
        OS_TASK_WAITX(OS_SEC_TICKS * 3);

        printf("exit task2\n");
        /*退出任务，清除任务状态*/
        OS_TASK_EXIT_ANOTHER(os_test2);
        OS_TASK_WAITX(OS_SEC_TICKS * 3);
        printf("call task2\n");
        OS_TASK_CALL_ANOTHER(os_test2);
        OS_TASK_WAITX(OS_SEC_TICKS * 3);
    }
    OS_TASK_END(os_test1);
}

void tmos_test_main(void)
{
    OS_INIT_TASKS();
    while (1)
    {
        /* 所有的主任务都需要手动在main函数的while(1)中调用 */
        OS_RUN_TASK(os_test1);
        OS_RUN_TASK(os_test2);
    }
}

#elif 0 /* OS_CALL_SUB子任务运行例子， */

/* 子任务不退出，查看效果 */
OS_TASK(os_test2)
{
    OS_TASK_START(os_test2);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        printf("child task 1\n");
        OS_TASK_WAITX(OS_SEC_TICKS * 3 / 10);
        printf("child task 2\n");
        OS_TASK_WAITX(OS_SEC_TICKS * 4 / 10);
        printf("child task 3\n");
        OS_TASK_WAITX(OS_SEC_TICKS * 5 / 10);
    }
    OS_TASK_END(os_test2);
}

OS_TASK(os_test1, void)
{
    OS_TASK_START(os_test1);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        printf("father task\n");
        OS_TASK_WAITX(OS_SEC_TICKS * 2 / 10);
        /* 如果需要每次开始执行子任务时都需要子任务从头开始执行，在OS_CALL_SUB之前使用OS_TASK_RESET_ANOTHER复位子任务 */
        OS_TASK_RESET_ANOTHER(os_test2);
#if 0
        /* 没有超时时间 */
        OS_CALL_SUB(os_test2);
#else
        /* 有超时时间 */
        OS_CALL_SUBX(OS_SEC_TICKS * 5, os_test2);
#endif
    }
    OS_TASK_END(os_test1);
}

void tmos_test_main(void)
{
    OS_INIT_TASKS();
    while (1)
    {
        /* 所有的主任务都需要手动在main函数的while(1)中调用 */
        OS_RUN_TASK(os_test1);
    }
}

#elif 0     /* 信号量例子 */

OS_SEM_t sem_test;

OS_TASK(os_test2)
{
    OS_TASK_START(os_test2);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        OS_WAIT_SEM(sem_test, 1);
        printf("OS_WAIT_SEM get\n");
        OS_WAIT_SEMX(sem_test, 1, 20);
        if (sem_test == OS_SEM_TIMEOUT)
        {
            printf("OS_WAIT_SEMX 1 timeout\n");
        }
        else
        {
            printf("OS_WAIT_SEMX 1 ok\n");
        }
        OS_WAIT_SEMX(sem_test, 1, 300);
        if (sem_test == OS_SEM_TIMEOUT)
        {
            printf("OS_WAIT_SEMX 2 timeout\n");
        }
        else
        {
            printf("OS_WAIT_SEMX 2 ok\n");
        }
    }
    OS_TASK_END(os_test2);
}

OS_TASK(os_test1, void)
{
    OS_TASK_START(os_test1);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        printf("send sem\n");
        OS_SEND_SEM(sem_test);
        OS_TASK_WAITX(OS_SEC_TICKS * 2 / 10);
    }
    OS_TASK_END(os_test1);
}

void tmos_test_main(void)
{
    OS_INIT_TASKS();
    OS_INIT_SEM(sem_test);
    while (1)
    {
        /* 所有的主任务都需要手动在main函数的while(1)中调用 */
        OS_RUN_TASK(os_test1);
        OS_RUN_TASK(os_test2);
    }
}

#elif 0
/* ctimer task test. */
unsigned char i = 0, j = 0;

/*void参数表示函数无参数，可以不写该void，但是定义不标准，所以写上void最好 */
OS_TASK(os_test1, void)
{
    OS_TASK_START(os_test1);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        printf("os_test1\n");
        OS_TASK_WAITX(OS_SEC_TICKS * 6 / 10);
    }
    OS_TASK_END(os_test1);
}

/* 带参数任务编写格式函数参数直接作为宏定义中的成员写进去即可 */
OS_TASK(os_test2, unsigned char params, ...)
{
    OS_TASK_START(os_test2);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        printf("os_test2:%d\n", params);
        OS_TASK_WAITX(OS_SEC_TICKS * 6 / 10);
    }
    OS_TASK_END(os_test2);
}

/* ctimer task是可以重入的，ctimer的数量是可用于重入的参数数量 */
OS_CTIMER_TASK(ctimer_test)
{
    OS_CTIMER_TASK_START();
    while(1)
    {
        printf("i:%d, j:%d, ctimer:%d\n", i, j, *((unsigned char *)(p)));
        OS_CTIMER_TASK_WAITX(10); /* 延迟10个CTIMER_PERIOD_TICKS */
        j++;
    }
    OS_CTIMER_TASK_END();
}

void tmos_test_main(void)
{
    OS_INIT_TASKS();

    OS_CTIMER_INIT(ctimer_test1, ctimer_test, &i);
    OS_CTIMER_INIT(ctimer_test2, ctimer_test, &j);
    while (1)
    {
        /* 所有的主任务都需要手动在main函数的while(1)中调用 */
        OS_RUN_TASK(ctimer);
        OS_RUN_TASK(os_test1);
        OS_RUN_TASK(os_test2, i++);
    }
}

#elif 0
/* sub no time task test. */

OS_SUBNT(sub_test1, unsigned char i)
{
    OS_SUBNT_START();
    while((i%10)!=0)
    {
        printf("sub_test1\n");
        OS_SUBNT_WAITX(OS_SEC_TICKS / 2);
    }
    OS_SUBNT_END();
}

/*void参数表示函数无参数，可以不写该void，但是定义不标准，所以写上void最好 */
OS_TASK(os_test1, void)
{
    OS_TASK_START(os_test1);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        printf("os_test1\n");
        OS_TASK_WAITX(OS_SEC_TICKS * 6 / 10);
    }
    OS_TASK_END(os_test1);
}

/* 带参数任务编写格式函数参数直接作为宏定义中的成员写进去即可 */
OS_TASK(os_test2, unsigned char params, ...)
{
    OS_TASK_START(os_test2);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        printf("os_test2\n");
        OS_CALL_SUBNT(sub_test1, params);
    }
    OS_TASK_END(os_test2);
}

void tmos_test_main(void)
{
    unsigned char i = 0;
    OS_INIT_TASKS();
    while (1)
    {
        /* 所有的主任务都需要手动在main函数的while(1)中调用 */
        OS_RUN_TASK(os_test1);
        OS_RUN_TASK(os_test2, i++);
    }
}

#endif
