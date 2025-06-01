#include "ROPGenerator.h"
#include <iostream>
#include <stdexcept>

// --- GadgetDB Implementation ---
void GadgetDB::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Không thể mở file gadget: " + filepath);
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string func_str;
        std::string addr_str;
        ss >> func_str >> addr_str; // Định dạng ví dụ: POP_R0_RET 0x12345678

        unsigned int addr = std::stoul(addr_str, nullptr, 16);

        // Ánh xạ chuỗi sang enum GadgetFunction
        if (func_str == "POP_R0_RET") gadget_map[GadgetFunction::POP_R0_RET] = addr;
        else if (func_str == "POP_R1_RET") gadget_map[GadgetFunction::POP_R1_RET] = addr;
        else if (func_str == "POP_R2_RET") gadget_map[GadgetFunction::POP_R2_RET] = addr;
        else if (func_str == "ADD_R0_R1_RET") gadget_map[GadgetFunction::ADD_R0_R1_RET] = addr;
        else if (func_str == "SUB_R0_R1_RET") gadget_map[GadgetFunction::SUB_R0_R1_RET] = addr;
        else if (func_str == "LOAD_R0_FROM_R1_RET") gadget_map[GadgetFunction::LOAD_R0_FROM_R1_RET] = addr;
        else if (func_str == "STORE_R0_TO_R1_RET") gadget_map[GadgetFunction::STORE_R0_TO_R1_RET] = addr;
        else if (func_str == "SYSCALL_PRINT_CHAR") gadget_map[GadgetFunction::SYSCALL_PRINT_CHAR] = addr;
        else {
            std::cerr << "Cảnh báo: Chức năng gadget không xác định trong file: " << func_str << std::endl;
        }
    }
    std::cout << "Đã tải " << gadget_map.size() << " gadget." << std::endl;
}

unsigned int GadgetDB::getAddress(GadgetFunction func) const {
    auto it = gadget_map.find(func);
    if (it != gadget_map.end()) {
        return it->second;
    }
    throw std::runtime_error("Lỗi: Không tìm thấy gadget cho chức năng: " + std::to_string(static_cast<int>(func)));
}

// --- ROPGenerator Implementation ---
ROPGenerator::ROPGenerator(const GadgetDB& db, const SymbolTable& sym_table)
    : gadget_db(db), symbol_table(sym_table) {}

void ROPGenerator::pushGadget(GadgetFunction func) {
    rop_chain.push_back(gadget_db.getAddress(func));
}

void ROPGenerator::pushData(unsigned int data) {
    rop_chain.push_back(data);
}

std::vector<unsigned int> ROPGenerator::generateROPChain(const ProgramNode& program_node) {
    rop_chain.clear();
    for (const auto& statement : program_node.statements) {
        generateForNode(*statement);
    }
    return rop_chain;
}

void ROPGenerator::generateForNode(const ASTNode& node) {
    switch (node.type) {
        case ASTNode::NodeType::Assignment:
            generateForAssignment(static_cast<const AssignmentNode&>(node));
            break;
        case ASTNode::NodeType::MemWrite:
            generateForMemWrite(static_cast<const MemWriteNode&>(node));
            break;
        case ASTNode::NodeType::PrintChar:
            generateForPrintChar(static_cast<const PrintCharNode&>(node));
            break;
        case ASTNode::NodeType::VarDeclaration:
            // Khai báo biến không sinh ROP trực tiếp (chỉ cập nhật bảng ký hiệu)
            break;
        // ... Thêm các case cho các loại nút AST khác
        default:
            std::cerr << "Cảnh báo: Không thể sinh ROP cho loại nút AST này: " << static_cast<int>(node.type) << std::endl;
            break;
    }
}

// Hàm này sẽ đánh giá biểu thức và đưa kết quả vào thanh ghi R0
// Nó sẽ trả về 0x01 nếu kết quả trong R0, hoặc 0x02 nếu trong R1, vv.
// Để đơn giản, giả sử luôn kết thúc với kết quả trong R0
unsigned int ROPGenerator::evaluateExpressionIntoRegister(const ASTNode& expr_node) {
    if (expr_node.type == ASTNode::NodeType::IntegerLiteral) {
        const auto& int_node = static_cast<const IntegerLiteralNode&>(expr_node);
        pushGadget(GadgetFunction::POP_R0_RET); // Đẩy gadget pop R0
        pushData(int_node.value);             // Đẩy giá trị
        return 0; // Ký hiệu R0
    } else if (expr_node.type == ASTNode::NodeType::Identifier) {
        const auto& id_node = static_cast<const IdentifierNode&>(expr_node);
        SymbolInfo* sym = symbol_table.get_symbol(id_node.name);
        if (!sym) throw std::runtime_error("Lỗi: Biến '" + id_node.name + "' chưa khai báo.");

        pushGadget(GadgetFunction::POP_R1_RET);     // Load địa chỉ biến vào R1
        pushData(sym->address);
        pushGadget(GadgetFunction::LOAD_R0_FROM_R1_RET); // Load giá trị từ [R1] vào R0
        return 0; // Ký hiệu R0
    } else if (expr_node.type == ASTNode::NodeType::BinaryOp) {
        const auto& bin_op_node = static_cast<const BinaryOpNode&>(expr_node);

        // Đánh giá vế trái vào R0
        evaluateExpressionIntoRegister(*bin_op_node.left); // R0 = left_expr_value

        // Đánh giá vế phải vào R1
        pushGadget(GadgetFunction::POP_R2_RET); // Push R2 (sẽ dùng để lưu địa chỉ của R1_POP_GADGET)
        pushData(gadget_db.getAddress(GadgetFunction::POP_R1_RET)); // Đẩy địa chỉ của POP_R1_RET
        pushGadget(GadgetFunction::POP_R2_RET); // Push R2 (sẽ dùng để lưu địa chỉ của value)
        pushData(evaluateExpressionIntoRegister(*bin_op_node.right)); // Đẩy giá trị của right_expr (giả sử nó sẽ được pop vào R1)
        // Đây là một cách đơn giản, thực tế cần phức tạp hơn để tránh ghi đè R0
        // và đảm bảo đúng thứ tự pop của các gadget.
        // Cần nhiều gadget POP_Ri_RET và logic quản lý thanh ghi.
        // Ví dụ:
        // pushGadget(POP_R1_RET); // Pop R1
        // pushData(evaluateExpressionIntoRegister(*bin_op_node.right)); // Value of right operand
        // Lúc này R0 có left_expr_value, R1 có right_expr_value

        // Thực hiện phép toán
        if (bin_op_node.op == TokenType::PLUS) {
            pushGadget(GadgetFunction::ADD_R0_R1_RET); // R0 = R0 + R1
        } else if (bin_op_node.op == TokenType::MINUS) {
            pushGadget(GadgetFunction::SUB_R0_R1_RET); // R0 = R0 - R1
        }
        return 0; // Kết quả vẫn trong R0
    } else if (expr_node.type == ASTNode::NodeType::MemRead) {
        const auto& mem_read_node = static_cast<const MemReadNode&>(expr_node);
        unsigned int addr_reg = evaluateExpressionIntoRegister(*mem_read_node.address_expr); // Đặt địa chỉ vào R0
        // Giả định địa chỉ luôn nằm trong R0 sau evaluateExpressionIntoRegister
        pushGadget(GadgetFunction::POP_R1_RET); // Load địa chỉ vào R1
        pushData(evaluateExpressionIntoRegister(*mem_read_node.address_expr));
        pushGadget(GadgetFunction::LOAD_R0_FROM_R1_RET); // Load giá trị từ [R1] vào R0
        return 0;
    }
    throw std::runtime_error("Lỗi: Không thể đánh giá biểu thức loại này vào thanh ghi: " + std::to_string(static_cast<int>(expr_node.type)));
}


void ROPGenerator::generateForAssignment(const AssignmentNode& node) {
    SymbolInfo* sym = symbol_table.get_symbol(node.var_name);
    if (!sym) throw std::runtime_error("Lỗi: Biến '" + node.var_name + "' chưa khai báo.");

    // 1. Đánh giá biểu thức bên phải và đưa kết quả vào R0
    evaluateExpressionIntoRegister(*node.expression); // Kết quả sẽ nằm trong R0

    // 2. Đặt địa chỉ của biến đích vào R1
    pushGadget(GadgetFunction::POP_R1_RET);
    pushData(sym->address);

    // 3. Store giá trị từ R0 vào địa chỉ trong R1
    pushGadget(GadgetFunction::STORE_R0_TO_R1_RET);
}

void ROPGenerator::generateForMemWrite(const MemWriteNode& node) {
    // 1. Đánh giá giá trị cần ghi vào R0
    evaluateExpressionIntoRegister(*node.value_expr); // Giá trị sẽ nằm trong R0

    // 2. Đánh giá địa chỉ đích vào R1
    pushGadget(GadgetFunction::POP_R1_RET);
    pushData(evaluateExpressionIntoRegister(*node.address_expr)); // Địa chỉ sẽ nằm trong R1
    // (Lưu ý: cần cẩn thận để evaluateExpressionIntoRegister không ghi đè R0 sau khi R0 đã có value)
    // Cách tốt hơn: dùng một thanh ghi tạm khác hoặc nhiều gadget pop
    // Ví dụ:
    // pushGadget(POP_R0_RET); // value
    // pushData(evaluated_value);
    // pushGadget(POP_R1_RET); // address
    // pushData(evaluated_address);
    // pushGadget(STORE_R0_TO_R1_RET);

    // 3. Thực hiện ghi
    pushGadget(GadgetFunction::STORE_R0_TO_R1_RET);
}

void ROPGenerator::generateForPrintChar(const PrintCharNode& node) {
    // 1. Đánh giá line, column, char_code
    // Các giá trị này cần được đẩy vào các thanh ghi hoặc địa chỉ mà gadget SYSCALL_PRINT_CHAR mong đợi.
    // Với VRAM Casio, ta cần tính địa chỉ: base_line_addr + column_offset
    unsigned int line_val = evaluateExpressionIntoRegister(*node.line_expr); // Giả định line_val trong R0
    unsigned int col_val = evaluateExpressionIntoRegister(*node.column_expr); // Giả định col_val trong R0 (sau khi đã dùng line)
    unsigned int char_val = evaluateExpressionIntoRegister(*node.char_code_expr); // Giả định char_val trong R0 (sau khi đã dùng col)

    // Ví dụ:
    // Tính toán địa chỉ VRAM đích
    // Dòng 1: 0x01, Dòng 2: 0x11, Dòng 3: 0x21, Dòng 4: 0x31
    // Offset cột: 1 ký tự = 1 byte
    // VRAM_Addr = (line - 1) * 0x10 + 0x01 + col

    // Để đơn giản ví dụ này, giả sử có một gadget phức tạp hơn hoặc
    // ta phải tự tính toán địa chỉ rồi dùng MEM_WRITE:
    // Với FxLaux, nếu ta có: PRINT_CHAR(line, col, char_code);
    // Compiler sẽ dịch nó thành: MEM_WRITE((line-1)*0x10 + 0x01 + col, char_code);

    // Để làm vậy, chúng ta sẽ cần gadget để thực hiện các phép toán trên.
    // Ví dụ này chỉ giả định là tính toán các giá trị, sau đó dùng gadget ghi.

    // Bước 1: Tính VRAM_address = (line_val - 1) * 0x10 + 0x01 + col_val
    // Đây là một biểu thức số học phức tạp, cần nhiều gadget.
    // Để đơn giản, giả sử chúng ta đã có một hàm tính toán này:
    unsigned int base_addr;
    if (line_val == 1) base_addr = 0x01;
    else if (line_val == 2) base_addr = 0x11;
    else if (line_val == 3) base_addr = 0x21;
    else if (line_val == 4) base_addr = 0x31;
    else throw std::runtime_error("Lỗi: Dòng màn hình không hợp lệ: " + std::to_string(line_val));

    unsigned int vram_addr = base_addr + col_val;

    // Bước 2: Chuỗi ROP để ghi char_val vào vram_addr
    pushGadget(GadgetFunction::POP_R0_RET); // Đẩy char_val vào R0
    pushData(char_val);
    pushGadget(GadgetFunction::POP_R1_RET); // Đẩy vram_addr vào R1
    pushData(vram_addr);
    pushGadget(GadgetFunction::STORE_R0_TO_R1_RET); // Ghi R0 vào địa chỉ R1

    // Hoặc nếu bạn có một SYSCALL_PRINT_CHAR đặc biệt:
    // pushGadget(GadgetFunction::POP_R0_RET); // Line
    // pushData(line_val);
    // pushGadget(GadgetFunction::POP_R1_RET); // Column
    // pushData(col_val);
    // pushGadget(GadgetFunction::POP_R2_RET); // Char Code
    // pushData(char_val);
    // pushGadget(GadgetFunction::SYSCALL_PRINT_CHAR); // Gọi gadget syscall
}
