#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include <vector>
#include <string>
#include <map>
#include <memory> // For std::unique_ptr

// --- AST Node Definitions ---
// Base class for all Abstract Syntax Tree nodes
struct ASTNode {
    enum class NodeType {
        Program,
        VarDeclaration,
        Assignment,
        IntegerLiteral,
        BinaryOp,
        Identifier,
        MemWrite, // New node type for memory write
        MemRead,  // New node type for memory read
        PrintChar // New node type for print_char
    };
    NodeType type;
    virtual ~ASTNode() = default;
};

// Program node (root of the AST)
struct ProgramNode : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> statements;
    ProgramNode() { type = NodeType::Program; }
};

// Variable Declaration node
struct VarDeclarationNode : public ASTNode {
    std::string var_name;
    VarDeclarationNode(std::string name) : var_name(std::move(name)) { type = NodeType::VarDeclaration; }
};

// Assignment node: var_name = expression
struct AssignmentNode : public ASTNode {
    std::string var_name;
    std::unique_ptr<ASTNode> expression;
    AssignmentNode(std::string name, std::unique_ptr<ASTNode> expr)
        : var_name(std::move(name)), expression(std::move(expr)) { type = NodeType::Assignment; }
};

// Integer Literal node (e.g., 10, 255)
struct IntegerLiteralNode : public ASTNode {
    unsigned int value;
    IntegerLiteralNode(unsigned int val) : value(val) { type = NodeType::IntegerLiteral; }
};

// Binary Operation node (e.g., a + b, x - y)
struct BinaryOpNode : public ASTNode {
    TokenType op; // PLUS, MINUS, MULT, DIV
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
    BinaryOpNode(TokenType op_type, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r)
        : op(op_type), left(std::move(l)), right(std::move(r)) { type = NodeType::BinaryOp; }
};

// Identifier node (variable name)
struct IdentifierNode : public ASTNode {
    std::string name;
    IdentifierNode(std::string var_name) : name(std::move(var_name)) { type = NodeType::Identifier; }
};

// Memory Write node: [address_expr] = value_expr
struct MemWriteNode : public ASTNode {
    std::unique_ptr<ASTNode> address_expr;
    std::unique_ptr<ASTNode> value_expr;
    MemWriteNode(std::unique_ptr<ASTNode> addr_expr, std::unique_ptr<ASTNode> val_expr)
        : address_expr(std::move(addr_expr)), value_expr(std::move(val_expr)) { type = NodeType::MemWrite; }
};

// Memory Read node: [address_expr] (used in expressions)
struct MemReadNode : public ASTNode {
    std::unique_ptr<ASTNode> address_expr;
    MemReadNode(std::unique_ptr<ASTNode> addr_expr)
        : address_expr(std::move(addr_expr)) { type = NodeType::MemRead; }
};

// PRINT_CHAR node: PRINT_CHAR(line, column, char_code)
struct PrintCharNode : public ASTNode {
    std::unique_ptr<ASTNode> line_expr;
    std::unique_ptr<ASTNode> column_expr;
    std::unique_ptr<ASTNode> char_code_expr;
    PrintCharNode(std::unique_ptr<ASTNode> line, std::unique_ptr<ASTNode> col, std::unique_ptr<ASTNode> code)
        : line_expr(std::move(line)), column_expr(std::move(col)), char_code_expr(std::move(code)) {
        type = NodeType::PrintChar;
    }
};

// --- Symbol Table ---
struct SymbolInfo {
    unsigned int address;
    // Thêm các thông tin khác nếu cần (ví dụ: kích thước, kiểu dữ liệu)
};

class SymbolTable {
public:
    std::map<std::string, SymbolInfo> symbols;
    unsigned int next_available_address = 0x2000; // Start variable addresses from 0x2000

    unsigned int get_next_address(unsigned int size_bytes = 2); // Giả định 2 byte cho các biến

    void add_symbol(const std::string& name);
    // Sửa chữa: Thêm 'const' vào kiểu trả về và cuối hàm
    const SymbolInfo* get_symbol(const std::string& name) const;
};


// --- Parser Class ---
class Parser {
public:
    explicit Parser(Lexer& lexer);
    std::unique_ptr<ProgramNode> parse();

private:
    Lexer& lexer;
    Token current_token; // Sửa chữa: Khởi tạo trong constructor

    void advance(); // Move to the next token
    void expect(TokenType type); // Consume current token and expect next token to be of a certain type

    // Parsing functions for different grammar rules
    std::unique_ptr<ASTNode> parse_statement();
    std::unique_ptr<ASTNode> parse_var_declaration();
    std::unique_ptr<ASTNode> parse_assignment();
    std::unique_ptr<ASTNode> parse_mem_write();
    std::unique_ptr<ASTNode> parse_print_char();

    std::unique_ptr<ASTNode> parse_expression();
    std::unique_ptr<ASTNode> parse_term();     // Handles multiplication and division
    std::unique_ptr<ASTNode> parse_factor();   // Handles numbers, identifiers, and parentheses, memory reads

    SymbolTable symbol_table; // Symbol table instance
};

#endif // PARSER_H
