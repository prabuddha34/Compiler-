
#pragma once
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include "Tokenization.h"

struct NodeIntLiteral {
    Token int_literal; //here it is the int ones !
};

struct NodeIntIdentifier {
    Token identifier; //here it is the identifier!
};

struct NodeExpression;

struct NodeSum {
    std::vector<std::shared_ptr<NodeExpression>> args;
};

struct NodeEven {
    std::shared_ptr<NodeExpression> expr;
};
struct NodeAbs {
    std::shared_ptr<NodeExpression> expr;
};
struct NodeSqrt {
    std::shared_ptr<NodeExpression> expr;
};
struct NodeFact {
    std::shared_ptr<NodeExpression> expr;
};
struct NodeNthFibonacci {
    std::shared_ptr<NodeExpression> expr;
};
struct NodeMax {
    std::shared_ptr<NodeExpression> lhs;
    std::shared_ptr<NodeExpression> rhs;
};
struct NodeGCD {
    std::shared_ptr<NodeExpression> lhs;
    std::shared_ptr<NodeExpression> rhs;
};
struct NodePrime {
    std::shared_ptr<NodeExpression> expr;
};
struct NodeMin {
    std::shared_ptr<NodeExpression> lhs;
    std::shared_ptr<NodeExpression> rhs;
};
struct NodePalindrome {
    std::shared_ptr<NodeExpression> expr;
};
struct NodeBinary {
    TokenType op;
    std::shared_ptr<NodeExpression> lhs;
    std::shared_ptr<NodeExpression> rhs;
};

struct NodeExpression {
    std::variant<NodeIntLiteral, NodeIntIdentifier, NodeSum, NodeEven, NodeBinary, NodeAbs,NodeMax,NodeMin,NodeSqrt,NodeFact,NodeGCD,NodePrime,NodePalindrome,NodeNthFibonacci> var; //varaint!
};

struct NodeStatementExit {
    NodeExpression expr;
};

struct NodeStatementLet {
    Token identifier;
    NodeExpression expr;
};

struct NodeStatements {
    std::variant<NodeStatementExit, NodeStatementLet> expr;
};

struct NodeProgram {
    std::vector<NodeStatements> statements;
};

class Parser {
public:
    inline explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens)) {}

    inline std::optional<NodeExpression> parse_expr() {
        return parse_additive();
    }

    //checking the addition and subtraction !
    std::optional<NodeExpression> parse_additive() {
        auto lhs = parse_multiplicative();

        if (!lhs.has_value()) {
            return std::nullopt;
        }

        while (peak().has_value() &&
               (peak().value().type == TokenType::plus ||
                peak().value().type == TokenType::minus)) {

            TokenType op = consume().type;
            auto rhs = parse_multiplicative();

            if (!rhs.has_value()) {
                std::cerr << "Syntax error: expected expression after operator\n";
                std::exit(EXIT_FAILURE);
            }

            lhs = NodeExpression{
                .var = NodeBinary{
                    op,
                    std::make_shared<NodeExpression>(lhs.value()),
                    std::make_shared<NodeExpression>(rhs.value())
                }
            };
        }

        return lhs;
    }

    //checking the multiplication and division !
    std::optional<NodeExpression> parse_multiplicative() {
        auto lhs = parse_primary();

        if (!lhs.has_value()) {
            return std::nullopt;
        }

        while (peak().has_value() &&
               (peak().value().type == TokenType::mul ||
                peak().value().type == TokenType::division)) {

            TokenType op = consume().type;
            auto rhs = parse_primary();

            if (!rhs.has_value()) {
                std::cerr << "Syntax error: expected expression after operator\n";
                std::exit(EXIT_FAILURE);
            }

            lhs = NodeExpression{
                .var = NodeBinary{
                    op,
                    std::make_shared<NodeExpression>(lhs.value()),
                    std::make_shared<NodeExpression>(rhs.value())
                }
            };
        }

        return lhs;
    }

    //checking if the next token is an integer literal, identifier or function !
    std::optional<NodeExpression> parse_primary() {
        //checking if the next token is an integer literal !
        if (peak().has_value() &&
            peak().value().type == TokenType::int_literal) {
            return NodeExpression{
                .var = NodeIntLiteral{.int_literal = consume()}
            };
        }
        if (peak().has_value() &&peak().value().type == TokenType::abs) {
            consume();
            auto arg = parse_primary(); //parsing!
            if (!arg.has_value()) {
                std::cerr << "Syntax error: expected a value after 'abs' "<<std::endl;
                //some kind of the error !
                std::exit(EXIT_FAILURE); //exit and a failure!
            }
            return NodeExpression{ //the expression for it !
                .var = NodeAbs{
                    std::make_shared<NodeExpression>(arg.value())
                }
            };

        }
        //helps to get the gcd of the values!
        if (peak().has_value() && peak().value().type ==TokenType::gcd) {
            //helps in the gcd of them!
            consume();
            if (!peak().has_value() ||
    peak().value().type != TokenType::open_par) {
                std::cerr << "Syntax error: expected '(' after gcd\n";
                std::exit(EXIT_FAILURE);
            }
            //gets the first paren of them !
            consume();

            auto lhs = parse_expr();

            if (!lhs.has_value() ||
                !peak().has_value() ||
                peak().value().type != TokenType::comma) {
                std::cerr << "Syntax error: expected ',' in gcd\n";
                std::exit(EXIT_FAILURE);
                }

            consume(); // ,

            auto rhs = parse_expr();

            if (!rhs.has_value() ||
                !peak().has_value() ||
                peak().value().type != TokenType::close_par) {
                std::cerr << "Syntax error: expected ')' after gcd arguments\n";
                std::exit(EXIT_FAILURE);
                }

            consume(); // )

            return NodeExpression{
                .var = NodeGCD{
                    std::make_shared<NodeExpression>(lhs.value()),
                    std::make_shared<NodeExpression>(rhs.value())
                }
            };


        }

        //checking if the next token is an identifier !
        if (peak().has_value() &&
            peak().value().type == TokenType::identifier) {
            return NodeExpression{
                .var = NodeIntIdentifier{.identifier = consume()}
            };
        }

        //checking if the next token is sum !
        if (peak().has_value() &&
            peak().value().type == TokenType::sum) {

            consume(); // sum

            NodeSum sum_node;

            auto first_arg = parse_additive();

            if (!first_arg.has_value()) {
                std::cerr << "Syntax error: expected first value after 'sum'\n";
                std::exit(EXIT_FAILURE);
            }

            sum_node.args.push_back(
                std::make_shared<NodeExpression>(first_arg.value())
            );

            while (peak().has_value() &&
                   peak().value().type == TokenType::comma) {

                consume(); // ,

                auto arg = parse_additive();

                if (!arg.has_value()) {
                    std::cerr << "Syntax error: expected value after ',' in sum\n";
                    std::exit(EXIT_FAILURE);
                }

                sum_node.args.push_back(
                    std::make_shared<NodeExpression>(arg.value())
                );
            }

            if (sum_node.args.size() < 2) {
                std::cerr << "Syntax error: sum requires at least two values\n";
                std::exit(EXIT_FAILURE);
            }

            return NodeExpression{.var = std::move(sum_node)};
        }

        //checking if the next token is even !
        if (peak().has_value() &&
            peak().value().type == TokenType::even) {

            consume(); // even

            auto arg = parse_primary();

            if (!arg.has_value()) {
                std::cerr << "Syntax error: expected a value after 'even'\n";
                std::exit(EXIT_FAILURE);
            }

            return NodeExpression{
                .var = NodeEven{
                    std::make_shared<NodeExpression>(arg.value())
                }
            };
        }
        //here get the n-th term for the fibonacci vals
        if (peak().has_value() &&
        peak().value().type == TokenType::n_fibonacci) {

            consume(); // fibonacci !

            auto arg = parse_primary();

            if (!arg.has_value()) {
                std::cerr << "Syntax error: expected a value after 'n_fibonacci'\n";
                std::exit(EXIT_FAILURE);
            }

            return NodeExpression{
                .var = NodeNthFibonacci{
                    std::make_shared<NodeExpression>(arg.value())
                }
            };
        }
        //now we have to find the prime check of them and print 1-->if it is true and 0 -->if is is false!
        if (peak().has_value() && peak().value().type == TokenType::prime) {
            consume();

            if (!peak().has_value() ||
                peak().value().type != TokenType::open_par) {
                std::cerr << "Syntax error: expected '(' after prime\n";
                std::exit(EXIT_FAILURE);
                }

            consume(); // (

            auto expr = parse_expr();

            if (!expr.has_value() ||
                !peak().has_value() ||
                peak().value().type != TokenType::close_par) {
                std::cerr << "Syntax error: expected ')' after argument\n";
                std::exit(EXIT_FAILURE);
                }

            consume(); // )

            return NodeExpression{
                .var = NodePrime{
                    std::make_shared<NodeExpression>(expr.value())
                }
            };
        }

        //CHECK for the palindrome number! over here we all know that palindrome number is something thay can be read same from both the backwards and forwards!
        //give -->1 if it is true else 0--> if false!
        if (peak().has_value() && peak().value().type == TokenType::palindrome) {
            consume();

            if (!peak().has_value() ||
                peak().value().type != TokenType::open_par) {
                std::cerr << "Syntax error: expected '(' after palindrome\n";
                std::exit(EXIT_FAILURE);
                }

            consume(); // (

            auto expr = parse_expr();

            if (!expr.has_value() ||
                !peak().has_value() ||
                peak().value().type != TokenType::close_par) {
                std::cerr << "Syntax error: expected ')' after argument\n";
                std::exit(EXIT_FAILURE);
                }

            consume(); // )

            return NodeExpression{
                .var = NodePalindrome{
                    std::make_shared<NodeExpression>(expr.value())
                }
            };
        }
        if (peak().has_value() && peak().value().type == TokenType::max) { //gets the max one !
            consume();
            if (!peak().has_value() ||
        peak().value().type != TokenType::open_par) {
                std::cerr << "Syntax error: expected '(' after max\n";
                std::exit(EXIT_FAILURE);
        }

            consume(); // (

            auto lhs = parse_expr();

            if (!lhs.has_value() ||
                !peak().has_value() ||
                peak().value().type != TokenType::comma) {
                std::cerr << "Syntax error: expected two values separated by ','\n";
                std::exit(EXIT_FAILURE);
                }

            consume(); // ,consumes of it !

            auto rhs = parse_expr();

            if (!rhs.has_value() ||
                !peak().has_value() ||
                peak().value().type != TokenType::close_par) { //gets the closing and opening !
                std::cerr << "Syntax error: expected ')' after max arguments\n";
                std::exit(EXIT_FAILURE);
                }

            consume(); // )

            return NodeExpression{
                .var = NodeMax{
                    std::make_shared<NodeExpression>(lhs.value()),
                    std::make_shared<NodeExpression>(rhs.value())
                }
            };
        }
        //now the function for the sqrt function for it!
        if (peak().has_value() && peak().value().type == TokenType::sqrt) {

            //now the function for the sqrt function for it!
            if (peak().has_value() && peak().value().type == TokenType::sqrt) {
                consume(); // sqrt

                if (!peak().has_value() ||
                    peak().value().type != TokenType::open_par) {
                    std::cerr << "Syntax error: expected '(' after sqrt\n";
                    std::exit(EXIT_FAILURE);
                    }

                consume(); // (

                auto arg = parse_expr();

                if (!arg.has_value() ||
                    !peak().has_value() ||
                    peak().value().type != TokenType::close_par) {
                    std::cerr << "Syntax error: expected ')' after sqrt argument\n";
                    std::exit(EXIT_FAILURE);
                    }

                consume(); // )

                return NodeExpression{
                    .var = NodeSqrt{
                        std::make_shared<NodeExpression>(arg.value())
                    }
                };
            }

        }
        if (peak().has_value() && peak().value().type == TokenType::min) { //gets the max one !
            consume();
            if (!peak().has_value() ||
        peak().value().type != TokenType::open_par) {
                std::cerr << "Syntax error: expected '(' after min\n";
                std::exit(EXIT_FAILURE);
        }

            consume(); // (

            auto lhs = parse_expr();

            if (!lhs.has_value() ||
                !peak().has_value() ||
                peak().value().type != TokenType::comma) {
                std::cerr << "Syntax error: expected two values separated by ','\n";
                std::exit(EXIT_FAILURE);
                }

            consume(); // ,consumes of it !

            auto rhs = parse_expr();

            if (!rhs.has_value() ||
                !peak().has_value() ||
                peak().value().type != TokenType::close_par) { //gets the closing and opening !
                std::cerr << "Syntax error: expected ')' after min arguments\n";
                std::exit(EXIT_FAILURE);
                }

            consume(); // )

            return NodeExpression{
                .var = NodeMin{
                    std::make_shared<NodeExpression>(lhs.value()),
                    std::make_shared<NodeExpression>(rhs.value())
                }
            };
        }
        //gets the factorial value of it !
        if (peak().has_value() && peak().value().type == TokenType::fact) {
            consume(); // fact

            if (!peak().has_value() ||
                peak().value().type != TokenType::open_par) {
                std::cerr << "Syntax error: expected '(' after fact\n";
                std::exit(EXIT_FAILURE);
                }

            consume(); // (

            auto arg = parse_expr();

            if (!arg.has_value() ||
                !peak().has_value() ||
                peak().value().type != TokenType::close_par) {
                std::cerr << "Syntax error: expected ')' after fact argument\n";
                std::exit(EXIT_FAILURE);
                }

            consume(); // )

            return NodeExpression{
                .var = NodeFact{
                    std::make_shared<NodeExpression>(arg.value())
                }
            };
        }


        //checking if the expression is inside parentheses !
        if (peak().has_value() &&
            peak().value().type == TokenType::open_par) {

            consume(); // (

            auto expr = parse_expr();

            if (!expr.has_value()) {
                std::cerr << "Syntax error: expected expression after '('\n";
                std::exit(EXIT_FAILURE);
            }

            try_consume(TokenType::close_par, "expected ')'");
            return expr;
        }

        return std::nullopt;
    }

    std::optional<NodeStatements> parse_stmt() {
        // exit ( <expr> ) ;
        if (peak().has_value() &&
            peak().value().type == TokenType::exit) {

            consume();

            try_consume(TokenType::open_par, "expected '(' after 'exit'");

            auto node_expr = parse_expr();

            if (!node_expr.has_value()) {
                std::cerr << "Syntax error: expected an expression inside 'exit(...)'"
                          << std::endl;
                std::exit(EXIT_FAILURE);
            }

            try_consume(TokenType::close_par, "expected ')'");
            try_consume(TokenType::semicolon, "expected ';'");

            return NodeStatements{
                .expr = NodeStatementExit{.expr = node_expr.value()}
            };
        }

        // let <var_name> = <expr> ;
        if (peak().has_value() &&
            peak().value().type == TokenType::let &&
            peak(1).has_value() &&
            peak(1).value().type == TokenType::identifier &&
            peak(2).has_value() &&
            peak(2).value().type == TokenType::equal) {

            consume(); // let
            Token ident = consume(); //identifier
            consume(); // =

            auto node_expr = parse_expr();

            if (!node_expr.has_value()) {
                std::cerr << "Syntax error: expected an expression after '='"
                          << std::endl;
                std::exit(EXIT_FAILURE);
            }

            try_consume(TokenType::semicolon, "expected ';'");

            return NodeStatements{
                .expr = NodeStatementLet{
                    .identifier = ident,
                    .expr = node_expr.value()
                }
            };
        }

        return std::nullopt;
    }

    inline std::optional<NodeProgram> parse() {
        NodeProgram program;

        while (peak().has_value()) {
            if (auto stmt = parse_stmt()) {
                program.statements.push_back(stmt.value());
            } else {
                std::cerr << "Syntax error: unexpected token" << std::endl;
                std::exit(EXIT_FAILURE);
            }
        }

        return program;
    }

private:
    std::optional<Token> peak(size_t ahead = 0) const {
        if (m_index_vals + ahead >= m_tokens.size()) {
            return std::nullopt;
        } else {
            return m_tokens.at(m_index_vals + ahead);
        }
    }

    Token consume() {
        return m_tokens.at(m_index_vals++);
    }

    Token try_consume(TokenType type, const std::string& err_msg) {
        if (peak().has_value() && peak().value().type == type) {
            return consume();
        }

        std::cerr << "Syntax error: " << err_msg << std::endl;
        std::exit(EXIT_FAILURE);
    }

    const std::vector<Token> m_tokens;
    size_t m_index_vals = 0;
};
