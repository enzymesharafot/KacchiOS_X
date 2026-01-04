/* ctxsw.S - Context switch for x86 (XINU Style) */
.text
.globl ctxsw

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
