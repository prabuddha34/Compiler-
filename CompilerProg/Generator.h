#pragma once
#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include "Parser.h"
#include <bit>
#include <cstdint>

class Generator {
public:
    inline explicit Generator(NodeProgram root)
        : m_root(std::move(root))
    {
    }

    // generates the code for an expression, the result is left on top of the stack
    void gen_expr(const NodeExpression& expr) {

        if (std::holds_alternative<NodeIntLiteral>(expr.var)) {
            const auto& lit = std::get<NodeIntLiteral>(expr.var);
            m_output << "    mov rax, " << lit.int_literal.value.value() << "\n";
            push("rax");
        }
        else if (std::holds_alternative<NodeDoubleLiteral>(expr.var)) {
            const auto& lit = std::get<NodeDoubleLiteral>(expr.var);

            double value = std::stod(lit.double_literal.value.value());
            uint64_t bits = std::bit_cast<uint64_t>(value);

            m_output << "    mov rax, 0x"
                     << std::hex << bits << std::dec << "\n";

            push("rax");
        }
        else if (std::holds_alternative<NodeStringLiteral>(expr.var)) {
            const auto& lit = std::get<NodeStringLiteral>(expr.var);
            std::string value = lit.string_literal.value.value();

            const size_t id = m_string_count++;

            // Emit the string literal into a data section
            m_output << "section .data\n";
            m_output << "str_" << id << " db `" << value << "`, 0\n";
            m_output << "section .text\n";

            // Load the address of the string and push it
            m_output << "    lea rax, [rel str_" << id << "]\n";
            push("rax");
        }
        else if (std::holds_alternative<NodeIntIdentifier>(expr.var)) {
            const auto& id = std::get<NodeIntIdentifier>(expr.var);
            const std::string& name = id.identifier.value.value();

            auto it = m_vars.find(name);
            if (it == m_vars.end()) {
                std::cerr << "Error: undeclared identifier: " << name << std::endl;
                std::exit(EXIT_FAILURE);
            }

            // the variable lives at stack slot it->second, rsp points at the top slot
            size_t offset = (m_stack_size - it->second - 1) * 8;
            push("QWORD [rsp + " + std::to_string(offset) + "]");
        }
        else if (std::holds_alternative<NodeArrayIndex>(expr.var)) {
            const auto& a = std::get<NodeArrayIndex>(expr.var);

            gen_expr(*a.index);
            pop("rcx");                       // pop BEFORE computing offsets
            gen_array_addr(a.identifier.value.value());
            m_output << "    mov rax, [rbx]\n";
            push("rax");
        }
        else if (std::holds_alternative<NodeLen>(expr.var)) {
            const auto& l = std::get<NodeLen>(expr.var);

            auto it = m_arrays.find(l.identifier.value.value());
            if (it == m_arrays.end()) {
                std::cerr << "Error: undeclared array: " << l.identifier.value.value() << std::endl;
                std::exit(EXIT_FAILURE);
            }

            m_output << "    mov rax, " << it->second << "\n";
            push("rax");
        }
        else if (std::holds_alternative<NodeDigitSum>(expr.var)) {
            const auto& digit_sum = std::get<NodeDigitSum>(expr.var);

            gen_expr(*digit_sum.expr);
            pop("rax");

            const size_t id = m_digitsum_count++;

            m_output << "    mov r8, rax\n";
            m_output << "    mov r9, 0\n";
            m_output << "    mov r10, 10\n";

            // Convert negative values to unsigned magnitude.
            // This also handles INT64_MIN.
            m_output << "    test r8, r8\n";
            m_output << "    jns digitsum_loop_" << id << "\n";
            m_output << "    neg r8\n";

            m_output << "digitsum_loop_" << id << ":\n";
            m_output << "    test r8, r8\n";
            m_output << "    jz digitsum_done_" << id << "\n";

            m_output << "    mov rax, r8\n";
            m_output << "    xor rdx, rdx\n";
            m_output << "    div r10\n";

            m_output << "    add r9, rdx\n";
            m_output << "    mov r8, rax\n";
            m_output << "    jmp digitsum_loop_" << id << "\n";

            m_output << "digitsum_done_" << id << ":\n";
            m_output << "    mov rax, r9\n";

            push("rax");
        }
        else if (std::holds_alternative<NodeCountDigits>(expr.var)) {
            const auto& count = std::get<NodeCountDigits>(expr.var);

            gen_expr(*count.expr);
            pop("rax");

            m_output << "    mov r8, rax\n";
            m_output << "    mov r9, 0\n";
            m_output << "    mov r10, 10\n";

            m_output << "    test r8, r8\n";
            m_output << "    jns count_digits_loop\n";
            m_output << "    neg r8\n";

            m_output << "count_digits_loop:\n";
            m_output << "    inc r9\n";
            m_output << "    xor rdx, rdx\n";
            m_output << "    mov rax, r8\n";
            m_output << "    div r10\n";
            m_output << "    mov r8, rax\n";
            m_output << "    test r8, r8\n";
            m_output << "    jnz count_digits_loop\n";

            m_output << "    mov rax, r9\n";
            push("rax");
        }
        else if (std::holds_alternative<NodeSum>(expr.var)) {
            const auto& sum_node = std::get<NodeSum>(expr.var);

            // generate the first value of sum !
            gen_expr(*sum_node.args[0]);

            // add the remaining values !
            for (size_t i = 1; i < sum_node.args.size(); i++) {
                gen_expr(*sum_node.args[i]);
                pop("rbx");
                pop("rax");
                m_output << "    add rax, rbx\n";
                push("rax");
            }
        }
        else if (std::holds_alternative<NodeEven>(expr.var)) {
            const auto& even_node = std::get<NodeEven>(expr.var);

            // checking if the value is even !
            gen_expr(*even_node.expr);
            pop("rax");
            m_output << "    and rax, 1\n";
            m_output << "    xor rax, 1\n";
            push("rax");
        }
        else if (std::holds_alternative<NodeAbs>(expr.var)) {
            const auto& abs_node = std::get<NodeAbs>(expr.var);

            // getting the absolute value !
            gen_expr(*abs_node.expr);
            pop("rax");

            const size_t id = m_abs_count++;

            m_output << "    cmp rax, 0\n";
            m_output << "    jge abs_done_" << id << "\n";
            m_output << "    neg rax\n";
            m_output << "abs_done_" << id << ":\n";

            push("rax");
        }
        else if (std::holds_alternative<NodeMidVal>(expr.var)) {
            const auto& mid = std::get<NodeMidVal>(expr.var);

            gen_expr(*mid.expr);
            pop("rax");

            m_output << "    mov rbx, 2\n";
            m_output << "    cqo\n";
            m_output << "    idiv rbx\n";

            push("rax");
        }
        else if (std::holds_alternative<NodeMax>(expr.var)) {
            const auto& max_node = std::get<NodeMax>(expr.var);

            // generate both values !
            gen_expr(*max_node.lhs);
            gen_expr(*max_node.rhs);

            pop("rbx");
            pop("rax");

            const size_t id = m_max_count++;

            m_output << "    cmp rax, rbx\n";
            m_output << "    jge max_done_" << id << "\n";
            m_output << "    mov rax, rbx\n";
            m_output << "max_done_" << id << ":\n";

            push("rax");
        }
        else if (std::holds_alternative<NodeMin>(expr.var)) {
            const auto& min_node = std::get<NodeMin>(expr.var);

            // generate both values !
            gen_expr(*min_node.lhs);
            gen_expr(*min_node.rhs);

            pop("rbx");
            pop("rax");

            const size_t id = m_min_count++;

            m_output << "    cmp rax, rbx\n";
            m_output << "    jle min_done_" << id << "\n";
            m_output << "    mov rax, rbx\n";
            m_output << "min_done_" << id << ":\n";

            push("rax");
        }
        else if (std::holds_alternative<NodeFact>(expr.var)) {
            const auto& fact = std::get<NodeFact>(expr.var);

            // generate the argument !
            gen_expr(*fact.expr);
            pop("rax");

            const size_t id = m_fact_count++;

            // factorial result starts at 1 !
            m_output << "    mov rcx, rax\n";
            m_output << "    mov rax, 1\n";

            // factorial of 0 or 1 is 1 !
            m_output << "    cmp rcx, 1\n";
            m_output << "    jle fact_done_" << id << "\n";

            // multiply down to 1 !
            m_output << "fact_loop_" << id << ":\n";
            m_output << "    imul rax, rcx\n";
            m_output << "    dec rcx\n";
            m_output << "    cmp rcx, 1\n";
            m_output << "    jg fact_loop_" << id << "\n";

            m_output << "fact_done_" << id << ":\n";

            // leave the result on the stack !
            push("rax");
        }
        else if (std::holds_alternative<NodeSqrt>(expr.var)) {
            const auto& sqrt_node = std::get<NodeSqrt>(expr.var);

            // getting the integer square root !
            gen_expr(*sqrt_node.expr);
            pop("rax");

            const size_t id = m_sqrt_count++;

            m_output << "    cmp rax, 0\n";
            m_output << "    jle sqrt_zero_" << id << "\n";

            m_output << "    mov r8, rax\n";       // original number
            m_output << "    mov rbx, 0\n";        // low
            m_output << "    mov rcx, rax\n";      // high
            m_output << "    inc rcx\n";

            m_output << "sqrt_loop_" << id << ":\n";
            m_output << "    lea rdx, [rbx + 1]\n";
            m_output << "    cmp rdx, rcx\n";
            m_output << "    jge sqrt_done_" << id << "\n";

            m_output << "    lea rdx, [rbx + rcx]\n";
            m_output << "    shr rdx, 1\n";        // mid = (low + high) / 2

            m_output << "    mov rsi, rdx\n";
            m_output << "    imul rsi, rdx\n";     // mid * mid
            m_output << "    cmp rsi, r8\n";
            m_output << "    jle sqrt_low_" << id << "\n";

            m_output << "    mov rcx, rdx\n";      // high = mid
            m_output << "    jmp sqrt_loop_" << id << "\n";

            m_output << "sqrt_low_" << id << ":\n";
            m_output << "    mov rbx, rdx\n";      // low = mid
            m_output << "    jmp sqrt_loop_" << id << "\n";

            m_output << "sqrt_done_" << id << ":\n";
            m_output << "    mov rax, rbx\n";
            m_output << "    jmp sqrt_end_" << id << "\n";

            m_output << "sqrt_zero_" << id << ":\n";
            m_output << "    mov rax, 0\n";

            m_output << "sqrt_end_" << id << ":\n";
            push("rax");
        }
        else if (std::holds_alternative<NodeGCD>(expr.var)) {
            const auto& gcd_node = std::get<NodeGCD>(expr.var);

            // generate both values !
            gen_expr(*gcd_node.lhs);
            gen_expr(*gcd_node.rhs);

            pop("rbx");
            pop("rax");

            const size_t id = m_gcd_count++;

            // Euclidean algorithm !
            m_output << "gcd_loop_" << id << ":\n";
            m_output << "    cmp rbx, 0\n";
            m_output << "    je gcd_done_" << id << "\n";

            m_output << "    mov rcx, rbx\n";
            m_output << "    cqo\n";
            m_output << "    idiv rbx\n";

            m_output << "    mov rax, rcx\n";
            m_output << "    mov rbx, rdx\n";

            m_output << "    jmp gcd_loop_" << id << "\n";

            m_output << "gcd_done_" << id << ":\n";

            // leave the result on the stack !
            push("rax");
        }
        else if (std::holds_alternative<NodePrime>(expr.var)) {
            const auto& prime_node = std::get<NodePrime>(expr.var);

            // generate the number !
            gen_expr(*prime_node.expr);
            pop("rax");

            const size_t id = m_prime_count++;

            // numbers smaller than 2 are not prime !
            m_output << "    cmp rax, 2\n";
            m_output << "    jl prime_false_" << id << "\n";

            // save the original number !
            m_output << "    mov r8, rax\n";
            m_output << "    mov rcx, 2\n";

            // check possible divisors !
            m_output << "prime_loop_" << id << ":\n";

            // if divisor * divisor > number, it is prime !
            m_output << "    mov rdx, rcx\n";
            m_output << "    imul rdx, rcx\n";
            m_output << "    cmp rdx, r8\n";
            m_output << "    jg prime_true_" << id << "\n";

            // divide number by divisor !
            m_output << "    mov rax, r8\n";
            m_output << "    cqo\n";
            m_output << "    idiv rcx\n";

            // if remainder is zero, it is not prime !
            m_output << "    cmp rdx, 0\n";
            m_output << "    je prime_false_" << id << "\n";

            // try the next divisor !
            m_output << "    inc rcx\n";
            m_output << "    jmp prime_loop_" << id << "\n";

            // prime returns 1 !
            m_output << "prime_true_" << id << ":\n";
            m_output << "    mov rax, 1\n";
            m_output << "    jmp prime_done_" << id << "\n";

            // not prime returns 0 !
            m_output << "prime_false_" << id << ":\n";
            m_output << "    mov rax, 0\n";

            m_output << "prime_done_" << id << ":\n";

            // leave the result on the stack !
            push("rax");
        }
        else if (std::holds_alternative<NodeNthFibonacci>(expr.var)) {
            const auto& fib = std::get<NodeNthFibonacci>(expr.var);

            gen_expr(*fib.expr);
            pop("rax");

            const size_t id = m_n_fib_count++;

            m_output << "    cmp rax, 0\n";
            m_output << "    jl fib_negative_" << id << "\n";
            m_output << "    cmp rax, 1\n";
            m_output << "    jle fib_done_" << id << "\n";

            m_output << "    mov rcx, rax\n";
            m_output << "    mov r8, 0\n";
            m_output << "    mov r9, 1\n";

            m_output << "fib_loop_" << id << ":\n";
            m_output << "    mov r10, r9\n";
            m_output << "    add r10, r8\n";
            m_output << "    mov r8, r9\n";
            m_output << "    mov r9, r10\n";
            m_output << "    dec rcx\n";
            m_output << "    cmp rcx, 1\n";
            m_output << "    jg fib_loop_" << id << "\n";

            m_output << "    mov rax, r9\n";
            m_output << "    jmp fib_done_" << id << "\n";

            m_output << "fib_negative_" << id << ":\n";
            m_output << "    mov rax, 0\n";

            m_output << "fib_done_" << id << ":\n";
            push("rax");
        }
        else if (std::holds_alternative<NodeRandom>(expr.var)) {
            const auto& random_node = std::get<NodeRandom>(expr.var);

            gen_expr(*random_node.lhs);
            gen_expr(*random_node.rhs);

            pop("rbx"); // upper bound
            pop("rax"); // lower bound

            const size_t id = m_random_count++;

            m_output << "    cmp rax, rbx\n";
            m_output << "    jg random_invalid_" << id << "\n";

            m_output << "    mov r8, rax\n";
            m_output << "    mov r9, rbx\n";
            m_output << "    sub r9, r8\n";
            m_output << "    inc r9\n";             // range size

            m_output << "    mov r10, -1\n";
            m_output << "    xor rdx, rdx\n";
            m_output << "    div r9\n";
            m_output << "    mov r11, rdx\n";       // 2^64 mod range size
            m_output << "    mov rax, -1\n";
            m_output << "    sub rax, r11\n";       // largest acceptable threshold

            m_output << "random_retry_" << id << ":\n";
            m_output << "    rdrand r11\n";
            m_output << "    jnc random_retry_" << id << "\n";
            m_output << "    cmp r11, rax\n";
            m_output << "    ja random_retry_" << id << "\n";

            m_output << "    xor rdx, rdx\n";
            m_output << "    mov rax, r11\n";
            m_output << "    div r9\n";
            m_output << "    add rdx, r8\n";
            m_output << "    mov rax, rdx\n";
            m_output << "    jmp random_done_" << id << "\n";

            m_output << "random_invalid_" << id << ":\n";
            m_output << "    mov rax, 0\n";

            m_output << "random_done_" << id << ":\n";
            push("rax");
        }
        else if (std::holds_alternative<NodePalindrome>(expr.var)) {
            const auto& palindrome = std::get<NodePalindrome>(expr.var);

            gen_expr(*palindrome.expr);
            pop("rax");

            const size_t id = m_palindrome_count++;

            m_output << "    mov r8, rax\n";
            m_output << "    mov r9, 0\n";
            m_output << "    mov r10, 10\n";
            m_output << "    mov r11, rax\n";

            m_output << "    cmp rax, 0\n";
            m_output << "    jl palindrome_false_" << id << "\n";

            m_output << "palindrome_loop_" << id << ":\n";
            m_output << "    cmp r11, 0\n";
            m_output << "    je palindrome_check_" << id << "\n";

            m_output << "    mov rax, r11\n";
            m_output << "    xor rdx, rdx\n";
            m_output << "    div r10\n";
            m_output << "    imul r9, r10\n";
            m_output << "    add r9, rdx\n";
            m_output << "    mov r11, rax\n";
            m_output << "    jmp palindrome_loop_" << id << "\n";

            m_output << "palindrome_check_" << id << ":\n";
            m_output << "    cmp r8, r9\n";
            m_output << "    jne palindrome_false_" << id << "\n";

            m_output << "    mov rax, 1\n";
            m_output << "    jmp palindrome_done_" << id << "\n";

            m_output << "palindrome_false_" << id << ":\n";
            m_output << "    mov rax, 0\n";

            m_output << "palindrome_done_" << id << ":\n";

            push("rax");
        }
        else if (std::holds_alternative<NodeBinary>(expr.var)) {
            const auto& binary = std::get<NodeBinary>(expr.var);

            // generate both sides of the arithmetic expression !
            gen_expr(*binary.lhs);
            gen_expr(*binary.rhs);

            pop("rbx");
            pop("rax");

            if (binary.op == TokenType::plus) {
                m_output << "    add rax, rbx\n";
            }
            else if (binary.op == TokenType::minus) {
                m_output << "    sub rax, rbx\n";
            }
            else if (binary.op == TokenType::mul) {
                m_output << "    imul rax, rbx\n";
            }
            else if (binary.op == TokenType::division) {
                m_output << "    cqo\n";
                m_output << "    idiv rbx\n";
            }
            else if (binary.op == TokenType::equal_equal ||
                     binary.op == TokenType::not_equal ||
                     binary.op == TokenType::less ||
                     binary.op == TokenType::greater ||
                     binary.op == TokenType::less_equal ||
                     binary.op == TokenType::greater_equal) {
                m_output << "    cmp rax, rbx\n";

                if (binary.op == TokenType::equal_equal) {
                    m_output << "    sete al\n";
                }
                else if (binary.op == TokenType::not_equal) {
                    m_output << "    setne al\n";
                }
                else if (binary.op == TokenType::less) {
                    m_output << "    setl al\n";
                }
                else if (binary.op == TokenType::greater) {
                    m_output << "    setg al\n";
                }
                else if (binary.op == TokenType::less_equal) {
                    m_output << "    setle al\n";
                }
                else if (binary.op == TokenType::greater_equal) {
                    m_output << "    setge al\n";
                }

                m_output << "    movzx rax, al\n";
            }

            push("rax");
        }
        else if (std::holds_alternative<NodeEqual>(expr.var)) {
            const auto& equal = std::get<NodeEqual>(expr.var);

            gen_expr(*equal.lhs);
            gen_expr(*equal.rhs);

            pop("rbx");
            pop("rax");

            m_output << "    cmp rax, rbx\n";
            m_output << "    sete al\n";
            m_output << "    movzx rax, al\n";

            push("rax");
        }
        else if (std::holds_alternative<NodeTimeUTC>(expr.var)) {
            m_output << "    sub rsp, 16\n";
            m_output << "    xor rdi, rdi\n";
            m_output << "    mov rsi, rsp\n";
            m_output << "    mov rax, 228\n";
            m_output << "    syscall\n";

            m_output << "    cmp rax, 0\n";
            m_output << "    jl time_utc_error\n";

            m_output << "    mov rax, [rsp]\n";
            m_output << "    add rsp, 16\n";
            m_output << "    jmp time_utc_done\n";

            m_output << "time_utc_error:\n";
            m_output << "    add rsp, 16\n";
            m_output << "    xor rax, rax\n";

            m_output << "time_utc_done:\n";

            push("rax");
        }
        else if (std::holds_alternative<NodePower>(expr.var)) {
            const auto& power = std::get<NodePower>(expr.var);

            gen_expr(*power.lhs);
            gen_expr(*power.rhs);

            pop("rbx"); // exponent
            pop("rax"); // base

            const size_t id = m_power_count++;

            m_output << "    mov rcx, rbx\n";
            m_output << "    mov rbx, rax\n";
            m_output << "    mov rax, 1\n";

            m_output << "power_loop_" << id << ":\n";
            m_output << "    cmp rcx, 0\n";
            m_output << "    je power_done_" << id << "\n";
            m_output << "    imul rax, rbx\n";
            m_output << "    dec rcx\n";
            m_output << "    jmp power_loop_" << id << "\n";

            m_output << "power_done_" << id << ":\n";

            push("rax");
        }
        else if (std::holds_alternative<NodeInput>(expr.var)) {
            const size_t id = m_input_count++;

            // Read input from stdin.
            m_output << "    xor rax, rax\n";
            m_output << "    xor rdi, rdi\n";
            m_output << "    lea rsi, [rel input_buffer]\n";
            m_output << "    mov rdx, 64\n";
            m_output << "    syscall\n";

            m_output << "    cmp rax, 0\n";
            m_output << "    jle input_empty_" << id << "\n";

            // Save the number of characters read.
            m_output << "    mov r9, rax\n";
            m_output << "    lea r8, [rel input_buffer]\n";
            m_output << "    xor r10, r10\n";  // index
            m_output << "    xor r11, r11\n";  // result
            m_output << "    mov r12, 1\n";    // sign

            // Check for a negative number.
            m_output << "    cmp byte [r8], '-'\n";
            m_output << "    jne input_loop_" << id << "\n";
            m_output << "    mov r12, -1\n";
            m_output << "    inc r10\n";

            // Convert decimal characters into an integer.
            m_output << "input_loop_" << id << ":\n";
            m_output << "    cmp r10, r9\n";
            m_output << "    jge input_done_" << id << "\n";

            m_output << "    movzx rcx, byte [r8 + r10]\n";

            // Stop at newline or carriage return.
            m_output << "    cmp cl, 10\n";
            m_output << "    je input_done_" << id << "\n";
            m_output << "    cmp cl, 13\n";
            m_output << "    je input_done_" << id << "\n";

            // Stop if the character isn't a decimal digit.
            m_output << "    cmp cl, '0'\n";
            m_output << "    jl input_done_" << id << "\n";
            m_output << "    cmp cl, '9'\n";
            m_output << "    jg input_done_" << id << "\n";

            // result = result * 10 + digit
            m_output << "    imul r11, r11, 10\n";
            m_output << "    sub rcx, '0'\n";
            m_output << "    add r11, rcx\n";

            m_output << "    inc r10\n";
            m_output << "    jmp input_loop_" << id << "\n";

            m_output << "input_done_" << id << ":\n";
            m_output << "    mov rax, r11\n";
            m_output << "    imul rax, r12\n";
            m_output << "    jmp input_finish_" << id << "\n";

            m_output << "input_empty_" << id << ":\n";
            m_output << "    xor rax, rax\n";

            m_output << "input_finish_" << id << ":\n";
            push("rax");
        }
    }

    // generates the code for one statement
    void gen_stmt(const NodeStatements& stmt) {

        if (std::holds_alternative<NodeStatementExit>(stmt.expr)) {
            const auto& s = std::get<NodeStatementExit>(stmt.expr);

            gen_expr(s.expr);

            m_output << "    mov rax, 60\n";   // syscall number for exit
            pop("rdi");                        // the exit code
            m_output << "    syscall\n";
        }
        else if (std::holds_alternative<NodeStatementPrint>(stmt.expr)) {
            const auto& s = std::get<NodeStatementPrint>(stmt.expr);

            if (std::holds_alternative<NodeStringLiteral>(s.expr.var)) {
                const auto& lit = std::get<NodeStringLiteral>(s.expr.var);
                std::string value = lit.string_literal.value.value();

                gen_expr(s.expr);
                pop("rsi");

                m_output << "    mov rax, 1\n";
                m_output << "    mov rdi, 1\n";
                m_output << "    mov rdx, " << value.size() << "\n";
                m_output << "    syscall\n";
            }
            else {
                gen_expr(s.expr);
                pop("rax");

                m_output << "    mov rcx, 0\n";
                m_output << "    mov rbx, 10\n";
                m_output << "    mov byte [rel print_buffer + 31], 10\n";
                m_output << "    mov rcx, 1\n";

                m_output << ".print_loop:\n";
                m_output << "    xor rdx, rdx\n";
                m_output << "    div rbx\n";
                m_output << "    add dl, '0'\n";
                m_output << "    dec rcx\n";
                m_output << "    mov [rel print_buffer + rcx + 30], dl\n";
                m_output << "    test rax, rax\n";
                m_output << "    jnz .print_loop\n";

                m_output << "    mov rax, 1\n";
                m_output << "    mov rdi, 1\n";
                m_output << "    lea rsi, [rel print_buffer + rcx + 30]\n";
                m_output << "    mov rdx, 32\n";
                m_output << "    sub rdx, rcx\n";
                m_output << "    syscall\n";
            }
        }
        else if (std::holds_alternative<NodeStatementLet>(stmt.expr)) {
            const auto& s = std::get<NodeStatementLet>(stmt.expr);
            const std::string& name = s.identifier.value.value();

            if (m_vars.count(name)) {
                std::cerr << "Error: identifier already used: " << name << std::endl;
                std::exit(EXIT_FAILURE);
            }

            m_vars[name] = m_stack_size;       // the value will sit in this slot
            gen_expr(s.expr);                  // pushes it, and we just leave it there
        }
        else if (std::holds_alternative<NodeStatementArrayLet>(stmt.expr)) {
            const auto& s = std::get<NodeStatementArrayLet>(stmt.expr);
            const std::string& name = s.identifier.value.value();

            if (m_vars.count(name)) {
                std::cerr << "Error: identifier already used: " << name << std::endl;
                std::exit(EXIT_FAILURE);
            }

            const size_t base = m_stack_size;   // slot of element 0
            size_t count = 0;

            if (s.has_init) {
                for (const auto& e : s.elements) {
                    gen_expr(e);                // each leaves exactly one slot
                    count++;
                }
            } else {
                for (size_t i = 0; i < s.size; i++) {
                    push("0");
                    count++;
                }
            }

            m_vars[name] = base;
            m_arrays[name] = count;
        }
        else if (std::holds_alternative<NodeStatementArrayAssign>(stmt.expr)) {
            const auto& s = std::get<NodeStatementArrayAssign>(stmt.expr);

            gen_expr(s.value);
            gen_expr(s.index);
            pop("rcx");   // index
            pop("rax");   // value
            gen_array_addr(s.identifier.value.value());
            m_output << "    mov [rbx], rax\n";
        }
        else if (std::holds_alternative<std::shared_ptr<NodeIf>>(stmt.expr)) {
            const auto& if_node = *std::get<std::shared_ptr<NodeIf>>(stmt.expr);
            const size_t id = m_if_count++;

            // generate the condition; zero means false, nonzero means true
            gen_expr(if_node.condition);
            pop("rax");

            m_output << "    cmp rax, 0\n";
            m_output << "    je if_else_" << id << "\n";

            // variables created inside a block stay inside that block
            const size_t base_stack_size = m_stack_size;
            const auto base_vars = m_vars;
            const auto base_arrays = m_arrays;

            for (const NodeStatements& inner_stmt : if_node.then_body) {
                gen_stmt(inner_stmt);
            }

            if (m_stack_size > base_stack_size) {
                m_output << "    add rsp, " << (m_stack_size - base_stack_size) * 8 << "\n";
            }
            m_stack_size = base_stack_size;
            m_vars = base_vars;
            m_arrays = base_arrays;

            m_output << "    jmp if_end_" << id << "\n";
            m_output << "if_else_" << id << ":\n";

            for (const NodeStatements& inner_stmt : if_node.else_body) {
                gen_stmt(inner_stmt);
            }

            if (m_stack_size > base_stack_size) {
                m_output << "    add rsp, " << (m_stack_size - base_stack_size) * 8 << "\n";
            }
            m_stack_size = base_stack_size;
            m_vars = base_vars;
            m_arrays = base_arrays;

            m_output << "if_end_" << id << ":\n";
        }
        else if (std::holds_alternative<std::shared_ptr<NodeFor>>(stmt.expr)) {
            const auto& for_node = *std::get<std::shared_ptr<NodeFor>>(stmt.expr);

            const size_t id = m_for_count++;
            const std::string& name = for_node.identifier.value.value();

            if (m_vars.count(name)) {
                std::cerr << "Error: identifier already used: " << name << std::endl;
                std::exit(EXIT_FAILURE);
            }

            const size_t base_stack_size = m_stack_size;
            const auto base_vars = m_vars;
            const auto base_arrays = m_arrays;

            // Create the loop variable.
            gen_expr(for_node.start);
            m_vars[name] = m_stack_size - 1;

            // Evaluate the ending value once and store it on the stack.
            gen_expr(for_node.end);
            const size_t end_slot = m_stack_size - 1;

            m_output << "for_start_" << id << ":\n";

            // Compare loop variable with the ending value.
            size_t var_offset = (m_stack_size - m_vars[name] - 1) * 8;
            size_t end_offset = (m_stack_size - end_slot - 1) * 8;

            m_output << "    mov rax, QWORD [rsp + " << var_offset << "]\n";
            m_output << "    cmp rax, QWORD [rsp + " << end_offset << "]\n";
            m_output << "    jge for_end_" << id << "\n";

            // Run the body. Clean up variables declared in the body each iteration.
            const size_t body_stack_size = m_stack_size;
            const auto body_vars = m_vars;
            const auto body_arrays = m_arrays;

            for (const NodeStatements& inner_stmt : for_node.body) {
                gen_stmt(inner_stmt);
            }

            if (m_stack_size > body_stack_size) {
                m_output << "    add rsp, " << (m_stack_size - body_stack_size) * 8 << "\n";
            }

            m_stack_size = body_stack_size;
            m_vars = body_vars;
            m_arrays = body_arrays;

            // Increment the loop variable.
            var_offset = (m_stack_size - m_vars[name] - 1) * 8;

            m_output << "    inc QWORD [rsp + " << var_offset << "]\n";
            m_output << "    jmp for_start_" << id << "\n";

            m_output << "for_end_" << id << ":\n";

            // Remove the loop variable and ending value.
            if (m_stack_size > base_stack_size) {
                m_output << "    add rsp, " << (m_stack_size - base_stack_size) * 8 << "\n";
            }

            m_stack_size = base_stack_size;
            m_vars = base_vars;
            m_arrays = base_arrays;
        }
    }

    // walks the tree and writes the assembly
    inline std::string generate() {
        m_output << "section .bss\n";
        m_output << "input_buffer: resb 64\n";
        m_output << "print_buffer: resb 32\n";

        m_output << "section .text\n";
        m_output << "global _start\n";
        m_output << "_start:\n";

        for (const NodeStatements& stmt : m_root.statements) {
            gen_stmt(stmt);
        }

        // default exit(0) in case the program has no exit statement
        m_output << "    mov rax, 60\n";
        m_output << "    mov rdi, 0\n";
        m_output << "    syscall\n";

        // out of bounds array access lands here and exits with code 1
        m_output << "array_oob:\n";
        m_output << "    mov rax, 60\n";
        m_output << "    mov rdi, 1\n";
        m_output << "    syscall\n";

        return m_output.str();
    }

private:
    size_t m_abs_count = 0;
    size_t m_max_count = 0;
    size_t m_min_count = 0;
    size_t m_sqrt_count = 0;
    size_t m_fact_count = 0;
    size_t m_gcd_count = 0;
    size_t m_prime_count = 0;
    size_t m_palindrome_count = 0;
    size_t m_n_fib_count = 0;
    size_t m_stack_size = 0;
    size_t m_random_count = 0;
    size_t m_digitsum_count = 0;
    size_t m_power_count = 0;
    size_t m_if_count = 0;
    size_t m_input_count = 0;
    size_t m_for_count = 0;
    size_t m_print_count = 0;
    size_t m_string_count = 0;

    void push(const std::string& reg) {
        m_output << "    push " << reg << "\n";
        m_stack_size++;
    }

    void pop(const std::string& reg) {
        m_output << "    pop " << reg << "\n";
        m_stack_size--;
    }

    // expects the index in rcx and leaves the address of the element in rbx
    // (also does the bounds check, out of range jumps to array_oob)
    void gen_array_addr(const std::string& name) {
        auto vit = m_vars.find(name);
        auto ait = m_arrays.find(name);
        if (vit == m_vars.end() || ait == m_arrays.end()) {
            std::cerr << "Error: undeclared array: " << name << std::endl;
            std::exit(EXIT_FAILURE);
        }

        // bounds check
        m_output << "    cmp rcx, 0\n";
        m_output << "    jl array_oob\n";
        m_output << "    cmp rcx, " << ait->second << "\n";
        m_output << "    jge array_oob\n";

        // address of element 0, then walk down 8 bytes per index
        size_t offset = (m_stack_size - vit->second - 1) * 8;
        m_output << "    lea rbx, [rsp + " << offset << "]\n";
        m_output << "    shl rcx, 3\n";
        m_output << "    sub rbx, rcx\n";
    }

    NodeProgram m_root;
    std::stringstream m_output;
    std::unordered_map<std::string, size_t> m_vars;
    std::unordered_map<std::string, size_t> m_arrays; // name -> length
};