/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "hal_export.h"
#include "stm32f1xx_hal.h"



void HAL_Printf(_IN_ const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    fflush(stdout);
}

int HAL_Snprintf(_IN_ char *str, const int len, const char *fmt, ...)
{
    va_list args;
    int rc;

    va_start(args, fmt);
    rc = vsnprintf(str, len, fmt, args);
    va_end(args);

    return rc;
}

int HAL_Vsnprintf(_IN_ char *str, _IN_ const int len, _IN_ const char *format, va_list ap)
{
    return vsnprintf(str, len, format, ap);
}


void HAL_DelayUs(_IN_ uint32_t us)
{
    HAL_Delay(us);
}

uint32_t HAL_GetTimeMs(void)
{	
    return HAL_GetTick();
}

void HAL_DelayMs(_IN_ uint32_t ms)
{
   (void)HAL_Delay(ms);
}

#ifdef OS_USED
void hal_thread_create(volatile void* threadId, uint16_t stackSize, int Priority, void (*fn)(void*), void* arg)
{
	osThreadDef(parseTask, (os_pthread)fn, (osPriority)Priority, 0, stackSize);
	threadId = osThreadCreate(osThread(parseTask), arg);
	(void)threadId; //Eliminate warning
}


void hal_thread_destroy(void* threadId)
{
	osThreadTerminate(threadId);
}

void HAL_SleepMs(_IN_ uint32_t ms)
{
   (void)osDelay(ms);
}

void *HAL_Malloc(_IN_ uint32_t size)
{
	return pvPortMalloc( size);
}

void HAL_Free(_IN_ void *ptr)
{
    vPortFree(ptr);
}

void *HAL_MutexCreate(void)
{
	return (void *)osMutexCreate (NULL);
}


void HAL_MutexDestroy(_IN_ void * mutex)
{
	osStatus ret;
	
    if(osOK != (ret = osMutexDelete((osMutexId)mutex)))
    {
		HAL_Printf("HAL_MutexDestroy err, err:%d\n\r",ret);
	}
}

void HAL_MutexLock(_IN_ void * mutex)
{
	osStatus ret;

	if(osOK != (ret = osMutexWait((osMutexId)mutex, osWaitForever)))
	{
		HAL_Printf("HAL_MutexLock err, err:%d\n\r",ret);
	}
}

void HAL_MutexUnlock(_IN_ void * mutex)
{
	osStatus ret;

	if(osOK != (ret = osMutexRelease((osMutexId)mutex)))
	{
		HAL_Printf("HAL_MutexUnlock err, err:%d\n\r",ret);
	}	
}
#else
void hal_thread_create(void** threadId, void (*fn)(void*), void* arg)
{

}

void HAL_SleepMs(_IN_ uint32_t ms)
{

}

void *HAL_Malloc(_IN_ uint32_t size)
{
	return NULL;
}

void HAL_Free(_IN_ void *ptr)
{
   
}

void *HAL_MutexCreate(void)
{
	return NULL;
}


void HAL_MutexDestroy(_IN_ void* mutex)
{
	
}

void HAL_MutexLock(_IN_ void* mutex)
{

}

void HAL_MutexUnlock(_IN_ void* mutex)
{

}

#endif





