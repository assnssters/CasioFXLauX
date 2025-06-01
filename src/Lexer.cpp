#include "Lexer.h"
#include <cctype> // For isalpha, isdigit, isspace
#include <iostream>

Lexer::Lexer(const std::string& source)
    : source_code(source), current_pos(0), current_line(1), current_column(1) {}

char Lexer::peek() {
    if (current_pos >= source_code.length()) {
        return '\0'; // End of file
    }
    return source_code[current_pos];
}

char Lexer::consume() {
    char c = peek();
    if (c != '\0') {
        current_pos++;
        if (c == '\n') {
            current_line++;
            current_column = 1;
        } else {
            current_column++;
        }
    }
    return c;
}

void Lexer::skipWhitespace() {
    while (isspace(peek())) {
        consume();
    }
}

Token Lexer::readIdentifier() {
    std::string id_str;
    int start_col = current_column;
    while (isalnum(peek()) || peek() == '_') {
        id_str += consume();
    }

    // Kiểm tra từ khóa
    if (id_str == "VAR") return Token(TokenType::VAR, id_str, current_line, start_col);
    if (id_str == "MEM_WRITE") return Token(TokenType::MEM_WRITE, id_str, current_line, start_col);
    if (id_str == "MEM_READ") return Token(TokenType::MEM_READ, id_str, current_line, start_col);
    if (id_str == "PRINT_CHAR") return Token(TokenType::PRINT_CHAR, id_str, current_line, start_col);
    
    return Token(TokenType::IDENTIFIER, id_str, current_line, start_col);
}

Token Lexer::readNumber() {
    std::string num_str;
    int start_col = current_column;
    while (isdigit(peek())) {
        num_str += consume();
    }
    return Token(TokenType::INTEGER_LITERAL, num_str, current_line, start_col);
}

Token Lexer::getNextToken() {
    skipWhitespace();

    if (peek() == '\0') {
        return Token(TokenType::END_OF_FILE, "", current_line, current_column);
    }

    char c = peek();
    int start_col = current_column;

    if (isalpha(c) || c == '_') {
        return readIdentifier();
    }

    if (isdigit(c)) {
        return readNumber();
    }

    switch (c) {
        case '=': consume(); return Token(TokenType::ASSIGN, "=", current_line, start_col);
        case '+': consume(); return Token(TokenType::PLUS, "+", current_line, start_col);
        case '-': consume(); return Token(TokenType::MINUS, "-", current_line, start_col);
        case ';': consume(); return Token(TokenType::SEMICOLON, ";", current_line, start_col);
        case '(': consume(); return Token(TokenType::LPAREN, "(", current_line, start_col);
        case ')': consume(); return Token(TokenType::RPAREN, ")", current_line, start_col);
        case ',': consume(); return Token(TokenType::COMMA, ",", current_line, start_col);
        // ... thêm các toán tử và ký tự khác
        default:
            std::cerr << "Lỗi Lexer: Ký tự không hợp lệ '" << c << "' tại dòng "
                      << current_line << ", cột " << current_column << std::endl;
            consume(); // Bỏ qua ký tự lỗi để tiếp tục
            return Token(TokenType::UNKNOWN, std::string(1, c), current_line, start_col);
    }
}
