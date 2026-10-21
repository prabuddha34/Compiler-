global _start
_start:
    mov rax, 145
    push rax
    mov rax, 2
    push rax
    push QWORD [rsp + 8]
    push QWORD [rsp + 8]
    pop rbx
    pop rax
    cmp rax, rbx
    jle min_done_0
    mov rax, rbx
min_done_0:
    push rax
    push QWORD [rsp + 0]
    pop rax
    cmp rax, 0
    jle sqrt_zero_0
    mov r8, rax
    mov rbx, 0
    mov rcx, rax
    inc rcx
sqrt_loop_0:
    lea rdx, [rbx + 1]
    cmp rdx, rcx
    jge sqrt_done_0
    lea rdx, [rbx + rcx]
    shr rdx, 1
    mov rsi, rdx
    imul rsi, rdx
    cmp rsi, r8
    jle sqrt_low_0
    mov rcx, rdx
    jmp sqrt_loop_0
sqrt_low_0:
    mov rbx, rdx
    jmp sqrt_loop_0
sqrt_done_0:
    mov rax, rbx
    jmp sqrt_end_0
sqrt_zero_0:
    mov rax, 0
sqrt_end_0:
    push rax
    mov rax, 3
    push rax
    pop rax
    mov rcx, rax
    mov rax, 1
    cmp rcx, 1
    jle fact_done_0
fact_loop_0:
    imul rax, rcx
    dec rcx
    cmp rcx, 1
    jg fact_loop_0
fact_done_0:
    push rax
    mov rax, 20
    push rax
    mov rax, 18
    push rax
    pop rbx
    pop rax
gcd_loop_0:
    cmp rbx, 0
    je gcd_done_0
    mov rcx, rbx
    cqo
    idiv rbx
    mov rax, rcx
    mov rbx, rdx
    jmp gcd_loop_0
gcd_done_0:
    push rax
    mov rax, 3
    push rax
    pop rax
    cmp rax, 2
    jl prime_false_0
    mov r8, rax
    mov rcx, 2
prime_loop_0:
    mov rdx, rcx
    imul rdx, rcx
    cmp rdx, r8
    jg prime_true_0
    mov rax, r8
    cqo
    idiv rcx
    cmp rdx, 0
    je prime_false_0
    inc rcx
    jmp prime_loop_0
prime_true_0:
    mov rax, 1
    jmp prime_done_0
prime_false_0:
    mov rax, 0
prime_done_0:
    push rax
    mov rax, 141
    push rax
    pop rax
    mov r8, rax
    mov r9, 0
    mov r10, 10
    mov r11, rax
    cmp rax, 0
    jl palindrome_false_0
palindrome_loop_0:
    cmp r11, 0
    je palindrome_check_0
    mov rax, r11
    xor rdx, rdx
    div r10
    imul r9, r10
    add r9, rdx
    mov r11, rax
    jmp palindrome_loop_0
palindrome_check_0:
    cmp r8, r9
    jne palindrome_false_0
    mov rax, 1
    jmp palindrome_done_0
palindrome_false_0:
    mov rax, 0
palindrome_done_0:
    push rax
    mov rax, 10
    push rax
    pop rax
    cmp rax, 0
    jl fib_negative_0
    cmp rax, 1
    jle fib_done_0
    mov rcx, rax
    mov r8, 0
    mov r9, 1
fib_loop_0:
    mov r10, r9
    add r10, r8
    mov r8, r9
    mov r9, r10
    dec rcx
    cmp rcx, 1
    jg fib_loop_0
    mov rax, r9
    jmp fib_done_0
fib_negative_0:
    mov rax, 0
fib_done_0:
    push rax
    push QWORD [rsp + 0]
    mov rax, 60
    pop rdi
    syscall
    mov rax, 60
    mov rdi, 0
    syscall
