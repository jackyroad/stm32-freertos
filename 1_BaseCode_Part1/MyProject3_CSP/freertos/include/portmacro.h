#ifndef PORTMACRO_H
#define PORTMACRO_H

#include "stdint.h"
#include "stddef.h"

/* 数据类型重定义 */
#define portCHAR char
#define portFLOAT float
#define portDOUBLE double
#define portLONG long
#define portSHORT short
#define portSTACK_TYPE uint32_t
#define portBASE_TYPE long

typedef portSTACK_TYPE StackType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;

#if (configUSE_16_BIT_TICKS == 1)
typedef uint16_t TickType_t;
#define portMAX_DELAY (TickType_t)0xffff
#else
typedef uint32_t TickType_t;
#define portMAX_DELAY (TickType_t)0xffffffffUL

#define portTICK_TYPE_IS_ATOMIC 1
#endif

/* Interrupt control and state register (SCB_ICSR)：0xe000ed04
 * Bit 28 PENDSVSET: PendSV set-pending bit
 */
#define portNVIC_INT_CTRL_REG (*((volatile uint32_t *)0xe000ed04))
#define portNVIC_PENDSVSET_BIT (1UL << 28UL)

#define portSY_FULL_READ_WRITE (15)
/* Scheduler utilities. */
#define portYIELD()                                                                \
	{                                                                              \
		/* 设置 PendSV 的中断挂起位，产生上下文切换 */                             \
		portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;                            \
                                                                                   \
		/* Barriers are normally not required but do ensure the code is completely \
		within the specified behaviour for the architecture. */                    \
		__dsb(portSY_FULL_READ_WRITE);                                             \
		__isb(portSY_FULL_READ_WRITE);                                             \
	}

/* 临界区管理 */
extern void vPortEnterCritical(void);
extern void vPortExitCritical(void);

/* 使能与否临界段 */
/* ISR不能直接使用portDISABLE_INTERRUPTS()*/
#define portDISABLE_INTERRUPTS() vPortRaiseBASEPRI()
#define portENABLE_INTERRUPTS() vPortSetBASEPRI(0)

/* 进入退出临界段 */
#define portENTER_CRITICAL() vPortEnterCritical()
#define portEXIT_CRITICAL() vPortExitCritical()

/* 在 ISR 内部临时屏蔽中断，并返回当前中断状态（用于后续恢复） */
/* 仅用于 中断服务程序（ISR），不能用在普通任务中 */
#define portSET_INTERRUPT_MASK_FROM_ISR() ulPortRaiseBASEPRI()
#define portCLEAR_INTERRUPT_MASK_FROM_ISR(x) vPortSetBASEPRI(x)

#define portINLINE __inline

#ifndef portFORCE_INLINE
#define portFORCE_INLINE __forceinline
#endif

/*-----------------------------------------------------------*/

static portFORCE_INLINE void vPortSetBASEPRI(uint32_t ulBASEPRI)
{
	__asm
	{
		/* Barrier instructions are not used as this function is only used to
		lower the BASEPRI value. */
		msr basepri, ulBASEPRI
	}
}
/*-----------------------------------------------------------*/

static portFORCE_INLINE void vPortRaiseBASEPRI(void)
{
	/* 允许 FreeRTOS 调用的最高中断优先级 */
	uint32_t ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;

	__asm
	{
		/* Set BASEPRI to the max syscall priority to effect a critical
		section. */
		/* basepri可以屏蔽优先级 ≥ 设定值的中断 */
		msr basepri, ulNewBASEPRI
		dsb
		isb
	}
}
/*-----------------------------------------------------------*/

static portFORCE_INLINE void vPortClearBASEPRIFromISR(void)
{
	__asm
	{
		/* Set BASEPRI to 0 so no interrupts are masked.  This function is only
		used to lower the mask in an interrupt, so memory barriers are not
		used. */
		msr basepri, #0
	}
}
/*-----------------------------------------------------------*/

static portFORCE_INLINE uint32_t ulPortRaiseBASEPRI(void)
{
	uint32_t ulReturn, ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;

	__asm
	{
		/* Set BASEPRI to the max syscall priority to effect a critical
		section. */
		mrs ulReturn, basepri
		msr basepri, ulNewBASEPRI
		dsb
		isb
	}

	return ulReturn;
}
/*-----------------------------------------------------------*/

// static portFORCE_INLINE BaseType_t xPortIsInsideInterrupt( void )
//{
// uint32_t ulCurrentInterrupt;
// BaseType_t xReturn;

//	/* Obtain the number of the currently executing interrupt. */
//	__asm
//	{
//		mrs ulCurrentInterrupt, ipsr
//	}

//	if( ulCurrentInterrupt == 0 )
//	{
//		xReturn = pdFALSE;
//	}
//	else
//	{
//		xReturn = pdTRUE;
//	}

//	return xReturn;
//}

#endif /* PORTMACRO_H */
