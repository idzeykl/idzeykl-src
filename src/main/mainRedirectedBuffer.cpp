#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "BufferFunc.hpp"
#include "../interpreter/interpreter.hpp"
#include "../lexer/lexer.hpp"
#include "../parser/parser.hpp"



int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Ошибка: Недостаточно аргументов. Использование: " << argv[0] << " <входной_файл> <выходной_файл>\n";
        return 1;
    }

    const std::string inputName = argv[1];
    const std::string outputName = argv[2];

    std::string source = readFileIdzeyKL(inputName);

    if (source.empty()) {
        std::cerr << "Ошибка: Исходный код пуст или не удалось прочитать файл.\n";
        return 1;
    }

    redirectCoutStreamToFile(outputName);

    Lexer lexer(source);
    Parser parser(lexer);

    try {
        auto program = parser.parse();
        try {
            Interpreter interpreter;
            interpreter.interpret(std::move(program));

        } catch (const RuntimeError& error) {
            if (originalCoutBuffer != nullptr) {
                std::cout.flush();
                std::cout.rdbuf(originalCoutBuffer);
            }
            std::cerr << "Runtime Error: " << error.what() << std::endl;

            if (outputFileStream.is_open()) {
                outputFileStream.close();
            }
            return 1;
        } catch (const std::exception& e) {
            if (originalCoutBuffer != nullptr) {
                std::cout.flush();
                std::cout.rdbuf(originalCoutBuffer);
            }
            std::cerr << "Exception: " << e.what() << std::endl;
            if (outputFileStream.is_open()) {
                outputFileStream.close();
            }
            return 1;
        } catch (...) {
            if (originalCoutBuffer != nullptr) {
                std::cout.flush();
                std::cout.rdbuf(originalCoutBuffer);
            }
            std::cerr << "Unknown error occurred in interpreter" << std::endl;

            if (outputFileStream.is_open()) {
                outputFileStream.close();
            }
            return 1;
        }
    } catch (const std::exception& e) {
        if (originalCoutBuffer != nullptr) {
            std::cout.flush();
            std::cout.rdbuf(originalCoutBuffer);
        }
        std::cerr << "Parser Exception: " << e.what() << std::endl;

        if (outputFileStream.is_open()) {
            outputFileStream.close();
        }
        return 1;
    } catch (...) {
        if (originalCoutBuffer != nullptr) {
            std::cout.flush();
            std::cout.rdbuf(originalCoutBuffer);
        }
        std::cerr << "Unknown error occurred in parser" << std::endl;
        if (outputFileStream.is_open()) {
            outputFileStream.close();
        }
        return 1;
    }
    std::cout.flush();

    if (originalCoutBuffer != nullptr) {
        std::cout.rdbuf(originalCoutBuffer);
    }

    if (outputFileStream.is_open()) {
        outputFileStream.close();
    }

    return 0;
}