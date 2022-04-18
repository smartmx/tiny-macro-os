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

#ifndef _CTIMER_H_
#define _CTIMER_H_

#include "tiny-macro-os.h"

/* 如果准备使用ctimer，需要在tiny-macro-os.h中的任务枚举中添加ctimer，并在主任务循环中，添加ctimer任务调用OS_RUN_TASK(ctimer)。 */

/* 准备使用几个ctimer就在这里加上枚举 */
enum
{
    ctimer_test1 = 0,
    ctimer_test2,
    CTIMER_MAX_NUM,
};

#define CTIMER_PERIOD_TICKS     (OS_SEC_TICKS / 100) /* ctimer更新自身时间 */

/* callback timer */
typedef struct _ctimer_struct
{
    TINY_MACRO_OS_TIME_t (*f)(TINY_MACRO_OS_LINE_t *, void *);
    void *ptr;
    TINY_MACRO_OS_TIME_t ticks;
    TINY_MACRO_OS_LINE_t line;
} ctimer_t;

extern ctimer_t os_ctimers[CTIMER_MAX_NUM];

/* 初始化ctimer参数 */
#define OS_CTIMER_INIT(NAME, FUNC, PTR)                     do{os_ctimers[NAME].f=(FUNC);os_ctimers[NAME].ptr=(PTR);os_ctimers[NAME].ticks=0;os_ctimers[NAME].line=0;}while(0)

/* ctimer任务定义，和OS_TASK不同，必须不可和枚举变量名称一样 */
#define OS_CTIMER_TASK(FUNC)                                TINY_MACRO_OS_TIME_t (FUNC)(TINY_MACRO_OS_LINE_t *lc, void *p)

/* 函数任务系统调度开始定义 */
#define OS_CTIMER_TASK_START()                              switch(*lc){case 0U:

/* 函数任务系统调度结束定义 */
#define OS_CTIMER_TASK_END()                                break;}(*lc)=0;return (TINY_MACRO_OS_TIME_MAX)

/* 函数任务系统调度结束定义 */
#define OS_CTIMER_TASK_RESTART()                            do{(*lc)=0;return 0;}while(0)

/* 退出当前任务，并等待对应时间，保存当前运行位置，时间单位CTIMER_PERIOD_TICKS。 */
#define OS_CTIMER_TASK_WAITX(TICKS)                         do{(*lc)=(((TINY_MACRO_OS_LINE_t)(__LINE__)%(TINY_MACRO_OS_LINE_MAX))+1U);return(TICKS);case (((TINY_MACRO_OS_LINE_t)(__LINE__)%(TINY_MACRO_OS_LINE_MAX))+1U):;}while(0)

/* 跳出当前任务，保存当前运行位置，等下一次执行时继续运行 */
#define OS_CTIMER_TASK_YIELD()                              OS_CTIMER_TASK_WAITX(0)

/* 挂起当前任务 */
#define OS_CTIMER_TASK_SUSPEND()                            OS_CTIMER_TASK_WAITX(TINY_MACRO_OS_TIME_MAX)

/* 复位当前任务，在指定时间后开始下一次运行，并从头开始 */
#define OS_CTIMER_TASK_RESET(TICKS)                         do{(*lc)=0;}while(0);return (TICKS)

/* 等待条件 C 满足再继续向下执行，查询频率为每ticks个时钟一次 */
#define OS_CTIMER_TASK_WAIT_UNTIL(C, TICKS)                 do{OS_CTIMER_TASK_WAITX(TICKS); if(!(C)) return (TICKS);} while(0)

/* 等待条件 C 满足再继续向下执行，查询频率为每ticks个时钟一次，带有超时次数，需要用户自己提供一个变量进行超时计算，不可为局部变量，必须为全局变量或者函数内静态变量 */
#define OS_CTIMER_TASK_WAIT_UNTILX(C, TICKS, TIMES, VAR)    do{(VAR)=(TIMES);OS_CTIMER_TASK_WAITX((TICKS));if(!(C)&&((VAR)>0)){(VAR)--;return (TICKS);}} while(0)

/*************************************信号量*******************************************/

/* 等待信号量，查询频率为每ticks个时钟一次 */
#define OS_CTIMER_WAIT_SEM(SEM, TICKS)                      do{(SEM)=1;OS_CTIMER_TASK_WAITX(TICKS);if((SEM)>0){return (TICKS);}}while(0)

/* 等待信号量，并有超时次数，查询频率为每ticks个时钟一次，超时时间为ticks*TIMES，所以ticks最好不要为0 */
#define OS_CTIMER_WAIT_SEMX(SEM, TICKS, TIMES)              do{(SEM)=(TIMES)+1;OS_CTIMER_TASK_WAITX(TICKS); if((SEM)>1){(SEM)--;return (TICKS);}else{if((SEM)!=0){(SEM)=(OS_SEM_TIMEOUT);}}}while(0)

/*************************************以下函数只可以在其他任务函数中调用，不可以在自身函数中调用*******************************/
/* 挂起另一个指定任务，不可挂起自身 */
#define OS_CTIMER_TASK_SUSPEND_ANOTHER(ANAME)               do{os_ctimers[(ANAME)].ticks=(TINY_MACRO_OS_TIME_MAX);}while(0)

/* 复位另一个任务，在下次运行时从头开始，可以和OS_CTIMER_TASK_CALL_ANOTHER配合使用来立刻重新开始运行另一个任务 */
#define OS_CTIMER_TASK_RESET_ANOTHER(ANAME)                 do{os_ctimers[(ANAME)].line=0;}while(0)

/* 在指定时间后重新启动另一个已经停止运行的任务*/
#define OS_CTIMER_TASK_RESTART_ANOTHER(ANAME, TICKS)        do{if(os_ctimers[(ANAME)].ticks==(TINY_MACRO_OS_TIME_MAX)){os_ctimers[(ANAME)].ticks=(TICKS);}}while(0)

/* 更新另一个任务的时间，以便突发事件运行任务 */
#define OS_CTIMER_TASK_UPDATE_ANOTHER(ANAME, TICKS)         do{os_ctimers[(ANAME)].ticks=(TICKS);}while(0)

/* 停止并且再不运行运行另一个任务，复位任务状态，下一次运行从头开始。 */
#define OS_CTIMER_TASK_EXIT_ANOTHER(ANAME)                  do{os_ctimers[(ANAME)].line=0U;os_ctimers[(ANAME)].ticks=(TINY_MACRO_OS_TIME_MAX);}while(0)

/* 运行任务，不管任务时间状态立刻调用任务函数运行。 */
#define OS_CTIMER_TASK_CALL_ANOTHER(ANAME, ...)             do{os_ctimers[ANAME].ticks=os_ctimers[ANAME].f(&os_ctimers[ANAME].line,os_ctimers[ANAME].ptr);}while(0)

/*在主任务中需要循环添加的任务*/
extern OS_TASK(ctimer, void);

#endif /* _CTIMER_H_ */
