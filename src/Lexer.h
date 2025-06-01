#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <map>

// Định nghĩa các loại Token
enum class TokenType {
    VAR,            // VAR (từ khóa)
    IDENTIFIER,     // Tên biến (my_var, counter)
    INTEGER_LITERAL,// Số nguyên (10, 5)
    ASSIGN,         // =
    PLUS,           // +
    MINUS,          // -
    SEMICOLON,      // ;
    LPAREN,         // (
    RPAREN,         // )
    COMMA,          // ,
    MEM_WRITE,      // MEM_WRITE (từ khóa)
    MEM_READ,       // MEM_READ (từ khóa)
    PRINT_CHAR,     // PRINT_CHAR (từ khóa màn hình Casio)
    END_OF_FILE,    // Kết thúc file
    UNKNOWN         // Token không xác định
};

// Cấu trúc một Token
struct Token {
    TokenType type;
    std::string value; // Giá trị của token (ví dụ: "my_var", "10")
    int line;
    int column;

    // Constructor tiện lợi
    Token(TokenType type, std::string value, int line, int column)
        : type(type), value(std::move(value)), line(line), column(column) {}
};

class Lexer {
public:
    explicit Lexer(const std::string& source);
    Token getNextToken();

private:
    std::string source_code;
    size_t current_pos;
    int current_line;
    int current_column;

    char peek();
    char consume();
    void skipWhitespace();
    Token readIdentifier();
    Token readNumber();
    // Các hàm hỗ trợ khác
};

#endif // LEXER_H
