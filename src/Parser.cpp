#include "Parser.h"
#include <iostream>
#include <stdexcept> // For std::runtime_error

// --- SymbolTable Implementation ---
int SymbolTable::get_next_address(int size_bytes) {
    int addr = next_available_address;
    next_available_address += size_bytes;
    return addr;
}

void SymbolTable::add_symbol(const std::string& name) {
    if (symbols.count(name)) {
        std::cerr << "Lỗi ngữ nghĩa: Biến '" << name << "' đã được khai báo." << std::endl;
        return; // hoặc throw exception
    }
    symbols[name] = {get_next_address()};
    std::cout << "Khai báo biến: '" << name << "' tại địa chỉ 0x" << std::hex << symbols[name].address << std::dec << std::endl;
}

SymbolInfo* SymbolTable::get_symbol(const std::string& name) {
    if (symbols.count(name)) {
        return &symbols[name];
    }
    std::cerr << "Lỗi ngữ nghĩa: Biến '" << name << "' chưa được khai báo." << std::endl;
    return nullptr;
}


// --- Parser Implementation ---
Parser::Parser(Lexer& lexer) : lexer(lexer) {
    advance(); // Đọc token đầu tiên
}

void Parser::advance() {
    current_token = lexer.getNextToken();
}

void Parser::expect(TokenType type, const std::string& error_msg) {
    if (current_token.type != type) {
        throw std::runtime_error("Lỗi cú pháp tại dòng " + std::to_string(current_token.line) +
                                 ", cột " + std::to_string(current_token.column) + ": " +
                                 error_msg + ". Nhận được '" + current_token.value + "'");
    }
    advance();
}

std::unique_ptr<ProgramNode> Parser::parse() {
    auto program_node = std::make_unique<ProgramNode>();
    while (current_token.type != TokenType::END_OF_FILE) {
        program_node->statements.push_back(parseStatement());
    }
    return program_node;
}

std::unique_ptr<ASTNode> Parser::parseStatement() {
    if (current_token.type == TokenType::VAR) {
        advance(); // Consume VAR
        expect(TokenType::IDENTIFIER, "Cần tên biến sau VAR");
        std::string var_name = current_token.value;
        symbol_table.add_symbol(var_name); // Thêm biến vào bảng ký hiệu
        advance(); // Consume IDENTIFIER
        expect(TokenType::SEMICOLON, "Cần ';' sau khai báo biến");
        return std::make_unique<VarDeclarationNode>(var_name);
    } else if (current_token.type == TokenType::IDENTIFIER) {
        std::string var_name = current_token.value;
        // Kiểm tra biến đã khai báo chưa
        if (!symbol_table.get_symbol(var_name)) {
            throw std::runtime_error("Lỗi ngữ nghĩa: Biến '" + var_name + "' chưa được khai báo.");
        }
        advance(); // Consume IDENTIFIER
        expect(TokenType::ASSIGN, "Cần '=' sau tên biến trong lệnh gán");
        auto expr = parseExpression();
        expect(TokenType::SEMICOLON, "Cần ';' sau lệnh gán");
        return std::make_unique<AssignmentNode>(var_name, std::move(expr));
    } else if (current_token.type == TokenType::MEM_WRITE) {
        return parseMemWrite();
    } else if (current_token.type == TokenType::PRINT_CHAR) {
        return parsePrintChar();
    }
    // ... Thêm các loại lệnh khác
    throw std::runtime_error("Lỗi cú pháp: Lệnh không hợp lệ tại dòng " + std::to_string(current_token.line));
}

// Rất đơn giản, chỉ hỗ trợ cộng/trừ. Có thể mở rộng để ưu tiên toán tử
std::unique_ptr<ASTNode> Parser::parseExpression() {
    auto left_expr = parseTerm();
    while (current_token.type == TokenType::PLUS || current_token.type == TokenType::MINUS) {
        TokenType op_type = current_token.type;
        advance(); // Consume PLUS/MINUS
        auto right_expr = parseTerm();
        left_expr = std::make_unique<BinaryOpNode>(op_type, std::move(left_expr), std::move(right_expr));
    }
    return left_expr;
}

std::unique_ptr<ASTNode> Parser::parseTerm() {
    // Hiện tại chỉ là một placeholder, sẽ là nơi parse nhân/chia nếu có
    return parseFactor();
}

std::unique_ptr<ASTNode> Parser::parseFactor() {
    if (current_token.type == TokenType::INTEGER_LITERAL) {
        int val = std::stoi(current_token.value);
        advance(); // Consume INTEGER_LITERAL
        return std::make_unique<IntegerLiteralNode>(val);
    } else if (current_token.type == TokenType::IDENTIFIER) {
        std::string var_name = current_token.value;
        if (!symbol_table.get_symbol(var_name)) {
            throw std::runtime_error("Lỗi ngữ nghĩa: Biến '" + var_name + "' chưa được khai báo.");
        }
        advance(); // Consume IDENTIFIER
        return std::make_unique<IdentifierNode>(var_name);
    } else if (current_token.type == TokenType::LPAREN) {
        advance(); // Consume (
        auto expr = parseExpression();
        expect(TokenType::RPAREN, "Cần ')' sau biểu thức trong dấu ngoặc đơn");
        return expr;
    } else if (current_token.type == TokenType::MEM_READ) {
        advance(); // Consume MEM_READ
        expect(TokenType::LPAREN, "Cần '(' sau MEM_READ");
        auto addr_expr = parseExpression();
        expect(TokenType::RPAREN, "Cần ')' sau địa chỉ trong MEM_READ");
        return std::make_unique<MemReadNode>(std::move(addr_expr));
    }
    // ... Thêm các trường hợp khác
    throw std::runtime_error("Lỗi cú pháp: Yếu tố không hợp lệ tại dòng " + std::to_string(current_token.line) +
                             ", cột " + std::to_string(current_token.column) + ". Nhận được '" + current_token.value + "'");
}

std::unique_ptr<ASTNode> Parser::parseMemWrite() {
    advance(); // Consume MEM_WRITE
    expect(TokenType::LPAREN, "Cần '(' sau MEM_WRITE");
    auto addr_expr = parseExpression();
    expect(TokenType::COMMA, "Cần ',' sau địa chỉ trong MEM_WRITE");
    auto value_expr = parseExpression();
    expect(TokenType::RPAREN, "Cần ')' sau giá trị trong MEM_WRITE");
    expect(TokenType::SEMICOLON, "Cần ';' sau lệnh MEM_WRITE");
    return std::make_unique<MemWriteNode>(std::move(addr_expr), std::move(value_expr));
}

std::unique_ptr<ASTNode> Parser::parsePrintChar() {
    advance(); // Consume PRINT_CHAR
    expect(TokenType::LPAREN, "Cần '(' sau PRINT_CHAR");
    auto line_expr = parseExpression();
    expect(TokenType::COMMA, "Cần ',' sau dòng trong PRINT_CHAR");
    auto col_expr = parseExpression();
    expect(TokenType::COMMA, "Cần ',' sau cột trong PRINT_CHAR");
    auto char_code_expr = parseExpression();
    expect(TokenType::RPAREN, "Cần ')' sau mã ký tự trong PRINT_CHAR");
    expect(TokenType::SEMICOLON, "Cần ';' sau lệnh PRINT_CHAR");
    return std::make_unique<PrintCharNode>(std::move(line_expr), std::move(col_expr), std::move(char_code_expr));
}
