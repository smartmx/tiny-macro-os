/*
 * Copyright (c) 2022-2023, smartmx - smartmx@qq.com
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
 * <https://github.com/smartmx/tiny-macro-os>
 *
 */

#include "ctimer.h"

/* callback timer tasks params for tiny-macro-os. */
ctimer_t TASK_CTIMERS[CTIMER_MAX_NUM];

#if COMPILER_SUPPORT_VA_ARGS
extern OS_TASK(os_ctimer, void);
#else
extern OS_TASK(os_ctimer);
#endif
{
    unsigned char i;
    OS_TASK_START(os_ctimer);
    {
        OS_TASK_WAITX(CTIMER_PERIOD_TICKS);
        for (i = 0; i < CTIMER_MAX_NUM; i++)
        {
            if (TASK_CTIMERS[i].ticks != TINY_MACRO_OS_TIME_MAX)
            {
                if (TASK_CTIMERS[i].ticks == 0)
                {
                    if (TASK_CTIMERS[i].f != NULL)
                    {
                        TASK_CTIMERS[i].ticks = TASK_CTIMERS[i].f(&TASK_CTIMERS[i].line, TASK_CTIMERS[i].ptr);
                    }
                }
                else
                {
                    TASK_CTIMERS[i].ticks--;
                }
            }
        }
        OS_TASK_CWAITX(CTIMER_PERIOD_TICKS);
    }
    OS_TASK_END(os_ctimer);
}
