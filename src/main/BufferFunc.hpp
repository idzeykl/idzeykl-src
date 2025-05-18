#ifndef BUFFERFUNC_H
#define BUFFERFUNC_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

static std::ofstream outputFileStream;

static std::streambuf* originalCoutBuffer = nullptr;

bool isIdzeyKLFile(const std::string& fileName) {
    std::filesystem::path filePath(fileName);
    return filePath.extension() == ".idzey";
}

void redirectCoutStreamToFile(const std::string& fileName) {
    if (outputFileStream.is_open()) {
        outputFileStream.close();
    }

    outputFileStream.open(fileName, std::ios::out | std::ios::trunc);
    if (!outputFileStream.is_open()) {
        std::cerr << "Ошибка: Не удалось открыть файл для записи: " << fileName << "'\n";
        return;
    }

    originalCoutBuffer = std::cout.rdbuf();

    std::cout.rdbuf(outputFileStream.rdbuf());
}

std::string readFileIdzeyKL(const std::string& fileName) {
    try {
        if (!isIdzeyKLFile(fileName)) {
            std::cerr << "Ошибка: Неверный формат файла. Ожидался файл с расширением '.idzey'\n";
            return "";
        }

        std::ifstream inputFile(fileName, std::ios::in | std::ios::binary);
        if (!inputFile.is_open()) {
            std::cerr << "Ошибка: Не удалось открыть файл для чтения: " << fileName << "'\n";
            return "";
        }

        inputFile.seekg(0, std::ios::end);
        std::streampos fileSize = inputFile.tellg();
        inputFile.seekg(0, std::ios::beg);

        if (fileSize <= 0) {
            std::cerr << "Ошибка: Файл пуст: " << fileName << "\n";
            inputFile.close();
            return "";
        }

        std::string content(fileSize, '\0');
        inputFile.read(&content[0], fileSize);

        if (inputFile.fail()) {
            std::cerr << "Ошибка: Не удалось прочитать файл: " << fileName << "\n";
            inputFile.close();
            return "";
        }

        inputFile.close();
        return content;
    } catch (const std::exception& e) {
        std::cerr << "Ошибка при чтении файла: " << e.what() << "\n";
        return "";
    } catch (...) {
        std::cerr << "Неизвестная ошибка при чтении файла\n";
        return "";
    }
}

#endif //BUFFERFUNC_H
