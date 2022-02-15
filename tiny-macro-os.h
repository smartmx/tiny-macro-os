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
#ifndef _TINY_MACRO_OS_H_
#define _TINY_MACRO_OS_H_

/****TINY_MACRO_OS CONFIG*********************************************************************************/
#define TINY_MACRO_OS_TASKS_NUM           3U                //定义使用的主任务数量，最大255个任务
#define TINY_MACRO_OS_TIME_t              unsigned short    //定义时间计数变量的类型，根据最长延迟修改
#define TINY_MACRO_OS_LINE_t              unsigned short    //定义任务切换记录变量的类型，根据最大函数占用行数修改
#define os_SEM_t                          signed short      //信号量类型声明，必须为signed类型
#define os_SEC_TICKS                      1000              //定时器时钟更新频率，每秒钟多少个ticks

/****TINY_MACRO_OS 调度系统开始*********************************************************************************/
/*
 *  调度器通过使用os_task_start和os_task_end的任务函数中不可以使用switch case，但是可以通过调用函数，在另一个非任务函数中使用switch case。
 *  一定注意，下面os的宏定义不能有任何的换行修改，必须是一整行！！！！！！
 */

extern volatile TINY_MACRO_OS_TIME_t os_task_timers[TINY_MACRO_OS_TASKS_NUM];      //所有任务的时间变量值。

//如果编译器不能正确的返回最大值，应该将该宏手动修改，uint8_t->255，uint16_t->65535，uint32_t->4294967295
#define TINY_MACRO_OS_TIME_MAX            (TINY_MACRO_OS_TIME_t)(-1) //最大时间计数值

#define TINY_MACRO_OS_LINE_MAX            (TINY_MACRO_OS_LINE_t)(-1) //最大函数行数计数值

#define os_task         TINY_MACRO_OS_TIME_t                              //所有任务用其定义

#define os_task_boot()      static TINY_MACRO_OS_LINE_t os_task_lc=0U; //任务函数初始化，在os_task_boot和os_task_start之间可以通过带参数运行函数，进行任务复位等操作

#define os_task_start()        switch(os_task_lc){case 0U:       //所有任务开始之前

#define os_task_end()          break; default:break;}  os_task_lc=0U;  return TINY_MACRO_OS_TIME_MAX     //所有任务结束之前

#define os_task_Reset()        do{os_task_lc=0;}while(0); //任务从头开始执行

#define os_task_WaitX(ticks)   os_task_lc=(TINY_MACRO_OS_LINE_t)(__LINE__)%TINY_MACRO_OS_LINE_MAX+1U;  if(os_task_lc){return (ticks);}  break; case ((TINY_MACRO_OS_LINE_t)(__LINE__)%TINY_MACRO_OS_LINE_MAX)+1U:  //等待时间 单位时间为中断里定义的系统最小时间

#define os_task_CWaitX(ticks)    return ticks   //等待时间，不改变上一次函数跳转位置

#define os_task_Yield()      os_task_WaitX(0)      //跳出当前任务

#define os_task_Exit()        do{os_task_lc=0U;}while(0); return TINY_MACRO_OS_TIME_MAX     //停止并且再不运行运行当前任务

#define os_task_Suspend()      os_task_WaitX(TINY_MACRO_OS_TIME_MAX)       //挂起当前任务

#define os_task_Suspend_Another(TaskName,TaskID)        do{os_task_timers[(TaskID)]=TINY_MACRO_OS_TIME_MAX;}while(0);     //挂起另一个指定任务，不可用于自身

#define os_task_Restart_Now(TaskName,TaskID)    do {if(os_task_timers[(TaskID)]==TINY_MACRO_OS_TIME_MAX){os_task_timers[(TaskID)]=(TaskName)();} } while(0)     //重新启动一个已经停止运行的任务，立刻运行任务

#define os_task_Restart_Later(TaskName,TaskID,ticks) do {if(os_task_timers[(TaskID)]==TINY_MACRO_OS_TIME_MAX){os_task_timers[(TaskID)]=ticks;}} while(0) //在指定时间后重新启动任务

#define os_task_Restart(TaskName,TaskID)    os_task_Restart_Later(TaskName,TaskID,0)    //在下一次任务轮询时启动任务

#define os_task_Ticks_Update(TaskName,TaskID,ticks) do {os_task_timers[(TaskID)]=ticks;} while(0)   //更新任务时间，以便突发事件运行任务

#define os_task_WaitUntil(C,ticks) do{ os_task_WaitX(ticks); if(!(C)) return ticks;} while(0) //等待条件 C 满足再继续向下执行，查询频率为每ticks个时钟一次

#define os_RunTask(TaskName,TaskID)  do {if (os_task_timers[(TaskID)]==0U){os_task_timers[(TaskID)]=(TaskName)(); } } while(0)   //运行任务，在主程序的while（1）里调用

#define os_RunHpTask(TaskName,TaskID) { if (os_task_timers[(TaskID)]==0U) {os_task_timers[(TaskID)]=(TaskName)(); continue;} }    //High Priority高优先级运行任务，只带一个参数，为了兼容51单片机，不使用可变参数。该任务返回的时间值不能一直为0，必须有等待时间让出使用权，不然下面的其他任务将无法继续运行。

#define os_RunTaskWithParam(TaskName,TaskID,param)  do {if (os_task_timers[(TaskID)]==0U){os_task_timers[(TaskID)]=(TaskName)(param); } } while(0)   //带参数运行任务，在主程序的while（1）里调用

#define os_RunHpTaskWithParam(TaskName,TaskID,param) { if (os_task_timers[(TaskID)]==0U) {os_task_timers[(TaskID)]=(TaskName)(param); continue;} }    //High Priority高优先级运行任务，带一个参数

#define os_CallTask(TaskName,TaskID) do {os_task_timers[(TaskID)]=(TaskName)();} while(0) //运行任务，不管任务时间状态立刻运行。

#define os_CallTaskWithParam(TaskName,TaskID,param)  do {os_task_timers[(TaskID)]=(TaskName)(param);} while(0)   //带参数运行任务，不管任务时间状态立刻运行。

#define os_CallSub(SubTaskName) do { os_task_lc=(TINY_MACRO_OS_LINE_t)(__LINE__)%TINY_MACRO_OS_LINE_MAX+1U; if(os_task_lc) {return 0U;} break; case (TINY_MACRO_OS_LINE_t)(__LINE__)%TINY_MACRO_OS_LINE_MAX+1U:{ TINY_MACRO_OS_TIME_t subtasktime; subtasktime=(SubTaskName)(); if(subtasktime!=TINY_MACRO_OS_TIME_MAX) {return subtasktime;}}} while(0) //在任务中调用的子任务，在子任务结束之前不会继续主任务，即子任务占用了主任务的系统时间变量使用权

#define os_CallSubWithParam(SubTaskName, param) do { os_task_lc=(TINY_MACRO_OS_LINE_t)(__LINE__)%TINY_MACRO_OS_LINE_MAX+1U; if(os_task_lc) {return 0U;} break; case (TINY_MACRO_OS_LINE_t)(__LINE__)%TINY_MACRO_OS_LINE_MAX+1U: {TINY_MACRO_OS_TIME_t subtasktime; subtasktime=(SubTaskName)(param); if(subtasktime!=TINY_MACRO_OS_TIME_MAX) {return subtasktime;}}} while(0) //在任务中调用的子任务（带参数）

#define os_InitTasks() do {unsigned char i; for(i=0U;i<TINY_MACRO_OS_TASKS_NUM ;i++) {os_task_timers[i]=0U;} } while(0) //初始化任务，每个任务接下来都会被执行

#define os_UpdateTimers() do {unsigned char i; for(i=0U;i<TINY_MACRO_OS_TASKS_NUM ;i++){  if(os_task_timers[i]!=0U){  if(os_task_timers[i]!=TINY_MACRO_OS_TIME_MAX) { os_task_timers[i]--;}}}} while(0) //更新系统时间，该函数应该放入定时器中断中处理

#define os_TIMEOUT      (-1) //超时

#define os_InitSem(sem) (sem)=0;  //初始化信号量

#define os_WaitSem(sem,ticks) do{ (sem)=1; os_task_WaitX(ticks); if ((sem)>0) return ticks;} while(0) //等待信号量，查询频率为每ticks个时钟一次

#define os_WaitSemX(sem,overtimes,ticks)  do { (sem)=(overtimes)+1; os_task_WaitX(ticks); if((sem)>1){ (sem)--;  return ticks;} else{(sem)=os_TIMEOUT;} } while(0)  //等待信号量，并有超时次数，查询频率为每ticks个时钟一次，超时时间为ticks*overtimes，所以ticks最好不要为0

#define os_SendSem(sem)  do {(sem)=0;} while(0) //发送信号量

#endif /* _TINY_MACRO_OS_H_ */
