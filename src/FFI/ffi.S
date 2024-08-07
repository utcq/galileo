.global ll_ffi_call
// Lower level FFI call

.section .data
  // Allocated stack size for ffi call
  st_size:
    .quad 0

.section .text
/*
Array in rdi:
 [0-40]: registers
 [48]: stack num
 [56]: stack data
Fn Address in rsi
*/
ll_ffi_call:
  movq %rdi, %r15
  movq %rsi, %r14

  movq (%r15), %rdi
  movq 8(%r15), %rsi
  movq 16(%r15), %rdx
  movq 24(%r15), %rcx
  movq 32(%r15), %r8
  movq 40(%r15), %r9

  movq 48(%r15), %rax
  movq 56(%r15), %r13

  1:
    test %rax, %rax
    jz 2f
    dec %rax
    lea (%r13, %rax, 8), %r11
    pushq (%r11)
    addq $8, st_size(%rip)
    jmp 1b
  2:
    mov $0, %rax
    call *%r14
    add st_size(%rip), %rsp
    ret
