# tiny macro os

## 简介

Tiny Macro OS是借鉴了protothread和时间片轮转思想的宏定义调度内核。  
从各个论坛汲取代码汇总而成,  
本版本是个人在开发中结合自身使用习惯优化后的版本。

## ProtoThread机制

Protothread是专为资源有限的系统设计的一种耗费资源特别少并且不使用堆栈的线程模型，其特点是：

- 以纯C语言实现，无硬件依赖性；  
- 极少的资源需求，每个Protothread最少仅需要2个额外的字节；  
- 可以用于有操作系统或无操作系统的场合；  
- 支持阻塞操作且没有栈的切换。

使用Protothread实现多任务的最主要的好处在于它的轻量级。  
每个Protothread不需要拥有自已的堆栈，所有的Protothread共享同一个堆栈空间，对于RAM资源有限的系统尤为有利。  
相对于操作系统下的多任务而言，每个任务都有自已的堆栈空间，这将消耗大量的RAM资源，而每个Protothread仅使用一个变量保存当前函数状态。  

## 时间轮询机制

时间轮询机制是一种比较简单易用的系统架构之一，它对于系统中的任务调度算法是分时处理。核心思路是把 CPU 的时间分时给各个任务使用。  
需要注意的是，这种方法的要保证每个任务都是短小精悍的，要不然一个任务执行的时间过长，那其它任务就无法保证按它预设的时间来执行。  

## tiny macro os任务调度机制

tiny macro os是结合了ProtoThread机制和时间轮询机制的调度内核，每个任务都有单独的时间变量和函数状态变量。  
时间变量为全局变量，在软硬件定时器中更新。  
函数状态变量为全局变量，用于保存函数目前运行的位置。  
目的只为极度精简，并且可为51所用。
任务分主任务和子任务，所以不采用链表管理任务，需要手动在主循环中添加运行主任务，子任务在主任务中调用。

## 移植

移植首先需要根据自己的需求在tiny-macro-os.h中修改下列宏定义：

```c
#define TINY_MACRO_OS_TIME_t                        unsigned short    /* 定义时间计数变量的类型，根据最长延迟修改 */
#define TINY_MACRO_OS_LINE_t                        unsigned short    /* 定义任务切换记录变量的类型，根据最大函数占用行数修改 */
#define OS_SEM_t                                    signed short      /* 信号量类型声明，必须为signed类型 */
#define OS_SEC_TICKS                                1000              /* 定时器时钟更新频率，每秒钟多少个ticks */
```

`TINY_MACRO_OS_LINE_t`根据系统中所有TASK的最大函数行数决定，如果最大行数都不超过255，那么可以设为`unsigned char`

`TINY_MACRO_OS_TIME_t`是系统中任务时间计算的变量类型，决定了任务可以延迟的最大TICKS数量，例如`unsigned char`最大为255，时钟更新频率为`10ms`，则最大延迟时间为`255*10ms`

`OS_SEM_t`是信号量类型定义，信号量类型必须为signed，其类型的最大值决定了在使用信号量超时判断时的最大判断次数，和信号量检测时间一起决定了信号量超时时间，可以通过判断信号量是否为`-1`来判断是否超时

移植需要实现一个定时器更新时间变量，定时器周期需要是`1s / OS_SEC_TICKS`：

```c
void SysTick_Handler(void)
{
    OS_UPDATE_TIMERS();
}
```

## 任务定义

所有任务task都需要在下面的enum枚举中增加自己的名字，`TINY_MACRO_OS_TASKS_MAX_NUM`不可修改或删除，必须保留，它是用来定义变量长度必要的常量，用户自己添加的task名在它前面写入即可。

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

任务定义采用了可变参数宏定义，任务携带的参数类型随意，数量不限。

该定义方法巧妙运用了编译器的编译顺序：宏定义展开->枚举变量替换，从而实现了用一个枚举变量的名字当作OS调度的识别和操作任务参数的数组下标。

在任务中通过OS_TASK_START(NAME)定义，将数组下标变为局部枚举变量`_task_name`，实现函数内部对函数状态和时间的操作，所以函数内部不可以定义和使用变量`_task_name`。

### 无参数任务

void参数表示函数无参数，可以不写该void，但是定义不标准，所以写上void最好

```c
OS_TASK(OS_TASK_TEST1, void)
{
    OS_TASK_START(OS_TASK_TEST1);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        printf("OS_TASK_TEST1\n");
        OS_TASK_WAITX(OS_SEC_TICKS * 6 / 10);
    }
    OS_TASK_END(OS_TASK_TEST1);
}
```

上述任务定义展开后为

```c
unsigned short (OS_TASK_TEST1_task)( void )
{
    enum
    {
        _task_name = OS_TASK_TEST1
    };
    switch( os_task_linenums[(OS_TASK_TEST1)] )
    {
        case 0U:
            ;
            while(1)
            {
                printf("OS_TASK_TEST1\n");
                os_task_linenums[(_task_name)]=(unsigned short)(54)%((unsigned short)(0xffffffff))+1U;
                if(os_task_linenums[(_task_name)])
                {
                    return(1000 * 6 / 10);
                }
                break;
        case ((unsigned short)(54)%((unsigned short)(0xffffffff)))+1U:
            ;
            }
            break;
        default:
            break;
    }
    os_task_linenums[(OS_TASK_TEST1)] = 0U;
    return ((unsigned short) (0xffffffff));
}
```

### 带参数任务

带参数任务函数编写时将函数参数直接作为宏定义中的成员写进去即可

```c
OS_TASK(OS_TASK_TEST2, unsigned char params, ...)
{
    OS_TASK_START(OS_TASK_TEST2);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        printf("OS_TASK_TEST2:%d\n", params);
        OS_TASK_WAITX(OS_SEC_TICKS * 6 / 10);
    }
    OS_TASK_END(OS_TASK_TEST2);
}
```

上述任务定义展开后为

```c
unsigned short (OS_TASK_TEST2_task)( unsigned char params, ... )
{
    enum
    {
        _task_name = OS_TASK_TEST2
    };
    switch( os_task_linenums[(OS_TASK_TEST2)] )
    {
        case 0U:
            ;
            while(1)
            {
                printf("OS_TASK_TEST2:%d\n", params);
                os_task_linenums[(_task_name)]=(unsigned short)(67)%((unsigned short)(0xffffffff))+1U;
                if(os_task_linenums[(_task_name)])
                {
                    return(1000 * 6 / 10);
                }
                break;
        case ((unsigned short)(67)%((unsigned short)(0xffffffff)))+1U:
            ;
            }
            break;
        default:
            break;
    }
    os_task_linenums[(OS_TASK_TEST2)] = 0U;
    return ((unsigned short) (0xffffffff));
}
```

### 任务需要在主函数中循环调用

```c
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
```

## 使用例子

### 任务延迟、条件等待例子

```c
unsigned char params_test;

/*void参数表示函数无参数，可以不写该void，但是定义不标准，所以写上void最好 */
OS_TASK(OS_TASK_TEST1, void)
{
    OS_TASK_START(OS_TASK_TEST1);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        /* 每0.1秒params_test加1 */
        printf("++\n");
        OS_TASK_WAITX(OS_SEC_TICKS / 10);
        params_test++;
    }
    OS_TASK_END(OS_TASK_TEST1);
}

/* 带参数任务编写格式函数参数直接作为宏定义中的成员写进去即可 */
OS_TASK(OS_TASK_TEST2, unsigned char params, ...)
{
    /* OS_TASK_WAIT_UNTILX需要一个常驻变量保存时间信息，可以是全局变量或局部静态变量 */
    static unsigned char timecnt;
    OS_TASK_START(OS_TASK_TEST2);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        /*每1个tick检测一次params是否可以被10整除*/
        OS_TASK_WAIT_UNTIL((params % 10) == 0, 1);
        printf("OS_TASK_TEST2:%d\n", params);
        /*每OS_SEC_TICKS / 10个tick检测一次params是否可以被10整除，检测10次超时*/
        OS_TASK_WAIT_UNTILX((params % 100) == 0, OS_SEC_TICKS / 10, 10, timecnt);
        printf("OS_TASK_TEST2:%d,cnt:%d\n", params, timecnt);
        OS_TASK_WAITX(OS_SEC_TICKS / 10);
    }
    OS_TASK_END(OS_TASK_TEST2);
}

void tmos_test_main(void)
{
    OS_INIT_TASKS();
    params_test = 0;
    while (1)
    {
        /* 所有的主任务都需要手动在main函数的while(1)中调用 */
        OS_RUN_TASK(OS_TASK_TEST1);
        OS_RUN_TASK(OS_TASK_TEST2, params_test);
    }
}
```

### 任务挂起、复位、延迟、重启

```c
OS_TASK(OS_TASK_TEST2)
{
    OS_TASK_START(OS_TASK_TEST2);
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
    OS_TASK_END(OS_TASK_TEST2);
}

OS_TASK(OS_TASK_TEST1, void)
{
    OS_TASK_START(OS_TASK_TEST1);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        printf("task1 start\n");
        OS_TASK_WAITX(OS_SEC_TICKS * 3);

        printf("suspend task2\n");
        /*挂起任务，保留任务状态*/
        OS_TASK_SUSPEND_ANOTHER(OS_TASK_TEST2);
        OS_TASK_WAITX(OS_SEC_TICKS * 3);
        printf("restart task2\n");
        /* 必须已经停止的任务才可以重启，不然无效 */
        OS_TASK_RESTART_ANOTHER(OS_TASK_TEST2, 0);
        OS_TASK_WAITX(OS_SEC_TICKS * 3);

        printf("exit task2\n");
        /*退出任务，清除任务状态*/
        OS_TASK_EXIT_ANOTHER(OS_TASK_TEST2);
        OS_TASK_WAITX(OS_SEC_TICKS * 3);
        printf("call task2\n");
        OS_TASK_CALL_ANOTHER(OS_TASK_TEST2);
        OS_TASK_WAITX(OS_SEC_TICKS * 3);
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
```

### 子任务

主任务调用子任务后，会先切换出去执行其他任务，到下一次执行主任务时，不再运行主任务相关程序，开始执行子任务相关程序。

直到子任务退出，才会继续运行主任务程序。

```c
/* 子任务不退出，查看效果 */
OS_TASK(OS_TASK_TEST2)
{
    OS_TASK_START(OS_TASK_TEST2);
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
    OS_TASK_END(OS_TASK_TEST2);
}

OS_TASK(OS_TASK_TEST1, void)
{
    OS_TASK_START(OS_TASK_TEST1);
    /* 禁止在OS_TASK_START和OS_TASK_END之间使用switch */
    while (1)
    {
        printf("father task\n");
        OS_TASK_WAITX(OS_SEC_TICKS * 2 / 10);
        /* 如果需要每次开始执行子任务时都需要子任务从头开始执行，在OS_CALL_SUB之前使用OS_TASK_RESET_ANOTHER复位子任务 */
        OS_TASK_RESET_ANOTHER(OS_TASK_TEST2);
#if 0
        /* 没有超时时间 */
        OS_CALL_SUB(OS_TASK_TEST2);
#else
        /* 有超时时间 */
        OS_CALL_SUBX(OS_SEC_TICKS * 5, OS_TASK_TEST2);
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
```

### 信号量

```c
OS_SEM_t sem_test;

OS_TASK(OS_TASK_TEST2)
{
    OS_TASK_START(OS_TASK_TEST2);
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
        OS_TASK_WAITX(OS_SEC_TICKS * 2 / 10);
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

## 注意事项

宏定义中参数为`NAME`的，必须为自身任务的NAME

宏定义中参数为`ANAME`的，参数必须为另一个任务的NAME

宏定义中参数为`SUBNAME`的，该SUBNAME代表的子任务必须没有在主函数中循环执行

## [博客主页](https://blog.maxiang.vip/)
