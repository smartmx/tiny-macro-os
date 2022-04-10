# Tiny Macro OS

## 简介

Tiny Macro OS是借鉴了protothread和时间片轮转思想的宏定义调度内核。  
从各个论坛汲取代码汇总而成,  
本版本是个人在开发中结合自身使用习惯优化后的版本。

## ProtoThread机制

Protothread是专为资源有限的系统设计的一种耗费资源特别少并且不使用堆栈的线程模型，其特点是：

1.以纯C语言实现，无硬件依赖性；  
2.极少的资源需求，每个Protothread最少仅需要2个额外的字节；  
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
#define TINY_MACRO_OS_TIME_t                        unsigned short    /* 定义时间计数变量的类型，根据最长延迟修改 */
#define TINY_MACRO_OS_LINE_t                        unsigned short    /* 定义任务切换记录变量的类型，根据最大函数占用行数修改 */
#define OS_SEM_t                                    signed short      /* 信号量类型声明，必须为signed类型 */
#define OS_SEC_TICKS                                1000              /* 定时器时钟更新频率，每秒钟多少个ticks */
```

然后需要实现一个定时器更新时间变量：

```c
void SysTick_Handler(void)
{
    OS_UPDATE_TIMERS();
}
```

## 任务定义

所有任务task都需要在下面的enum枚举中增加自己的名字，`TINY_MACRO_OS_TASKS_MAX_NUM`不可修改，必须保留，它是用来定义变量长度必要的常量。

```c
/****TINY_MACRO_OS TASKS DECLARE**************************************************************************/
/* 将tiny macro os任务函数的识别名字放到这里，后续使用都是从这里使用。自己每次创建任务都需要在这里添加 */
enum
{
    OS_TASK_TEST1 = 0,              /* 替换成自己的任务函数识别文字，如使用OS_TASK_DEFAULT，则任务函数名为OS_TASK_DEFAULT_task */
    OS_TASK_TEST2,
    TINY_MACRO_OS_TASKS_MAX_NUM,    /* 定义使用的主任务数量，最大255个任务 */
};
```

### 无参数任务

```c
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
```

### 带参数任务

```c
OS_TASK(OS_TASK_TEST2, unsigned char params)
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
```

### 任务需要在主函数中循环调用

```c
void main()
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
```

## 使用例子

### 信号量

```c
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
```

### 子任务

```c
#if 0
/* 子任务退出，查看效果 */
OS_TASK(OS_TASK_TEST2)
{
    OS_TASK_START(OS_TASK_TEST2);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    printf("child task 1\n");
    OS_TASK_WAITX(OS_TASK_TEST2, OS_SEC_TICKS * 3 / 10);
    printf("child task 2\n");
    OS_TASK_WAITX(OS_TASK_TEST2, OS_SEC_TICKS * 4 / 10);
    printf("child task 3\n");
    OS_TASK_WAITX(OS_TASK_TEST2, OS_SEC_TICKS * 5 / 10);
    OS_TASK_END(OS_TASK_TEST2);
}
#else
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
#endif

OS_TASK(OS_TASK_TEST1, void)
{
    OS_TASK_START(OS_TASK_TEST1);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        printf("father task\n");
        OS_TASK_WAITX(OS_TASK_TEST1, OS_SEC_TICKS * 2 / 10);
        OS_CALL_SUB(OS_TASK_TEST1, OS_TASK_TEST2);
    }
    OS_TASK_END(OS_TASK_TEST1);
}

void tmos_test_main(void)
{
    OS_INIT_TASKS();
    unsigned char i = 0;
    while (1)
    {
        /* 所有的主任务都需要手动在main函数的while(1)中调用 */
        OS_RUN_TASK(OS_TASK_TEST1);
    }
}
```

## [博客主页](https://blog.maxiang.vip/)
