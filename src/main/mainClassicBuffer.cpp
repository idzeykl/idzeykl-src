#include "../lexer/lexer.hpp"
#include "../parser/parser.hpp"
#include "../interpreter/interpreter.hpp"
#include "BufferFunc.hpp"
int main(int argc,char* argv[]) {
    const std::string inputName = argv[1];
    std::string source = readFileIdzeyKL(inputName);

    Lexer lexer(source);
    Parser parser(lexer);

    try {
        auto program = parser.parse();
        try {
            Interpreter interpreter;
            interpreter.interpret(std::move(program));

        } catch (const RuntimeError& error) {
            std::cerr << "Runtime Error: " << error.what() << std::endl;
            return 1;
        } catch (const std::exception& e) {
            std::cerr << "Exception: " << e.what() << std::endl;
            return 1;
        } catch (...) {
            std::cerr << "Unknown error occurred in interpreter" << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Parser Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred in parser" << std::endl;
        return 1;
    }

    return 0;
}
