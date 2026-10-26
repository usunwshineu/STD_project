/*
 * FreeRTOSConfig.h - VexRiscv (RV32I, no M/C) on iCE40UP5K, emeb up5k_riscv SoC
 *
 * Kernel:  FreeRTOS-Kernel V11.1 or newer
 * Port:    portable/GCC/RISC-V
 * Chip:    portable/GCC/RISC-V/chip_specific_extensions/RV32I_CLINT_no_extensions
 * Heap:    portable/MemMang/heap_4.c
 * Build:   see Makefile in this folder
 */
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* ---- Clock and tick ---------------------------------------------------- */
#define configCPU_CLOCK_HZ                      ( 12000000UL )  /* clk_12; mtimer counts every clk */
#define configTICK_RATE_HZ                      ( 1000 )        /* drop to 250 if tick overhead is too high */

/* ---- Machine timer: mtimer.v at 0x70000000 (see system.v decoder) ------ */
#define configMTIME_BASE_ADDRESS                ( 0x70000000UL )
#define configMTIMECMP_BASE_ADDRESS             ( 0x70000008UL )

/* ---- Scheduler --------------------------------------------------------- */
#define configUSE_PREEMPTION                    1
#define configUSE_TIME_SLICING                  1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                ( 128 )         /* words = 512 bytes */
#define configMAX_TASK_NAME_LEN                 8
#define configTICK_TYPE_WIDTH_IN_BITS           TICK_TYPE_WIDTH_32_BITS
#define configIDLE_SHOULD_YIELD                 1

/* ---- Memory (64 KB SPRAM holds code + data + heap) --------------------- */
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configSUPPORT_STATIC_ALLOCATION         0
#define configTOTAL_HEAP_SIZE                   ( 16 * 1024 )
#define configISR_STACK_SIZE_WORDS              256             /* dedicated interrupt stack */

/* ---- Features (kept minimal for bring-up) ------------------------------ */
#define configUSE_MUTEXES                       1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_RECURSIVE_MUTEXES             0
#define configUSE_QUEUE_SETS                    0
#define configQUEUE_REGISTRY_SIZE               0
#define configUSE_TASK_NOTIFICATIONS            1
#define configUSE_TIMERS                        0
#define configUSE_CO_ROUTINES                   0

/* ---- Hooks and debugging ----------------------------------------------- */
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_MALLOC_FAILED_HOOK            1   /* vApplicationMallocFailedHook() in main.c */
#define configCHECK_FOR_STACK_OVERFLOW          2   /* vApplicationStackOverflowHook() in main.c */
#define configUSE_TRACE_FACILITY                0
#define configGENERATE_RUN_TIME_STATS           0

void vAssertCalled( const char * pcFile, int iLine );
#define configASSERT( x )    do { if( ( x ) == 0 ) vAssertCalled( __FILE__, __LINE__ ); } while( 0 )

/* ---- API inclusion ----------------------------------------------------- */
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskDelayUntil                 1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTaskGetSchedulerState          1

#endif /* FREERTOS_CONFIG_H */
