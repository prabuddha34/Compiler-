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
    int_literal,
    semicolon, exit
};

struct Token {
    //getting the tokens of it !
    TokenType type; //getting which type of token we have to use have to here ! to do the job !
    std::optional<std::string> value = std::nullopt; // so my thought is that i am just assuming every thing to be in string !and then do the operations!
};

class Tokenization {
    public:
    inline Tokenization(const std::string &source)
    :m_source_val(source)
    {

    }
    //make a tokenize vectors that can take it !
    inline std::vector<Token>tokenize() {
        std::string buff;
        while (peak().has_value()) {
            if (std::isalpha(peak().value()));
        }
           std::vector<Token> tokens;//getting the token array !
    /*for (char c : content) {
        std::cout << c << std::endl; //hence we are printing the each one of them !
    }*/
    for (size_t i=0;i<m_source_val.size();i++) {
        char ch= m_source_val.at(i); //it gets the character!
        //lets get the buffer array !
        std::string buff="";//gett it until we get the alpha numeric one and we dont want to star like this ';return not make sense

        if (std::isalpha(static_cast<unsigned char>(ch))) {
            //get it into the buffer ig
            buff.push_back(ch);
            i++;
            while (i < m_source_val.size() && std::isalnum(static_cast<unsigned char>(m_source_val.at(i)))) { //while the value is alpha numeric !of it !
                buff.push_back(m_source_val.at(i));
                i++;
            }
            i--; //getting for the keyword i am taking it !
            //here tally it to 'return' for now!
            if (buff =="exit") {
                //here if we get a 'return' keyword then show us the exit code of it !
                tokens.push_back(Token{TokenType::exit});//pushing it here
                //clear the buffer!
                buff.clear();
                continue;

            }
            else {
                //i think we shall give an error code!
                std::cerr << "Unexpected character !LOL Mistake!" << buff << std::endl; //custom error message!
                exit(EXIT_FAILURE); //so we are exiting from here!

            }

        }
        //check for the ' ' white spaces if white spaces then continue with the work!
        else if (std::isdigit(static_cast<unsigned char>(ch))) {
            //get the digits!
            buff.push_back(ch); //get the numbers!
            i++; //update it
            while (i < m_source_val.size() && std::isdigit(static_cast<unsigned char>(m_source_val.at(i)))) {
                buff.push_back(m_source_val.at(i));
                i++;
            }
            i--;
            tokens.push_back(Token{TokenType::int_literal, buff}); //getting the int value!
        }
        else if (ch == ';') {
            tokens.push_back(Token{TokenType::semicolon});
        }

        else if (std::isspace(static_cast<unsigned char>(ch))) {
            continue;
        }
        else {
            std::cerr << "Unexpected character !LOL Mistake! YOU SCREWD" << std::endl;
            exit(EXIT_FAILURE);
        }

    }
    return tokens;//return this shitty tokens!
}


std::string tokens_to_asm(const std::vector<Token>& tokens) {
    std::stringstream output;
    output << "global _start\n_start:\n";

    for (size_t i = 0; i < tokens.size(); i++) {
        const Token& token = tokens.at(i);

        if (token.type == TokenType::exit) {
            // expecting: return <int_literal> ;
            if (i + 2 < tokens.size()
                && tokens.at(i + 1).type == TokenType::int_literal
                && tokens.at(i + 2).type == TokenType::semicolon) {

                output << "    mov rax, 60\n";                         // syscall number for exit
                output << "    mov rdi, " << *tokens.at(i + 1).value << "\n"; // exit code
                output << "    syscall\n";
                i += 2; // skip the literal and the semicolon we just consumed
                }
            else {
                std::cerr << "Syntax error: expected 'return <int>;'" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }
    return output.str();
    }
private:

    std::optional <char>peak() const{ //we want to peak it i mean just view it !
        if (m_index_vals+1>=m_source_val.size()) {
            return std::nullopt; //return some kind of the garbage value!
        }
        else {
            return m_source_val.at(m_index_vals); //get the value!
        }

    }
    char consume() {
        //gets the consume function!
       return m_source_val.at(m_index_vals++);//getting the indexes updattion!


    }
    const std::string m_source_val;
    int m_index_vals;


};