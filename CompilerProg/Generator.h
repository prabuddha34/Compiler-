
#pragma once
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include "Parser.h"

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
        else {
            const auto& s = std::get<NodeStatementLet>(stmt.expr);
            const std::string& name = s.identifier.value.value();

            if (m_vars.count(name)) {
                std::cerr << "Error: identifier already used: " << name << std::endl;
                std::exit(EXIT_FAILURE);
            }

            m_vars[name] = m_stack_size;       // the value will sit in this slot
            gen_expr(s.expr);                  // pushes it, and we just leave it there
        }
    }

    // walks the tree and writes the assembly
    inline std::string generate() {
        m_output << "global _start\n_start:\n";

        for (const NodeStatements& stmt : m_root.statements) {
            gen_stmt(stmt);
        }

        // default exit(0) in case the program has no exit statement
        m_output << "    mov rax, 60\n";
        m_output << "    mov rdi, 0\n";
        m_output << "    syscall\n";

        return m_output.str();
    }

private:
    size_t m_abs_count = 0;
    size_t m_max_count = 0;
    size_t m_min_count = 0;
    size_t m_sqrt_count = 0;
    size_t m_fact_count = 0;
    size_t m_gcd_count  = 0;
    size_t m_prime_count = 0;
    size_t m_palindrome_count = 0;
    size_t m_n_fib_count = 0;
    void push(const std::string& reg) {
        m_output << "    push " << reg << "\n";
        m_stack_size++;
    }

    void pop(const std::string& reg) {
        m_output << "    pop " << reg << "\n";
        m_stack_size--;
    }

    const NodeProgram m_root;
    std::stringstream m_output;
    size_t m_stack_size = 0;  // number of 8 byte slots pushed
    std::unordered_map<std::string, size_t> m_vars;  // name -> stack slot
};
