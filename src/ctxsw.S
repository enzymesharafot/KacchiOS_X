/* ctxsw.S - Context switch for x86 */
.text
.globl ctxsw
.globl start_first_process

/*
 * ctxsw - context switch between processes
 * void ctxsw(uint32_t **old_esp, uint32_t **new_esp);
 * 
 * Arguments:
 *   old_esp: pointer to old process's ESP storage
 *   new_esp: pointer to new process's ESP storage
 */
ctxsw:
    /* Save old process context */
    pushl   %ebp
    pushl   %ebx
    pushl   %esi
    pushl   %edi
    pushfl

    /* Get old_esp pointer (first argument) */
    movl    24(%esp), %eax      /* old_esp is at offset 24 after pushes */
    movl    %esp, (%eax)        /* Save current ESP */

    /* Get new_esp pointer (second argument) */
    movl    28(%esp), %eax      /* new_esp is at offset 28 */
    movl    (%eax), %esp        /* Load new ESP */

    /* Restore new process context */
    popfl
    popl    %edi
    popl    %esi
    popl    %ebx
    popl    %ebp

    ret

/*
 * start_first_process - Initialize first process execution
 * void start_first_process(uint32_t *esp);
 * 
 * This simulates returning from a context switch for the very first process.
 */
start_first_process:
    cli                         /* Disable interrupts */
    movl    4(%esp), %ecx       /* Get ESP parameter into ECX */
    movl    %ecx, %esp          /* Load new stack pointer */
    
    /* Restore registers as if returning from ctxsw */
    popfl                       /* Restore EFLAGS */
    popl    %edi               /* Restore EDI */
    popl    %esi               /* Restore ESI */
    popl    %ebx               /* Restore EBX */
    popl    %ebp               /* Restore EBP */
    
    /* Now ESP points to the function address */
    ret                        /* Pop and jump to process entry point */
