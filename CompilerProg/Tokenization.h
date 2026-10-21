
#pragma once
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

enum class TokenType { //for now i have 3 things return (int) and a semicolon !
    _return, //keyword
    int_literal, //int one
    semicolon, exit, plus, minus, mul, division, //so i am just want to do the airthmetic ops in here!
    open_par, close_par, identifier, let, equal, sum, even, comma ,abs,max,min,sqrt,fact,gcd,prime,palindrome,
    n_fibonacci
    //i forgot this !sorry ! 😢//i am a noob lol !
};

struct Token {
    //getting the tokens of it !
    TokenType type; //getting which type of token we have to use have to here ! to do the job !
    std::optional<std::string> value = std::nullopt; // so my thought is that i am just assuming every thing to be in string !and then do the operations!
};

class Tokenization {
public:
    inline Tokenization(const std::string &source)
        : m_source_val(source)
    {
    }

    //make a tokenize vectors that can take it !
    inline std::vector<Token> tokenize() {
        std::vector<Token> tokens; //tokens for our work!
        std::string buff;

        while (peak().has_value()) {
            if (std::isalpha(static_cast<unsigned char>(peak().value())) || peak().value() == '_') {
                buff.push_back(consume()); //consume the value

                while (peak().has_value() &&
                       (std::isalnum(static_cast<unsigned char>(peak().value())) || peak().value() == '_')) {
                    buff.push_back(consume());
                }

                //check for the exit code !
                if (buff == "exit") {
                    tokens.push_back(Token{TokenType::exit});
                    buff.clear();
                    continue;
                }
                else if (buff == "let") { //let <variable_name>=<expr> //so this is my thinking!
                    tokens.push_back(Token{TokenType::let});
                    buff.clear();
                    continue;
                }
                else if (buff == "sum") {
                    tokens.push_back(Token{TokenType::sum}); //so i am making some custom function do it !
                    buff.clear(); //and this will help me to cal the sum of the vals !
                    continue;
                }
                else if (buff == "even") { //helps to get the even vals!
                    tokens.push_back(Token{TokenType::even});
                    buff.clear();
                    continue;
                }
                else if (buff == "abs") { //for absolute value!
                    tokens.push_back(Token{TokenType::abs}); //abs vals!
                    buff.clear();
                    continue;
                }
                else if (buff == "max") {
                    tokens.push_back(Token{TokenType::max}); //helps to get the max function!
                    buff.clear();
                    continue;
                }
                else if (buff =="n_fibonacci") {
                    tokens.push_back(Token{TokenType::n_fibonacci});//here we will cal the f(n) term for fibonacci
                    buff.clear(); //clears the damn thing out !
                    continue;

                }
                else if (buff == "gcd") {
                    tokens.push_back(Token{TokenType::gcd}); //helps to get the gcd of the numbers!
                    buff.clear();
                    continue;
                }
                else if (buff == "sqrt") { //helps for the sqrt one !
                    tokens.push_back(Token{TokenType::sqrt});
                    buff.clear();
                    continue;
                }
                else if (buff == "fact") { //helps to get the factorial of the vals !
                    tokens.push_back(Token{TokenType::fact});
                    buff.clear(); //clear it !
                    continue;
                }
                else if (buff == "min") { //here i am getting the min value to get the min one!
                    tokens.push_back(Token{TokenType::min}); // tokens
                    buff.clear();
                    continue;
                }
                else if (buff == "prime") { //here the checks for the prime !
                    tokens.push_back(Token{TokenType::prime});
                    buff.clear();
                    continue;
                }
                else if (buff == "palindrome") { //here we can check for the palindrome number!
                    tokens.push_back(Token{TokenType::palindrome});
                    buff.clear();
                }
                else {
                    //not a keyword so it is a variable name!
                    tokens.push_back(Token{TokenType::identifier, buff});
                    buff.clear();
                    continue;
                }
            }
            else if (std::isdigit(static_cast<unsigned char>(peak().value()))) {
                buff.push_back(consume()); //get the number here in her e 'exit 67; is my one !

                while (peak().has_value() &&
                       std::isdigit(static_cast<unsigned char>(peak().value()))) {
                    buff.push_back(consume());
                }

                tokens.push_back(Token{TokenType::int_literal, buff}); //getting the integer literal!
                buff.clear(); //clear the buffer!
                continue; //continue our work!
            }
            else if (peak().value() == '(') {
                consume();
                tokens.push_back(Token{TokenType::open_par});
                continue;
            }
            else if (peak().value() == ')') {
                consume();
                tokens.push_back(Token{TokenType::close_par});
                continue;
            }
            else if (peak().value() == ';') { //check for the end of the instructions that we have given!
                consume();
                tokens.push_back(Token{TokenType::semicolon}); //here we are getting the semicolon token type!
                continue;
            }
            else if (peak().value() == ',') { //and it helps us to get the values!
                consume();
                tokens.push_back(Token{TokenType::comma}); // this gets the comma values!
                continue;
            }
            else if (peak().value() == '+') {
                consume(); //gets the + sign and will do the work!
                tokens.push_back(Token{TokenType::plus});
                continue;
            }
            else if (peak().value() == '-') {
                consume(); //consumong it !
                tokens.push_back(Token{TokenType::minus}); //pusing it !
                continue; //we are working!
            }
            else if (peak().value() == '*') {
                consume();
                tokens.push_back(Token{TokenType::mul});
                continue;
            }
            else if (peak().value() == '/') {
                consume();
                tokens.push_back(Token{TokenType::division});
                continue;
            }
            else if (peak().value() == '=') {
                consume();
                tokens.push_back(Token{TokenType::equal}); //so this is the = one for the <expr>that we wanr@
                continue;
            }
            //check for the ' ' white spaces if white spaces then continue with the work!
            else if (std::isspace(static_cast<unsigned char>(peak().value()))) {
                consume();
                continue;
            }
            else {
                std::cerr << "Unexpected character! Invalid input: "
                          << peak().value() << std::endl;
                std::exit(EXIT_FAILURE);
            }
        }

        return tokens; //return this shitty tokens!
    }

private:
    std::optional<char> peak(int ahead = 0) const { //we want to peak it i mean just view it !
        if (m_index_vals + ahead >= m_source_val.size()) { //so the erros was to take from and see the current one!
            return std::nullopt; //return some kind of the garbage value!
        }
        else {
            return m_source_val.at(m_index_vals + ahead); //get the value!
        }
    }

    char consume() {
        //gets the consume function!
        return m_source_val.at(m_index_vals++); //getting the indexes updattion!
    }

    const std::string m_source_val;
    size_t m_index_vals = 0;
};
