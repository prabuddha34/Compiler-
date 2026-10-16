#include <cctype>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include "Tokenization.h"
    int main(int argc, char *argv[]) {
        //if we are not getting the value !and then there are any errors of them so we will return an error!
        if (argc != 2) {
            std::cerr << "Incorrect number of arguments" << std::endl;
            std::cerr << "Usage: Baal <input.hy>" << std::endl;
            return EXIT_FAILURE; //getting fancy here so that we can return them lol I mean cool you know >>?>
        }
        std::cout << argv[0] << std::endl; //just trying to get the first argument !
        //here we will get the path !

        //so i think i shall read the file lol !
        std::string content;
        {
            std::fstream input(argv[1], std::ios::in);
            if (!input) {
                std::cerr << "Could not open file" << std::endl;
                return EXIT_FAILURE;
            }
            std::stringstream content_stream;

            content_stream << input.rdbuf();
            content = content_stream.str();

            input.close(); //close it!
        }

        Tokenization tokenizer(content);
        std::vector<Token> tokens = tokenizer.tokenize(); //getting it to be tokenized by them !
        {
            std::fstream file("out.asm", std::ios::out);//making the assembly code!
            file << tokenizer.tokens_to_asm(tokens);
        }
        return EXIT_SUCCESS; //UM getting the code for it and i want 67 to be my exit code !
    }