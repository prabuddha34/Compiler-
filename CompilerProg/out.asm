section .bss
input_buffer: resb 64
print_buffer: resb 32
section .text
global _start
_start:
    mov rax, 1
    push rax
    mov rax, 2
    push rax
    mov rax, 4
    push rax
    mov rax, 2
    push rax
    pop rcx
    cmp rcx, 0
    jl array_oob
    cmp rcx, 3
    jge array_oob
    lea rbx, [rsp + 16]
    shl rcx, 3
    sub rbx, rcx
    mov rax, [rbx]
    push rax
    pop rax
    mov rcx, 0
    mov rbx, 10
    mov byte [rel print_buffer + 31], 10
    mov rcx, 1
.print_loop:
    xor rdx, rdx
    div rbx
    add dl, '0'
    dec rcx
    mov [rel print_buffer + rcx + 30], dl
    test rax, rax
    jnz .print_loop
    mov rax, 1
    mov rdi, 1
    lea rsi, [rel print_buffer + rcx + 30]
    mov rdx, 32
    sub rdx, rcx
    syscall
    mov rax, 60
    mov rdi, 0
    syscall
array_oob:
    mov rax, 60
    mov rdi, 1
    syscall
