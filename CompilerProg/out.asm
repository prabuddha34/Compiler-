section .bss
input_buffer: resb 64
section .text
global _start
_start:
    xor rax, rax
    xor rdi, rdi
    lea rsi, [rel input_buffer]
    mov rdx, 64
    syscall
    cmp rax, 0
    jle input_empty_0
    mov r9, rax
    lea r8, [rel input_buffer]
    xor r10, r10
    xor r11, r11
    mov r12, 1
    cmp byte [r8], '-'
    jne input_loop_0
    mov r12, -1
    inc r10
input_loop_0:
    cmp r10, r9
    jge input_done_0
    movzx rcx, byte [r8 + r10]
    cmp cl, 10
    je input_done_0
    cmp cl, 13
    je input_done_0
    cmp cl, '0'
    jl input_done_0
    cmp cl, '9'
    jg input_done_0
    imul r11, r11, 10
    sub rcx, '0'
    add r11, rcx
    inc r10
    jmp input_loop_0
input_done_0:
    mov rax, r11
    imul rax, r12
    jmp input_finish_0
input_empty_0:
    xor rax, rax
input_finish_0:
    push rax
    xor rax, rax
    xor rdi, rdi
    lea rsi, [rel input_buffer]
    mov rdx, 64
    syscall
    cmp rax, 0
    jle input_empty_1
    mov r9, rax
    lea r8, [rel input_buffer]
    xor r10, r10
    xor r11, r11
    mov r12, 1
    cmp byte [r8], '-'
    jne input_loop_1
    mov r12, -1
    inc r10
input_loop_1:
    cmp r10, r9
    jge input_done_1
    movzx rcx, byte [r8 + r10]
    cmp cl, 10
    je input_done_1
    cmp cl, 13
    je input_done_1
    cmp cl, '0'
    jl input_done_1
    cmp cl, '9'
    jg input_done_1
    imul r11, r11, 10
    sub rcx, '0'
    add r11, rcx
    inc r10
    jmp input_loop_1
input_done_1:
    mov rax, r11
    imul rax, r12
    jmp input_finish_1
input_empty_1:
    xor rax, rax
input_finish_1:
    push rax
    push QWORD [rsp + 8]
    push QWORD [rsp + 8]
    pop rbx
    pop rax
    add rax, rbx
    push rax
    push QWORD [rsp + 0]
    mov rax, 60
    pop rdi
    syscall
    mov rax, 60
    mov rdi, 0
    syscall
