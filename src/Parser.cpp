#include "Parser.h"
#include <iostream>
#include <stdexcept>

// --- SymbolTable Implementation ---
unsigned int SymbolTable::get_next_address(unsigned int size_bytes) {
    unsigned int current_addr = next_available_address;
    next_available_address += size_bytes;
    return current_addr;
}

void SymbolTable::add_symbol(const std::string& name) {
    if (symbols.count(name)) {
        std::cerr << "Lỗi ngữ nghĩa: Biến '" << name << "' đã được khai báo." << std::endl;
        return;
    }
    SymbolInfo info;
    info.address = get_next_address(); // Assign an address to the new variable
    symbols[name] = info;
    std::cout << "DEBUG: Biến '" << name << "' được gán địa chỉ: 0x" << std::hex << info.address << std::dec << std::endl;
}

// Sửa chữa: Thêm 'const' vào kiểu trả về và cuối hàm
const SymbolInfo* SymbolTable::get_symbol(const std::string& name) const {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        return &it->second; // Trả về con trỏ const
    }
    std::cerr << "Lỗi ngữ nghĩa: Biến '" << name << "' chưa được khai báo." << std::endl;
    return nullptr;
}


// --- Parser Implementation ---

// Sửa chữa: Khởi tạo current_token ngay tại danh sách khởi tạo
Parser::Parser(Lexer& lexer) : lexer(lexer), current_token(lexer.getNextToken()) {
    // Không cần gọi advance() ở đây nữa vì current_token đã được khởi tạo
}

void Parser::advance() {
    current_token = lexer.getNextToken();
    // std::cout << "DEBUG: Advanced to token: " << tokenTypeToString(current_token.type)
    //           << " ('" << current_token.value << "')" << std::endl;
}

void Parser::expect(TokenType type) {
    if (current_token.type == type) {
        advance();
    } else {
        std::string error_msg = "Lỗi cú pháp tại dòng ";
        error_msg += std::to_string(current_token.line) + ", cột ";
        error_msg += std::to_string(current_token.column) + ": ";
        error_msg += "Mong đợi '" + tokenTypeToString(type) + "' nhưng nhận được '" + current_token.value + "' ('" + tokenTypeToString(current_token.type) + "').";
        throw std::runtime_error(error_msg);
    }
}

std::unique_ptr<ProgramNode> Parser::parse() {
    auto program_node = std::make_unique<ProgramNode>();
    while (current_token.type != TokenType::EOF_TOKEN) {
        program_node->statements.push_back(parse_statement());
    }
    return program_node;
}

std::unique_ptr<ASTNode> Parser::parse_statement() {
    if (current_token.type == TokenType::VAR) {
        return parse_var_declaration();
    } else if (current_token.type == TokenType::IDENTIFIER) {
        // Có thể là lệnh gán hoặc lệnh gọi hàm sau này
        // Tạm thời coi là lệnh gán nếu tiếp theo là ASSIGN
        // Hoặc là MemWrite nếu là '['
        Token peek_token = lexer.peekNextToken(); // Xem trước token tiếp theo
        if (peek_token.type == TokenType::ASSIGN) {
            return parse_assignment();
        } else if (peek_token.type == TokenType::OPEN_BRACKET) {
            return parse_mem_write();
        }
        else {
            throw std::runtime_error("Lỗi cú pháp không xác định sau định danh tại dòng " + std::to_string(current_token.line));
        }
    } else if (current_token.type == TokenType::PRINT_CHAR_KW) {
        return parse_print_char();
    }
    else {
        throw std::runtime_error("Lỗi cú pháp: Mong đợi khai báo biến, gán, hoặc lệnh tại dòng " + std::to_string(current_token.line));
    }
}

std::unique_ptr<ASTNode> Parser::parse_var_declaration() {
    expect(TokenType::VAR);
    std::string var_name = current_token.value;
    expect(TokenType::IDENTIFIER);
    expect(TokenType::SEMICOLON);

    symbol_table.add_symbol(var_name); // Add variable to symbol table
    return std::make_unique<VarDeclarationNode>(var_name);
}

std::unique_ptr<ASTNode> Parser::parse_assignment() {
    std::string var_name = current_token.value;
    expect(TokenType::IDENTIFIER);
    expect(TokenType::ASSIGN);
    auto expr = parse_expression();
    expect(TokenType::SEMICOLON);
    return std::make_unique<AssignmentNode>(var_name, std::move(expr));
}

std::unique_ptr<ASTNode> Parser::parse_mem_write() {
    std::string var_name_or_base_addr_id = current_token.value; // Có thể là tên biến chứa base addr
    expect(TokenType::IDENTIFIER); // Consume the identifier (e.g., 'MEM')

    expect(TokenType::OPEN_BRACKET); // Consume '['
    auto address_expr = parse_expression(); // Parse the address expression
    expect(TokenType::CLOSE_BRACKET); // Consume ']'

    expect(TokenType::ASSIGN); // Consume '='
    auto value_expr = parse_expression(); // Parse the value expression
    expect(TokenType::SEMICOLON); // Consume ';'

    return std::make_unique<MemWriteNode>(std::move(address_expr), std::move(value_expr));
}

std::unique_ptr<ASTNode> Parser::parse_print_char() {
    expect(TokenType::PRINT_CHAR_KW);
    expect(TokenType::OPEN_PAREN);
    auto line_expr = parse_expression();
    expect(TokenType::COMMA);
    auto column_expr = parse_expression();
    expect(TokenType::COMMA);
    auto char_code_expr = parse_expression();
    expect(TokenType::CLOSE_PAREN);
    expect(TokenType::SEMICOLON);
    return std::make_unique<PrintCharNode>(std::move(line_expr), std::move(column_expr), std::move(char_code_expr));
}


std::unique_ptr<ASTNode> Parser::parse_expression() {
    auto node = parse_term(); // Start with term (multiplication/division)

    while (current_token.type == TokenType::PLUS || current_token.type == TokenType::MINUS) {
        TokenType op_type = current_token.type;
        advance();
        auto right = parse_term();
        node = std::make_unique<BinaryOpNode>(op_type, std::move(node), std::move(right));
    }
    return node;
}

std::unique_ptr<ASTNode> Parser::parse_term() {
    auto node = parse_factor(); // Start with factor (numbers, identifiers, parentheses, memory reads)

    while (current_token.type == TokenType::MULTIPLY || current_token.type == TokenType::DIVIDE) {
        TokenType op_type = current_token.type;
        advance();
        auto right = parse_factor();
        node = std::make_unique<BinaryOpNode>(op_type, std::move(node), std::move(right));
    }
    return node;
}

std::unique_ptr<ASTNode> Parser::parse_factor() {
    std::unique_ptr<ASTNode> node;
    if (current_token.type == TokenType::INTEGER) {
        node = std::make_unique<IntegerLiteralNode>(std::stoul(current_token.value));
        advance();
    } else if (current_token.type == TokenType::IDENTIFIER) {
        Token peek_token = lexer.peekNextToken(); // Peek to check for array/memory access
        if (peek_token.type == TokenType::OPEN_BRACKET) { // If it's like VAR[EXPR]
            std::string base_id = current_token.value;
            expect(TokenType::IDENTIFIER); // Consume the identifier (e.g., 'MEM')
            expect(TokenType::OPEN_BRACKET); // Consume '['
            auto address_expr = parse_expression(); // Parse the address expression
            expect(TokenType::CLOSE_BRACKET); // Consume ']'
            node = std::make_unique<MemReadNode>(std::move(address_expr));
        } else { // Just an identifier (variable)
            node = std::make_unique<IdentifierNode>(current_token.value);
            // Semantic check: ensure identifier is declared
            if (!symbol_table.get_symbol(current_token.value)) {
                 throw std::runtime_error("Lỗi ngữ nghĩa: Biến '" + current_token.value + "' chưa được khai báo tại dòng " + std::to_string(current_token.line));
            }
            advance();
        }
    } else if (current_token.type == TokenType::OPEN_PAREN) {
        advance();
        node = parse_expression();
        expect(TokenType::CLOSE_PAREN);
    } else {
        std::string error_msg = "Lỗi cú pháp: Mong đợi số nguyên, định danh, hoặc '(' tại dòng ";
        error_msg += std::to_string(current_token.line) + ", cột ";
        error_msg += std::to_string(current_token.column) + ". Nhận được '" + current_token.value + "'.";
        throw std::runtime_error(error_msg);
    }
    return node;
}
