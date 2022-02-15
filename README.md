# Tiny Macro OS

## 简介

Tiny Macro OS是借鉴了protothread思想的宏定义调度内核。  
从各个论坛汲取代码汇总而成,  
主要来自于阿莫论坛的smset率先提出的小小调度器,后经过多人优化修改:  
[https://www.amobbs.com/thread-5508720-2-1.html](https://www.amobbs.com/thread-5508720-2-1.html)
本版本是个人在开发中结合自身使用习惯优化后的版本。

## ProtoThread机制

Protothread是专为资源有限的系统设计的一种耗费资源特别少并且不使用堆栈的线程模型，其特点是：

1.以纯C语言实现，无硬件依赖性；  
2.极少的资源需求，每个Protothread仅需要2个额外的字节；  
3.可以用于有操作系统或无操作系统的场合；  
4.支持阻塞操作且没有栈的切换。

使用Protothread实现多任务的最主要的好处在于它的轻量级。  
每个Protothread不需要拥有自已的堆栈，所有的Protothread共享同一个堆栈空间，对于RAM资源有限的系统尤为有利。  
相对于操作系统下的多任务而言，每个任务都有自已的堆栈空间，这将消耗大量的RAM资源，而每个Protothread仅使用一个变量保存当前函数状态。  

## 时间轮询机制

时间轮询机制是一种比较简单易用的系统架构之一，它对于系统中的任务调度算法是分时处理。核心思路是把 CPU 的时间分时给各个任务使用。  
需要注意的是，这种方法的要保证每个任务都是短小精悍的，要不然一个任务执行的时间过长，那其它任务就无法保证按它预设的时间来执行。  

## tiny macro os

tiny macro os是结合了ProtoThread机制和时间轮询机制的调度内核，每个任务都有单独的时间变量和函数状态变量。  
时间变量为全局变量，在软硬件定时器中更新。  
函数状态变量为函数内部局部静态变量。只在函数内使用。  

## 移植

移植首先需要在tiny-macro-os.h中修改下列宏定义：

```c
#define TINY_MACRO_OS_TASKS_NUM           3U                //定义使用的主任务数量，最大255个任务
#define TINY_MACRO_OS_TIME_t              unsigned short    //定义时间计数变量的类型，根据最长延迟修改
#define TINY_MACRO_OS_LINE_t              unsigned short    //定义任务切换记录变量的类型，根据最大函数占用行数修改
#define os_SEM_t                          signed short      //信号量类型声明，必须为signed类型
#define os_SEC_TICKS                      1000              //定时器时钟更新频率，每秒钟多少个ticks
```

然后需要实现一个定时器更新时间变量：

```c
void SysTick_Handler(void)
{
    os_UpdateTimers();
}
```

## 使用例子

无参数任务：

```c
os_task os_task_test1(void)
{
    os_task_boot();
    os_task_start();
    static uint8_t i = 0;//任务中定义的会在任务切换前后都使用的局部变量需要使用static定义，不然变量会丢失
    //禁止在os_task_start和os_task_end中使用switch和return;
    while(1) {
        printf("os test1\n");
        os_task_WaitX(1000);
    }
    os_task_end();
}
```

带参数任务：

```c
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
```

任务循环调用：

```c
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
```

信号量：

```c
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
```

子任务：

```c
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
```

## [博客主页](https://blog.maxiang.vip/)
