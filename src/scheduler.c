#include "scheduler.h"
#include "process.h"
#include "serial.h"

void scheduler_initialize(void){
    serial_puts("Scheduler initialized.\n");
}

void scheduler_start(void){
    serial_puts("Scheduler started.\n");
    process_scheduler_start();
}

void scheduler_yield(void){
    process_yield_cpu();
}
