
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
struct NodeStringLiteral { //helps to get the string literal in my code- base !
    Token string_literal;
};
struct NodeDoubleLiteral {
    Token double_literal;
}; //getting the double_literal!

struct NodeInput {};
struct NodeTimeUTC {};

struct NodeIntIdentifier {
    Token identifier; //here it is the identifier!
};

struct NodeExpression;

struct NodeCountDigits {
    std::shared_ptr<NodeExpression> expr;
};

struct NodeSum {
    std::vector<std::shared_ptr<NodeExpression>> args;
};

struct NodeEqual {
    std::shared_ptr<NodeExpression> lhs;
    std::shared_ptr<NodeExpression> rhs;
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

// NodeStatementPrint moved below NodeExpression because it is a statement!

struct NodeDigitSum {
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

struct NodeMidVal {
    std::shared_ptr<NodeExpression> expr;
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

struct NodeRandom {
    std::shared_ptr<NodeExpression> lhs;
    std::shared_ptr<NodeExpression> rhs;
};

struct NodePower {
    std::shared_ptr<NodeExpression> lhs;
    std::shared_ptr<NodeExpression> rhs;
};

struct NodeExpression {
    std::variant<
        NodeIntLiteral,
        NodeIntIdentifier,
        NodeSum,
        NodeEven,
        NodeBinary,
        NodeAbs,
        NodeMax,
        NodeMin,
        NodeSqrt,
        NodeFact,
        NodeGCD,
        NodePrime,
        NodePalindrome,
        NodeNthFibonacci,
        NodeRandom,
        NodeMidVal,
        NodeDigitSum,
        NodeEqual,
        NodeTimeUTC,
        NodeCountDigits,
        NodePower,
        NodeInput,
        NodeDoubleLiteral,NodeStringLiteral
    > var; //varaint!
};

// print is a statement, so it belongs here and not in NodeExpression!
struct NodeStatementPrint {
    NodeExpression expr;
};

struct NodeStatementExit {
    NodeExpression expr;
};

struct NodeStatementLet {
    Token identifier;
    NodeExpression expr;
};

struct NodeIf;
struct NodeFor;

struct NodeStatements {
    std::variant<
        NodeStatementExit,
        NodeStatementLet,
        NodeStatementPrint,
        std::shared_ptr<NodeIf>,
        std::shared_ptr<NodeFor>
    > expr;
};

struct NodeFor {
    Token identifier;
    NodeExpression start;
    NodeExpression end;
    std::vector<NodeStatements> body;
};

struct NodeIf {
    NodeExpression condition;
    std::vector<NodeStatements> then_body;
    std::vector<NodeStatements> else_body;
};

struct NodeProgram {
    std::vector<NodeStatements> statements;
};

class Parser {
public:
    inline explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens)) {}

    inline std::optional<NodeExpression> parse_expr() {
        return parse_comparison();
    }

    //checking comparison operators for if conditions and expressions!
    std::optional<NodeExpression> parse_comparison() {
        auto lhs = parse_additive();

        if (!lhs.has_value()) {
            return std::nullopt;
        }

        while (peak().has_value()) {
            TokenType op = peak().value().type;

            if (op != TokenType::equal_equal &&
                op != TokenType::not_equal &&
                op != TokenType::less &&
                op != TokenType::greater &&
                op != TokenType::less_equal &&
                op != TokenType::greater_equal) {
                break;
            }

            consume();
            auto rhs = parse_additive();

            if (!rhs.has_value()) {
                std::cerr << "Syntax error: expected value after comparison\n";
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
        //helps to get the string one !
        // checking if the next token is a string literal
        //checks for the string one !
        if (peak().has_value() &&
            peak().value().type == TokenType::string_literal) {

            return NodeExpression{
                .var = NodeStringLiteral{
                    .string_literal = consume()
                }
            };
            }
        //for the double values !
        if (peak().has_value() &&
            peak().value().type == TokenType::double_literal) {
            return NodeExpression{
                .var = NodeDoubleLiteral{.double_literal = consume()}
            };
        }

        //checking if the next token is an integer literal !
        if (peak().has_value() &&
            peak().value().type == TokenType::int_literal) {
            return NodeExpression{
                .var = NodeIntLiteral{.int_literal = consume()}
            };
        }

        if (peak().has_value() &&
            peak().value().type == TokenType::abs) {
            consume();

            auto arg = parse_primary(); //parsing!

            if (!arg.has_value()) {
                std::cerr << "Syntax error: expected a value after 'abs' "
                          << std::endl;
                //some kind of the error !
                std::exit(EXIT_FAILURE); //exit and a failure!
            }

            return NodeExpression{ //the expression for it !
                .var = NodeAbs{
                    std::make_shared<NodeExpression>(arg.value())
                }
            };
        }

        //here we are getting the input () function

        //helps to get the gcd of the values!
        if (peak().has_value() &&
            peak().value().type == TokenType::gcd) {
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

        //now giving the mid_val() --> function!
        if (peak().has_value() &&
            peak().value().type == TokenType::mid_val) {

            consume(); // gets the mid_val one !

            auto arg = parse_primary();

            if (!arg.has_value()) {
                std::cerr << "Syntax error: expected a value after 'mid_val()'\n";
                std::exit(EXIT_FAILURE);
            }

            return NodeExpression{
                .var = NodeMidVal{
                    std::make_shared<NodeExpression>(arg.value())
                }
            };
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

        //now here  i am thinking to add the time -->utc one in here!
        if (peak().has_value() &&
            peak().value().type == TokenType::time_utc) {

            consume();

            if (!peak().has_value() ||
                peak().value().type != TokenType::open_par) {
                std::cerr << "Syntax error: expected '(' after time_utc\n";
                std::exit(EXIT_FAILURE);
            }

            consume();

            if (!peak().has_value() ||
                peak().value().type != TokenType::close_par) {
                std::cerr << "Syntax error: time_utc takes no arguments\n";
                std::exit(EXIT_FAILURE);
            }

            consume();

            return NodeExpression{
                .var = NodeTimeUTC{}
            };
        }

        //now we will convert the input() function to input().int to convert the val be like integer to do the work!
        // input().int
        if (peak().has_value() &&
            peak().value().type == TokenType::input) {

            consume(); // input

            try_consume(TokenType::open_par, "expected '(' after input");
            try_consume(TokenType::close_par, "expected ')' after input()");

            try_consume(TokenType::dot, "expected '.' after input()");

            if (!peak().has_value() ||
                peak().value().type != TokenType::identifier ||
                !peak().value().value.has_value() ||
                peak().value().value.value() != "int") {

                std::cerr << "Syntax error: expected 'int' after input().\n";
                std::exit(EXIT_FAILURE);
            }

            consume(); // int

            return NodeExpression{
                .var = NodeInput{}
            };
        }

        //gets the node for the digit sum --> digit_sum(123)-->outputs ---> 1+2+3-->6-->answer!
        if (peak().has_value() &&
            peak().value().type == TokenType::digit_sum) {

            consume(); // digit sum function!

            auto arg = parse_primary();

            if (!arg.has_value()) {
                std::cerr << "Syntax error: expected a value after 'digit_sum()'\n";
                std::exit(EXIT_FAILURE);
            }

            return NodeExpression{
                .var = NodeDigitSum{
                    std::make_shared<NodeExpression>(arg.value())
                } //cals the value !
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

        //now i am going to add the count the nodes for the count_digits()--> for ex --> count_digits(1234)-->gives the output be (4) as there are total 4 numbers!
        if (peak().has_value() &&
            peak().value().type == TokenType::count_digits) {

            consume();

            if (!peak().has_value() ||
                peak().value().type != TokenType::open_par) {
                std::cerr << "Syntax error: expected '(' after count_digits\n";
                std::exit(EXIT_FAILURE);
            }

            consume();

            auto expr = parse_expr();

            if (!expr.has_value() ||
                !peak().has_value() ||
                peak().value().type != TokenType::close_par) {
                std::cerr << "Syntax error: expected ')' after argument\n";
                std::exit(EXIT_FAILURE);
            }

            consume();

            return NodeExpression{
                .var = NodeCountDigits{
                    std::make_shared<NodeExpression>(expr.value())
                }
            };
        }

        //now we have to find the prime check of them and print 1-->if it is true and 0 -->if is is false!
        if (peak().has_value() &&
            peak().value().type == TokenType::prime) {

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
        if (peak().has_value() &&
            peak().value().type == TokenType::palindrome) {

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
        }//here now we will

        if (peak().has_value() &&
            peak().value().type == TokenType::max) { //gets the max one !

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

        //now lets add the equal() function over here!
        if (peak().has_value() &&
            peak().value().type == TokenType::equal) { //gets the equal one !

            consume();

            if (!peak().has_value() ||
                peak().value().type != TokenType::open_par) {
                std::cerr << "Syntax error: expected '(' after equal\n";
                std::exit(EXIT_FAILURE);
            }

            consume(); // (lol my erroe !

            auto lhs = parse_expr();

            if (!lhs.has_value() ||
                !peak().has_value() ||
                peak().value().type != TokenType::comma) {
                std::cerr << "Syntax error: expected two values separated by ','\n";
                std::exit(EXIT_FAILURE);
            }

            consume(); // ,consumes of it ! and a boring work!

            auto rhs = parse_expr();

            if (!rhs.has_value() ||
                !peak().has_value() ||
                peak().value().type != TokenType::close_par) { //gets the closing and opening !
                std::cerr << "Syntax error: expected ')' after max arguments\n";
                std::exit(EXIT_FAILURE);
            }

            consume(); // )

            return NodeExpression{
                .var = NodeEqual{
                    std::make_shared<NodeExpression>(lhs.value()),
                    std::make_shared<NodeExpression>(rhs.value())
                }
            };
        }

        //now we are trying to do the random function in my file!
        if (peak().has_value() &&
            peak().value().type == TokenType::random) { //gets the random !one !

            consume();

            if (!peak().has_value() ||
                peak().value().type != TokenType::open_par) {
                std::cerr << "Syntax error: expected '(' after random\n";
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
                std::cerr << "Syntax error: expected ')' after rando arguments\n";
                std::exit(EXIT_FAILURE);
            }

            consume(); // )

            return NodeExpression{
                .var = NodeRandom{
                    std::make_shared<NodeExpression>(lhs.value()),
                    std::make_shared<NodeExpression>(rhs.value())
                }
            };
        }

        //now the function for the sqrt function for it!
        if (peak().has_value() &&
            peak().value().type == TokenType::sqrt) {

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

        if (peak().has_value() &&
            peak().value().type == TokenType::min) { //gets the max one !

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

        //here we will implement the power() function!
        if (peak().has_value() &&
            peak().value().type == TokenType::power) { //gets the powerone !

            consume();

            if (!peak().has_value() ||
                peak().value().type != TokenType::open_par) {
                std::cerr << "Syntax error: expected '(' after power()\n";
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
                std::cerr << "Syntax error: expected ')' after power arguments\n";
                std::exit(EXIT_FAILURE);
            }

            consume(); // )

            return NodeExpression{
                .var = NodePower{
                    std::make_shared<NodeExpression>(lhs.value()),
                    std::make_shared<NodeExpression>(rhs.value())
                }
            };
        }

        //gets the factorial value of it !
        if (peak().has_value() &&
            peak().value().type == TokenType::fact) {

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

    //parsing the statements inside { } for if and else!
    std::vector<NodeStatements> parse_block() {
        try_consume(TokenType::open_brace, "expected '{'");

        std::vector<NodeStatements> statements;

        while (peak().has_value() &&
               peak().value().type != TokenType::close_brace) {

            auto stmt = parse_stmt();

            if (!stmt.has_value()) {
                std::cerr << "Syntax error inside block\n";
                std::exit(EXIT_FAILURE);
            }

            statements.push_back(stmt.value());
        }

        try_consume(TokenType::close_brace, "expected '}'");
        return statements;
    }

    std::optional<NodeStatements> parse_stmt() { //IMPORTTANT
        if (peak().has_value() &&
            peak().value().type == TokenType::print_keyword) { //print one very NEEDED

            consume();

            try_consume(TokenType::open_par, "expected '(' after print");

            auto expr = parse_expr();

            if (!expr.has_value()) {
                std::cerr << "expected expression inside print\n";
                std::exit(EXIT_FAILURE);
            }

            try_consume(TokenType::close_par, "expected ')' after print");
            try_consume(TokenType::semicolon, "expected ';' after print");

            return NodeStatements{
                .expr = NodeStatementPrint{expr.value()}
            };
        }

        // for (i = 0 to 5) { ... }
        if (peak().has_value() &&
            peak().value().type == TokenType::kw_for) {

            consume(); // for

            try_consume(TokenType::open_par, "expected '(' after for");

            if (!peak().has_value() ||
                peak().value().type != TokenType::identifier) {
                std::cerr << "Syntax error: expected loop variable\n";
                std::exit(EXIT_FAILURE);
            }

            Token identifier = consume();

            try_consume(TokenType::assign, "expected '=' after loop variable");

            auto start = parse_expr();

            if (!start.has_value()) {
                std::cerr << "Syntax error: expected starting value\n";
                std::exit(EXIT_FAILURE);
            }

            try_consume(TokenType::kw_to, "expected 'to' after starting value");

            auto end = parse_expr();

            if (!end.has_value()) {
                std::cerr << "Syntax error: expected ending value\n";
                std::exit(EXIT_FAILURE);
            }

            try_consume(TokenType::close_par, "expected ')' after loop range");

            auto body = parse_block();

            auto node = std::make_shared<NodeFor>(
                NodeFor{
                    identifier,
                    start.value(),
                    end.value(),
                    std::move(body)
                }
            );

            return NodeStatements{.expr = node};
        }

        // if (condition) { statements } else { statements }
        if (peak().has_value() &&
            peak().value().type == TokenType::if_keyword) {

            consume(); // if

            try_consume(TokenType::open_par, "expected '(' after if");

            auto condition = parse_expr();

            if (!condition.has_value()) {
                std::cerr << "Syntax error: expected if condition\n";
                std::exit(EXIT_FAILURE);
            }

            try_consume(TokenType::close_par, "expected ')' after condition");

            auto then_body = parse_block();
            std::vector<NodeStatements> else_body;

            if (peak().has_value() &&
                peak().value().type == TokenType::else_keyword) {

                consume(); // else
                else_body = parse_block();
            }

            auto node = std::make_shared<NodeIf>(
                NodeIf{
                    condition.value(),
                    std::move(then_body),
                    std::move(else_body)
                }
            );

            return NodeStatements{.expr = node};
        }

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
            peak(2).value().type == TokenType::assign) {

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
