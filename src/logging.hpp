#pragma once

#include <iostream>

#include "exceptions.hpp"
#include "lexeme.hpp"
#include "terminal_colors.hpp"

namespace log {
    void error(const std::wstring & code, const std::vector<Lexeme> & lexemes, const SyntaxAnalysisError & error) {
        size_t index = lexemes[error.GetIndex()].GetIndex();

        // Getting lexeme position in file
        size_t lineIndex = 1, columnIndex = 0, lineStartIndex = 0;
        for (size_t i = 0; i < index; ++i, ++columnIndex) {
            if (code[i] == '\n') {
                columnIndex = 0;
                ++lineIndex;
                lineStartIndex = i + 1;
            }
        }

        // Printing error info
        std::wcout << color::bright << lineIndex << ':' << columnIndex << ": ";
        std::wcout << color::red << "error: " << color::reset << color::bright << "unexpected lexeme" << color::reset;

        if (error.GetIndex() == lexemes.size()) {
            std::wcout << color::bright << " (";
            std::wcout << "expected: " << color::blue << ToString(error.GetExpected()) << color::reset;
            std::wcout << color::bright << ", got: " << color::red << "EOF" << color::reset;
            std::wcout << color::bright << ')' << color::reset;
            --index;
        } else {
            std::wcout << color::bright << " (";
            std::wcout << "expected: " << color::blue << ToString(error.GetExpected()) << color::reset;
            std::wcout << color::bright << ", got: " << color::red << ToString(lexemes[error.GetIndex()].GetType()) << color::reset;
            std::wcout << color::bright << ')' << color::reset;
        }
        std::wcout << std::endl;

        // Printing line with error
        for (size_t i = lineStartIndex; i < code.size() && code[i] != '\n'; ++i) {
            if (i == index) std::wcout << color::background::red << color::white;
            std::wcout << code[i];
            if (i == index + lexemes[error.GetIndex()].GetValue().size() - 1) std::wcout << color::reset;
        }
        std::wcout << std::endl << std::endl;
    }

    void warning(const std::wstring & code, const std::vector<Lexeme> & lexemes, uint64_t ind) {
        // For testing
        size_t index = lexemes[ind].GetIndex();

        // Getting lexeme position in file
        size_t lineIndex = 1, columnIndex = 0, lineStartIndex = 0;
        for (size_t i = 0; i < index; ++i, ++columnIndex) {
            if (code[i] == '\n') {
                columnIndex = 0;
                ++lineIndex;
                lineStartIndex = i + 1;
            }
        }

        // Printing warning info
        std::wcout << color::bright << lineIndex << ':' << columnIndex << ": ";
        std::wcout << color::blue << "warning: " << color::reset;
        std::wcout << color::bright << "converting " << color::green << "string" << color::reset;
        std::wcout << color::bright << " to " << color::red << "bool" << color::reset;
        std::wcout << std::endl;

        // Printing line with error
        for (size_t i = lineStartIndex; i < code.size() && code[i] != '\n'; ++i) {
            if (i == index) std::wcout << color::background::blue << color::white;
            std::wcout << code[i];
            if (i == index + lexemes[ind].GetValue().size() - 1) std::wcout << color::reset;
        }
        std::wcout << std::endl << std::endl;
    }
}