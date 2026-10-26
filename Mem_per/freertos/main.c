/*
 * main.c - FreeRTOS bring-up on VexRiscv / iCE40UP5K
 *
 * Three tasks, shaped like the final light-radar application:
 *
 *  RT   (prio 4)  periodic, 10 ms, xTaskDelayUntil(). Measures its own wake-up
 *                 latency (tick interrupt -> task running) with mtime and
 *                 blinks LED_RT at 1 Hz. Later: step the scanner + read the ADC.
 *  REP  (prio 2)  once per second prints latency, heap and stack usage on the
 *                 UART. Later: the remote-comms task.
 *  BG   (prio 1)  blinks LED_BG at 2 Hz with vTaskDelay(). Shows that lower
 *                 priority work still runs.
 *
 * If the LEDs blink at the right rates the scheduler, tick and context
 * switch all work - no UART needed for that first check.
 */

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "up5k_riscv.h"
#include "acia.h"
#include "printf.h"

/* LEDs: d1-d4 = gp_out[3:0] (active-low on this board).
 * After the scheduler starts, d1 blinks at 1 Hz (RT task), d2 at 2 Hz (BG task);
 * d3 blinking fast = error (assert, stack overflow, malloc failure, trap). */
#define LED_RT              LED_D1
#define LED_BG              LED_D2
#define LED_ERR             LED_D3

#define RT_PERIOD_MS        10
#define CYCLES_PER_US       ( configCPU_CLOCK_HZ / 1000000UL )

/* Port internals (portable/GCC/RISC-V/port.c): time of the NEXT-but-one tick
 * and cycles per tick. Used only to timestamp when the current tick fired. */
extern uint64_t ullNextTime;
extern const size_t uxTimerIncrementsForOneTick;

static TaskHandle_t xRtHandle, xRepHandle, xBgHandle;

/* written by RT, read by REP (32-bit stores/loads are atomic on RV32) */
static volatile uint32_t ulLatLast, ulLatMax, ulRtRuns;

static void prvHalt( void ) __attribute__( ( noreturn ) );

/* gp_out is read-modify-write and shared between tasks -> protect it. */
static void prvLedToggle( uint32_t ulMask )
{
    taskENTER_CRITICAL();
    gp_out ^= ulMask;
    taskEXIT_CRITICAL();
}

/*-----------------------------------------------------------*/

static void vRtTask( void * pv )
{
    const TickType_t xPeriod = pdMS_TO_TICKS( RT_PERIOD_MS );
    TickType_t xLastWake = xTaskGetTickCount();
    uint32_t ulTickTime, ulLat;

    ( void ) pv;

    for( ;; )
    {
        xTaskDelayUntil( &xLastWake, xPeriod );

        /* After the tick handler ran, ullNextTime holds the compare value two
         * ticks ahead, so the tick that woke us fired at ullNextTime - 2*inc.
         * Only the low word is needed: deltas are far below 2^32 cycles. */
        ulTickTime = ( uint32_t ) ullNextTime - 2U * uxTimerIncrementsForOneTick;
        ulLat = mtime_lo - ulTickTime;

        ulLatLast = ulLat;
        if( ( ulRtRuns > 0 ) && ( ulLat > ulLatMax ) )    /* skip first run */
        {
            ulLatMax = ulLat;
        }

        if( ( ++ulRtRuns % ( 500 / RT_PERIOD_MS ) ) == 0 )  /* every 500 ms */
        {
            prvLedToggle( LED_RT );
        }
    }
}

static void vBgTask( void * pv )
{
    ( void ) pv;

    for( ;; )
    {
        vTaskDelay( pdMS_TO_TICKS( 250 ) );
        prvLedToggle( LED_BG );
    }
}

static void vReportTask( void * pv )
{
    TickType_t xLastWake = xTaskGetTickCount();

    ( void ) pv;

    for( ;; )
    {
        xTaskDelayUntil( &xLastWake, pdMS_TO_TICKS( 1000 ) );

        printf( "t=%us  rt latency last=%uus max=%uus  heap free=%u min=%u  "
                "stack free (words) rt=%u rep=%u bg=%u\r\n",
                ( unsigned ) ( xTaskGetTickCount() / configTICK_RATE_HZ ),
                ( unsigned ) ( ulLatLast / CYCLES_PER_US ),
                ( unsigned ) ( ulLatMax / CYCLES_PER_US ),
                ( unsigned ) xPortGetFreeHeapSize(),
                ( unsigned ) xPortGetMinimumEverFreeHeapSize(),
                ( unsigned ) uxTaskGetStackHighWaterMark( xRtHandle ),
                ( unsigned ) uxTaskGetStackHighWaterMark( xRepHandle ),
                ( unsigned ) uxTaskGetStackHighWaterMark( xBgHandle ) );
    }
}

/*-----------------------------------------------------------*/

int main( void )
{
    gp_out = 7;                                /* boot stage 7 on d1-d4: app running */
    init_printf( 0, acia_printf_putc );
    printf( "\r\nFreeRTOS %s on VexRiscv / iCE40UP5K, %u Hz clock, %u Hz tick\r\n",
            tskKERNEL_VERSION_NUMBER,
            ( unsigned ) configCPU_CLOCK_HZ, ( unsigned ) configTICK_RATE_HZ );

    configASSERT( xTaskCreate( vRtTask, "RT", configMINIMAL_STACK_SIZE, NULL,
                               configMAX_PRIORITIES - 1, &xRtHandle ) == pdPASS );
    configASSERT( xTaskCreate( vReportTask, "REP", 2 * configMINIMAL_STACK_SIZE, NULL,
                               2, &xRepHandle ) == pdPASS );
    configASSERT( xTaskCreate( vBgTask, "BG", configMINIMAL_STACK_SIZE, NULL,
                               1, &xBgHandle ) == pdPASS );

    gp_out = 8;                                /* stage 8: scheduler starting */
    vTaskStartScheduler();

    /* Only reached if there was not enough heap for the idle task. */
    vAssertCalled( __FILE__, __LINE__ );
    return 0;
}

/*-------------------- hooks and fault handlers -------------*/

/* Blink LED_ERR forever with interrupts off: visible without a UART. */
static void prvHalt( void )
{
    taskDISABLE_INTERRUPTS();
    for( ;; )
    {
        gp_out ^= LED_ERR;
        for( volatile uint32_t i = 0; i < 200000; i++ )
        {
        }
    }
}

void vAssertCalled( const char * pcFile, int iLine )
{
    taskDISABLE_INTERRUPTS();
    printf( "\r\nASSERT %s:%d\r\n", ( char * ) pcFile, iLine );
    prvHalt();
}

void vApplicationMallocFailedHook( void )
{
    taskDISABLE_INTERRUPTS();
    printf( "\r\nmalloc failed - increase configTOTAL_HEAP_SIZE\r\n" );
    prvHalt();
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char * pcTaskName )
{
    ( void ) xTask;
    taskDISABLE_INTERRUPTS();
    printf( "\r\nstack overflow in task %s\r\n", pcTaskName );
    prvHalt();
}

/* Called by the port's trap handler for any exception other than ecall.
 * mepc arrives already advanced by 4. With catchIllegalInstruction=false
 * and no access-fault catching in GenUp5k.scala this should never fire. */
void freertos_risc_v_application_exception_handler( uint32_t ulMcause, uint32_t ulMepc )
{
    printf( "\r\nEXCEPTION mcause=%x mepc=%x\r\n",
            ( unsigned ) ulMcause, ( unsigned ) ( ulMepc - 4 ) );
    prvHalt();
}

/* Called for any interrupt other than the machine timer. externalInterrupt
 * is tied to 0 in system.v for now, so this should never fire either. */
void freertos_risc_v_application_interrupt_handler( uint32_t ulMcause )
{
    printf( "\r\nunexpected interrupt mcause=%x\r\n", ( unsigned ) ulMcause );
    prvHalt();
}
