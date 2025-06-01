#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include <memory> // For std::unique_ptr
#include <vector>
#include <map>

// --- AST Node Definitions ---
// Lớp cơ sở cho tất cả các nút AST
class ASTNode {
public:
    enum class NodeType {
        Program,
        VarDeclaration,
        Assignment,
        BinaryOp,
        IntegerLiteral,
        Identifier,
        MemWrite,
        MemRead,
        PrintChar,
        // ... thêm các loại nút khác
    };
    NodeType type;
    explicit ASTNode(NodeType t) : type(t) {}
    virtual ~ASTNode() = default;
};

// Nút cho Chương trình (Program)
class ProgramNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> statements;
    ProgramNode() : ASTNode(NodeType::Program) {}
};

// Nút cho Khai báo biến (VAR identifier;)
class VarDeclarationNode : public ASTNode {
public:
    std::string var_name;
    explicit VarDeclarationNode(std::string name) : ASTNode(NodeType::VarDeclaration), var_name(std::move(name)) {}
};

// Nút cho Gán giá trị (identifier = expression;)
class AssignmentNode : public ASTNode {
public:
    std::string var_name;
    std::unique_ptr<ASTNode> expression;
    AssignmentNode(std::string name, std::unique_ptr<ASTNode> expr)
        : ASTNode(NodeType::Assignment), var_name(std::move(name)), expression(std::move(expr)) {}
};

// Nút cho Phép toán hai ngôi (expression op expression)
class BinaryOpNode : public ASTNode {
public:
    TokenType op; // PLUS, MINUS
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
    BinaryOpNode(TokenType op_type, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r)
        : ASTNode(NodeType::BinaryOp), op(op_type), left(std::move(l)), right(std::move(r)) {}
};

// Nút cho Số nguyên (10, 5)
class IntegerLiteralNode : public ASTNode {
public:
    int value;
    explicit IntegerLiteralNode(int val) : ASTNode(NodeType::IntegerLiteral), value(val) {}
};

// Nút cho Tên biến (my_var, counter)
class IdentifierNode : public ASTNode {
public:
    std::string name;
    explicit IdentifierNode(std::string n) : ASTNode(NodeType::Identifier), name(std::move(n)) {}
};

// Nút cho MEM_WRITE(addr, value)
class MemWriteNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> address_expr;
    std::unique_ptr<ASTNode> value_expr;
    MemWriteNode(std::unique_ptr<ASTNode> addr, std::unique_ptr<ASTNode> val)
        : ASTNode(NodeType::MemWrite), address_expr(std::move(addr)), value_expr(std::move(val)) {}
};

// Nút cho MEM_READ(addr)
class MemReadNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> address_expr;
    explicit MemReadNode(std::unique_ptr<ASTNode> addr)
        : ASTNode(NodeType::MemRead), address_expr(std::move(addr)) {}
};

// Nút cho PRINT_CHAR(line, column, char_code)
class PrintCharNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> line_expr;
    std::unique_ptr<ASTNode> column_expr;
    std::unique_ptr<ASTNode> char_code_expr;
    PrintCharNode(std::unique_ptr<ASTNode> line, std::unique_ptr<ASTNode> col, std::unique_ptr<ASTNode> code)
        : ASTNode(NodeType::PrintChar), line_expr(std::move(line)), column_expr(std::move(col)), char_code_expr(std::move(code)) {}
};


// --- Symbol Table (Bảng Ký hiệu) ---
// Ánh xạ tên biến FxLaux đến địa chỉ bộ nhớ NX-U8
struct SymbolInfo {
    int address; // Địa chỉ bắt đầu của biến trong bộ nhớ NX-U8
    // Có thể thêm kích thước, kiểu dữ liệu, v.v.
};

class SymbolTable {
public:
    std::map<std::string, SymbolInfo> symbols;
    int next_available_address = 0x2000; // Địa chỉ bắt đầu cho biến toàn cục (ví dụ)
    // Casio VRAM là 0x01-0x31 (nhỏ), dùng vùng khác cho biến
    // Đảm bảo không trùng với VRAM hay các vùng đặc biệt khác của NX-U8
    int get_next_address(int size_bytes = 2); // Giả sử biến 2 bytes (16-bit)

    void add_symbol(const std::string& name);
    SymbolInfo* get_symbol(const std::string& name);
};


// --- Parser Class ---
class Parser {
public:
    explicit Parser(Lexer& lexer);
    std::unique_ptr<ProgramNode> parse();
    SymbolTable symbol_table; // Bảng ký hiệu của Parser

private:
    Lexer& lexer;
    Token current_token;

    void advance();
    void expect(TokenType type, const std::string& error_msg);
    std::unique_ptr<ASTNode> parseStatement();
    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<ASTNode> parseTerm(); // Xử lý nhân/chia (nếu có)
    std::unique_ptr<ASTNode> parseFactor(); // Xử lý số, biến, dấu ngoặc
    std::unique_ptr<ASTNode> parseMemWrite();
    std::unique_ptr<ASTNode> parseMemRead();
    std::unique_ptr<ASTNode> parsePrintChar();
};

#endif // PARSER_H
