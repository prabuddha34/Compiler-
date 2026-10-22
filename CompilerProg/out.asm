section .bss
input_buffer: resb 64
print_buffer: resb 32
section .text
global _start
_start:
section .data
str_0 db `I love Pussy suck me`, 0
section .text
    lea rax, [rel str_0]
    push rax
    pop rsi
    mov rax, 1
    mov rdi, 1
    mov rdx, 20
    syscall
    mov rax, 60
    mov rdi, 0
    syscall
