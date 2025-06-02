#include "ROPGenerator.h"
#include <iostream>
#include <fstream>   // For std::ifstream
#include <sstream>   // For std::stringstream
#include <stdexcept> // For std::runtime_error
#include <iomanip>   // For std::hex, std::dec

// --- GadgetDB Implementation ---

// Constructor để khởi tạo name_to_enum_map
GadgetDB::GadgetDB() {
    // Ánh xạ chuỗi tên gadget trong file nx_u8_gadgets.txt sang enum GadgetFunction
    // Đảm bảo rằng mỗi chuỗi ở đây khớp chính xác với chuỗi sau địa chỉ trong file txt của bạn.

    // Basic stack/register manipulation
    name_to_enum_map["setlr"] = GadgetFunction::SETLR;
    name_to_enum_map["DI,RT"] = GadgetFunction::DI_RT;
    name_to_enum_map["sp = er14,pop er14,rt"] = GadgetFunction::SP_ER14_POP_ER14_RT;
    name_to_enum_map["sp = er14,pop qr8"] = GadgetFunction::SP_ER14_POP_QR8;
    name_to_enum_map["sp = er14,pop qr8,pop qr0"] = GadgetFunction::SP_ER14_POP_QR8_POP_QR0;
    name_to_enum_map["sp = er14,pop er14"] = GadgetFunction::SP_ER14_POP_ER14;
    name_to_enum_map["sp = er6,pop er8"] = GadgetFunction::SP_ER6_POP_ER8;
    name_to_enum_map["sp = er14,pop xr12"] = GadgetFunction::SP_ER14_POP_XR12;
    name_to_enum_map["sp = er14,pop qr8,pop er6"] = GadgetFunction::SP_ER14_POP_QR8_POP_ER6;
    name_to_enum_map["er14 = sp,rt"] = GadgetFunction::ER14_SP_RT;
    name_to_enum_map["nop"] = GadgetFunction::NOP;

    // POP gadgets
    name_to_enum_map["pop ea"] = GadgetFunction::POP_EA;
    name_to_enum_map["pop er14,rt"] = GadgetFunction::POP_ER14_RT;
    name_to_enum_map["pop er0,rt"] = GadgetFunction::POP_ER0_RT;
    name_to_enum_map["pop er2"] = GadgetFunction::POP_ER2;
    name_to_enum_map["pop er4"] = GadgetFunction::POP_ER4;
    name_to_enum_map["pop er8"] = GadgetFunction::POP_ER8;
    name_to_enum_map["pop er12,rt"] = GadgetFunction::POP_ER12_RT;
    name_to_enum_map["pop qr0"] = GadgetFunction::POP_QR0;
    name_to_enum_map["pop qr8"] = GadgetFunction::POP_QR8;
    name_to_enum_map["pop r0"] = GadgetFunction::POP_R0;
    name_to_enum_map["pop r8"] = GadgetFunction::POP_R8;
    name_to_enum_map["pop xr0"] = GadgetFunction::POP_XR0;
    name_to_enum_map["pop xr4"] = GadgetFunction::POP_XR4;
    name_to_enum_map["pop xr8"] = GadgetFunction::POP_XR8;
    name_to_enum_map["pop er10"] = GadgetFunction::POP_ER10;
    name_to_enum_map["pop r12"] = GadgetFunction::POP_R12;
    name_to_enum_map["pop er0"] = GadgetFunction::POP_ER0;
    name_to_enum_map["pop er12"] = GadgetFunction::POP_ER12;
    name_to_enum_map["pop er14"] = GadgetFunction::POP_ER14;
    name_to_enum_map["pop er6"] = GadgetFunction::POP_ER6;
    name_to_enum_map["pop er6,rt"] = GadgetFunction::POP_ER6_RT;
    name_to_enum_map["pop xr12"] = GadgetFunction::POP_XR12;
    name_to_enum_map["pop er4,rt"] = GadgetFunction::POP_ER4_RT;
    name_to_enum_map["pop er8,rt"] = GadgetFunction::POP_ER8_RT;
    name_to_enum_map["pop qr0,rt"] = GadgetFunction::POP_QR0_RT;
    name_to_enum_map["pop qr8,rt"] = GadgetFunction::POP_QR8_RT;
    name_to_enum_map["pop r4"] = GadgetFunction::POP_R4;
    name_to_enum_map["pop r4,rt"] = GadgetFunction::POP_R4_RT;
    name_to_enum_map["pop r9"] = GadgetFunction::POP_R9;
    name_to_enum_map["pop xr12,rt"] = GadgetFunction::POP_XR12_RT;
    name_to_enum_map["pop xr4,rt"] = GadgetFunction::POP_XR4_RT;
    name_to_enum_map["pop xr8,rt"] = GadgetFunction::POP_XR8_RT;

    // ADD gadgets
    name_to_enum_map["er0+=er4,rt"] = GadgetFunction::ADD_ER0_ER4_RET;
    name_to_enum_map["er4+=er0,r8 = r8,rt"] = GadgetFunction::ADD_ER4_ER0_R8_RET;
    name_to_enum_map["er0+=er8,rt"] = GadgetFunction::ADD_ER0_ER8_RET;
    name_to_enum_map["er2+=er8,rt"] = GadgetFunction::ADD_ER2_ER8_RET;
    name_to_enum_map["er0+=er2,rt"] = GadgetFunction::ADD_ER0_ER2_RET;
    name_to_enum_map["er0+=1,rt"] = GadgetFunction::ADD_ER0_ONE_RET;
    name_to_enum_map["r0+=1,rt"] = GadgetFunction::ADD_R0_ONE_RET;

    // Move/Copy registers
    name_to_enum_map["er6 = er0,er0 = er8,pop qr8"] = GadgetFunction::MOV_ER6_ER0_ER0_ER8_POP_QR8;
    name_to_enum_map["er8 = er0"] = GadgetFunction::MOV_ER8_ER0_RET;
    name_to_enum_map["er2 = er0,er0 = er2,pop er8,rt"] = GadgetFunction::MOV_ER2_ER0_ER0_ER2_POP_ER8_RET;
    name_to_enum_map["er2 = er0,er0+=er4,rt"] = GadgetFunction::MOV_ER2_ER0_ADD_ER0_ER4_RET;
    name_to_enum_map["er0 = er2,rt"] = GadgetFunction::MOV_ER0_ER2_RET;
    name_to_enum_map["er0 = er4,pop er4"] = GadgetFunction::MOV_ER0_ER4_POP_ER4;
    name_to_enum_map["er0 = er8,pop er8,rt"] = GadgetFunction::MOV_ER0_ER8_POP_ER8_RET;
    name_to_enum_map["er0 = er8"] = GadgetFunction::MOV_ER0_ER8_RET; // No ,rt implies it's not a return gadget
    name_to_enum_map["er0 = er6,pop er8,pop xr4"] = GadgetFunction::MOV_ER0_ER6_POP_ER8_POP_XR4;
    name_to_enum_map["er2 = er0,r0 = r4,r1 = 0,pop xr4,rt"] = GadgetFunction::MOV_ER2_ER0_R0_R4_R1_ZERO_POP_XR4_RET;
    name_to_enum_map["r0 = r5,pop er4"] = GadgetFunction::MOV_R0_R5_POP_ER4;
    name_to_enum_map["r0 = r2 = 0"] = GadgetFunction::MOV_R0_R2_ZERO;
    name_to_enum_map["r0 = r2"] = GadgetFunction::MOV_R0_R2;
    name_to_enum_map["r2 = r0,pop er0"] = GadgetFunction::MOV_R2_R0_POP_ER0;
    name_to_enum_map["r2 = r0,pop r6,pop er12"] = GadgetFunction::MOV_R2_R0_POP_R6_POP_ER12;
    name_to_enum_map["r2 = 0,r7 = 4"] = GadgetFunction::MOV_R2_ZERO_R7_FOUR;
    name_to_enum_map["r0 = 0"] = GadgetFunction::MOV_R0_ZERO;
    name_to_enum_map["r0 = 0,rt"] = GadgetFunction::MOV_R0_ZERO_RET;
    name_to_enum_map["r0 = 1,rt"] = GadgetFunction::MOV_R0_ONE_RET;
    name_to_enum_map["r0 = 0,pop er2"] = GadgetFunction::MOV_R0_ZERO_POP_ER2;
    name_to_enum_map["r1 = 0,rt"] = GadgetFunction::MOV_R1_ZERO_RET;
    name_to_enum_map["r5 = 0,rt"] = GadgetFunction::MOV_R5_ZERO_RET;
    name_to_enum_map["er14 = er0,pop xr0"] = GadgetFunction::MOV_ER14_ER0_POP_XR0;
    name_to_enum_map["er0 = er12,pop er12,rt"] = GadgetFunction::MOV_ER0_ER12_POP_ER12_RET;
    name_to_enum_map["er10 = er2,rt"] = GadgetFunction::MOV_ER10_ER2_RET;
    name_to_enum_map["er0 = er10,pop xr8"] = GadgetFunction::MOV_ER0_ER10_POP_XR8;
    name_to_enum_map["er0 = 1,rt"] = GadgetFunction::MOV_ER0_ONE_RET;
    name_to_enum_map["er2 = 0,er4 = 0,er6 = 0,er8 = 1,rt"] = GadgetFunction::MOV_ER2_ZERO_ER4_ZERO_ER6_ZERO_ER8_ONE_RET;
    name_to_enum_map["er2 = 0,r0 = 2,[er8]=er2,pop xr8"] = GadgetFunction::MOV_ER2_ZERO_R0_TWO_STORE_ER8_ER2_POP_XR8;
    name_to_enum_map["er2 = 1,r0 = r2,rt"] = GadgetFunction::MOV_ER2_ONE_R0_ER2_RET;
    name_to_enum_map["r0 = 0,[er8]+=er2,pop xr8"] = GadgetFunction::MOV_R0_ZERO_STORE_ER8_ER2_POP_XR8;
    name_to_enum_map["r2 = 1,r0 = r2,pop er4,pop er8,rt"] = GadgetFunction::MOV_R2_ONE_R0_R2_POP_ER4_POP_ER8_RET;
    name_to_enum_map["r0 = r1,rt"] = GadgetFunction::MOV_R0_R1_RET;


    // Store (ST) gadgets
    name_to_enum_map["[er2]=er0,r2 = 0,pop er4,rt"] = GadgetFunction::STORE_ER2_ER0_R2_ZERO_POP_ER4_RET;
    name_to_enum_map["[er0]=er2,rt"] = GadgetFunction::STORE_ER0_ER2_RET;
    name_to_enum_map["[er0]=r2,rt"] = GadgetFunction::STORE_ER0_R2_RET;
    name_to_enum_map["[er0]=r2"] = GadgetFunction::STORE_ER0_R2;
    name_to_enum_map["[er2]=r0,r2 = 0"] = GadgetFunction::STORE_ER2_R0_R2_ZERO;
    name_to_enum_map["[er8]=er2,pop xr8"] = GadgetFunction::STORE_ER8_ER2_POP_XR8;
    name_to_enum_map["[er4]=er0,pop er0,rt"] = GadgetFunction::STORE_ER4_ER0_POP_ER0_RET;
    name_to_enum_map["[ea]=qr0"] = GadgetFunction::STORE_EA_QR0;
    name_to_enum_map["[er12]=er14,pop xr4,pop qr8"] = GadgetFunction::STORE_ER12_ER14_POP_XR4_POP_QR8;

    // Load (L) gadgets
    name_to_enum_map["er4=[er8],pop er8,rt"] = GadgetFunction::LOAD_ER4_FROM_ER8_POP_ER8_RET;
    name_to_enum_map["er0=[er2],r2 = 9,rt"] = GadgetFunction::LOAD_ER0_FROM_ER2_R2_NINE_RET;
    name_to_enum_map["er8=[er0],rt"] = GadgetFunction::LOAD_ER8_FROM_ER0_RET;
    name_to_enum_map["r0=[er2]"] = GadgetFunction::LOAD_R0_FROM_ER2;
    name_to_enum_map["r0=[er0]"] = GadgetFunction::LOAD_R0_FROM_ER0;
    name_to_enum_map["er0=[er0],pop xr8,rt"] = GadgetFunction::LOAD_ER0_FROM_ER0_POP_XR8_RET;
    name_to_enum_map["r0=[ea],rt"] = GadgetFunction::LOAD_R0_FROM_EA_RET;
    name_to_enum_map["sp=[er8],pop er8"] = GadgetFunction::LOAD_SP_FROM_ER8_POP_ER8;
    name_to_enum_map["qr0=[ea],lea D002H,[ea]=qr0"] = GadgetFunction::LOAD_QR0_FROM_EA_LEA_D002H_EA_QR0;

    // Subtract (SUB) gadgets
    name_to_enum_map["er0-=er2,rt"] = GadgetFunction::SUB_ER0_ER2_RET;
    name_to_enum_map["er0-=er12,pop er8,pop er12,rt"] = GadgetFunction::SUB_ER0_ER12_POP_ER8_POP_ER12_RET;
    name_to_enum_map["r0-=1,rt"] = GadgetFunction::SUB_R0_ONE_RET;
    name_to_enum_map["r0-=r8,pop er8,rt"] = GadgetFunction::SUB_R0_R8_POP_ER8_RET;

    // OR gadgets
    name_to_enum_map["or r0,r1"] = GadgetFunction::OR_R0_R1;
    name_to_enum_map["or qr0,qr8"] = GadgetFunction::OR_QR0_QR8;

    // Shift gadgets
    name_to_enum_map["r0 >> 4,rt"] = GadgetFunction::SRL_R0_4_RET;
    name_to_enum_map["qr0 >> 4,rt"] = GadgetFunction::SRL_QR0_4_RET;
    name_to_enum_map["r0 << 4,rt"] = GadgetFunction::SLL_R0_4_RET;
    name_to_enum_map["r1 << 4,rt"] = GadgetFunction::SLL_R1_4_RET;
    name_to_enum_map["er0 << 4,rt"] = GadgetFunction::SLL_ER0_4_RET;
    name_to_enum_map["xr0 << 4,rt"] = GadgetFunction::SLL_XR0_4_RET;
    name_to_enum_map["qr0 << 4,rt"] = GadgetFunction::SLL_QR0_4_RET;

    // Compare (CMP) gadgets
    name_to_enum_map["er0 - er2_gt,r0 = 0 |r0 = 1,rt"] = GadgetFunction::CMP_ER0_ER2_GT_R0_ZERO_OR_ONE_RET;
    name_to_enum_map["er0 - er2_eq,r0 = 1,rt"] = GadgetFunction::CMP_ER0_ER2_EQ_R0_ONE_RET;
    name_to_enum_map["er2 - er0_gt,r0 = 0 |r0 = 1,rt"] = GadgetFunction::CMP_ER2_ER0_GT_R0_ZERO_OR_ONE_RET;
    name_to_enum_map["er0 - er2_le,er0 = er2,rt"] = GadgetFunction::CMP_ER0_ER2_LE_ER0_ER2_RET;
    name_to_enum_map["er8 - er0_lt,pop xr8"] = GadgetFunction::CMP_ER8_ER0_LT_POP_XR8;
    name_to_enum_map["r0 - 0_lt,rt"] = GadgetFunction::CMP_R0_ZERO_LT_RET;
    name_to_enum_map["r1 - 0_lt,rt"] = GadgetFunction::CMP_R1_ZERO_LT_RET;

    // Multiply (MUL) gadgets
    name_to_enum_map["er0*=r2,er2 = er0,er0+=er4,rt"] = GadgetFunction::MUL_ER0_R2_ER2_ER0_ADD_ER0_ER4_RET;
    name_to_enum_map["er0*=r2,er0+=er6,er10 = er0,rt"] = GadgetFunction::MUL_ER0_R2_ADD_ER0_ER6_ER10_ER0_RET;

    // Divide (DIV) gadgets
    name_to_enum_map["er0/=r2,rt"] = GadgetFunction::DIV_ER0_R2_RET;

    // BRK gadget
    name_to_enum_map["break"] = GadgetFunction::BRK;

    // Other gadgets
    name_to_enum_map["[ea]+=1,r0=3"] = GadgetFunction::INC_EA_R0_THREE;
    name_to_enum_map["[ea]-=1,pop xr4"] = GadgetFunction::DEC_EA_POP_XR4;
    name_to_enum_map["B LEAVE"] = GadgetFunction::B_LEAVE;
    name_to_enum_map["calc_checksum_set_f004"] = GadgetFunction::CALC_CHECKSUM_SET_F004;
    name_to_enum_map["calc_checksum_no_set_f004"] = GadgetFunction::CALC_CHECKSUM_NO_SET_F004;
    name_to_enum_map["calc_checksum_0"] = GadgetFunction::CALC_CHECKSUM_0;
    name_to_enum_map["calc_checksum_1"] = GadgetFunction::CALC_CHECKSUM_1;
    name_to_enum_map["calc_checksum_2"] = GadgetFunction::CALC_CHECKSUM_2;
    name_to_enum_map["calc_checksum_3"] = GadgetFunction::CALC_CHECKSUM_3;
    name_to_enum_map["pr_checksum"] = GadgetFunction::PR_CHECKSUM;
    name_to_enum_map["[er8]+=er2,pop xr8"] = GadgetFunction::ADD_ER8_ER2_POP_XR8;


    // BL gadgets (Function calls)
    name_to_enum_map["BL memcpy,pop er0"] = GadgetFunction::BL_MEMCPY_POP_ER0;
    name_to_enum_map["BL strcpy"] = GadgetFunction::BL_STRCPY;
    name_to_enum_map["BL strcat"] = GadgetFunction::BL_STRCAT;
    name_to_enum_map["BL memset,pop er2"] = GadgetFunction::BL_MEMSET_POP_ER2;
    name_to_enum_map["BL delay,pop xr0"] = GadgetFunction::BL_DELAY_POP_XR0;
    name_to_enum_map["BL line_print"] = GadgetFunction::BL_LINE_PRINT;
    name_to_enum_map["BL printline"] = GadgetFunction::BL_PRINTLINE;
    name_to_enum_map["BL hex_byte,er6 = er0,er0 = er8,pop qr8"] = GadgetFunction::BL_HEX_BYTE_ER6_ER0_ER0_ER8_POP_QR8;
    name_to_enum_map["BL smart_strcpy,pop er8"] = GadgetFunction::BL_SMART_STRCPY_POP_ER8;
    name_to_enum_map["BL zero_KO"] = GadgetFunction::BL_ZERO_KO;
    name_to_enum_map["BL line_draw"] = GadgetFunction::BL_LINE_DRAW;
    name_to_enum_map["BL render.ddd4"] = GadgetFunction::BL_RENDER_DDD4;
}


void GadgetDB::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Không thể mở file gadget: " + filepath);
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string addr_str;
        std::string func_str_raw;

        // Read address (hex string)
        ss >> addr_str;
        if (addr_str.empty()) continue; // Should not happen after empty line check, but good for robustness

        // Read the rest of the line as the function string
        std::getline(ss, func_str_raw);
        // Remove leading whitespace from func_str_raw
        size_t first_char = func_str_raw.find_first_not_of(" \t");
        if (std::string::npos != first_char) {
            func_str_raw = func_str_raw.substr(first_char);
        }

        unsigned int addr = std::stoul(addr_str, nullptr, 16);

        // Find in the map that converts string names to enum
        auto it = name_to_enum_map.find(func_str_raw);
        if (it != name_to_enum_map.end()) {
            gadget_address_map[it->second] = addr; // Assign the address to the corresponding enum
        } else {
            // It's good to keep this warning during development to catch unmapped gadgets.
            // You can comment it out once all gadgets are mapped.
            std::cerr << "Cảnh báo: Chức năng gadget không xác định trong file hoặc chưa được ánh xạ trong GadgetDB constructor: '"
                      << func_str_raw << "' (Địa chỉ: 0x" << std::hex << addr << std::dec << ")" << std::endl;
        }
    }
    std::cout << "Đã tải " << gadget_address_map.size() << " gadget." << std::endl;
}

unsigned int GadgetDB::getAddress(GadgetFunction func) const {
    auto it = gadget_address_map.find(func);
    if (it != gadget_address_map.end()) {
        return it->second;
    }
    // Convert enum to string for better error message if possible, or just print int value
    throw std::runtime_error("Lỗi: Không tìm thấy địa chỉ cho gadget chức năng: " + std::to_string(static_cast<int>(func)));
}

// --- ROPGenerator Implementation ---
ROPGenerator::ROPGenerator(const GadgetDB& db, const SymbolTable& sym_table)
    : gadget_db(db), symbol_table(sym_table) {}

std::vector<unsigned int> ROPGenerator::generateROPChain(const ProgramNode& program_node) {
    rop_chain.clear(); // Clear previous chain

    for (const auto& statement : program_node.statements) {
        generateForNode(*statement);
    }

    // End the ROP chain with a breakpoint (BRK) for easier debugging
    pushGadget(GadgetFunction::BRK);

    return rop_chain;
}

void ROPGenerator::generateForNode(const ASTNode& node) {
    switch (node.type) {
        case ASTNode::NodeType::VarDeclaration:
            generateForVarDeclaration(static_cast<const VarDeclarationNode&>(node));
            break;
        case ASTNode::NodeType::Assignment:
            generateForAssignment(static_cast<const AssignmentNode&>(node));
            break;
        case ASTNode::NodeType::MemWrite:
            generateForMemWrite(static_cast<const MemWriteNode&>(node));
            break;
        case ASTNode::NodeType::PrintChar:
            generateForPrintChar(static_cast<const PrintCharNode&>(node));
            break;
        // Add more cases for other statement types as you implement them
        default:
            throw std::runtime_error("Lỗi: Loại ASTNode không được hỗ trợ trong ROP generation.");
    }
}

void ROPGenerator::generateForVarDeclaration(const VarDeclarationNode& node) {
    // Variable declarations in FxLaux primarily update the symbol table.
    // No direct ROP gadgets are generated for declaration itself.
    std::cout << "DEBUG: Xử lý khai báo biến: " << node.var_name << std::endl;
}

void ROPGenerator::generateForAssignment(const AssignmentNode& node) {
    const SymbolInfo* sym = symbol_table.get_symbol(node.var_name);
    if (!sym) {
        // This check should ideally be done in semantic analysis phase (Parser),
        // but keeping it here for robustness during ROP generation.
        throw std::runtime_error("Lỗi: Biến '" + node.var_name + "' chưa khai báo.");
    }

    // 1. Evaluate the right-hand side expression into ER0.
    // After this call, ER0 will contain the value to be assigned.
    evaluateExpressionIntoR0(*node.expression);

    // 2. Prepare the destination address for the variable.
    // We need to load the variable's address into a register, say ER1.
    pushGadget(GadgetFunction::POP_ER1); // Pop the address into ER1
    pushData(sym->address);

    // 3. Store the value from ER0 into the address in ER1.
    // We need a gadget that effectively does `[ER1] = ER0`.
    // Looking at your gadgets: `STORE_ER0_ER2_RET` is `[er0]=er2,rt`.
    // This means we need ER0 to hold the destination address and ER2 to hold the value.
    // Current state: ER0 (value), ER1 (address).
    // So, we need to swap/move them.
    pushGadget(GadgetFunction::MOV_ER2_ER0_RET); // ER2 = ER0 (Move value to ER2)
    pushGadget(GadgetFunction::MOV_ER0_ER1_RET); // ER0 = ER1 (Move address to ER0)
    pushGadget(GadgetFunction::STORE_ER0_ER2_RET); // Now: [ER0 (address)] = ER2 (value)

    std::cout << "DEBUG: Sinh mã gán: " << node.var_name << " = expr (địa chỉ 0x"
              << std::hex << sym->address << std::dec << ")" << std::endl;
}

void ROPGenerator::generateForMemWrite(const MemWriteNode& node) {
    // 1. Evaluate the value to be written into ER0.
    evaluateExpressionIntoR0(*node.value_expr); // Value will be in ER0
    pushGadget(GadgetFunction::MOV_ER2_ER0_RET); // Move value to ER2 (temporary storage)

    // 2. Evaluate the destination address into ER0.
    evaluateExpressionIntoR0(*node.address_expr); // Address will be in ER0

    // Current state: ER0 (address), ER2 (value to write).
    // Our target gadget `STORE_ER0_ER2_RET` (`[er0]=er2,rt`) directly supports this.
    pushGadget(GadgetFunction::STORE_ER0_ER2_RET); // Store ER2 (value) at [ER0 (address)]

    std::cout << "DEBUG: Sinh mã ghi bộ nhớ: [expr_addr] = expr_val" << std::endl;
}

void ROPGenerator::generateForPrintChar(const PrintCharNode& node) {
    // Casio fx-9860G VRAM mapping (example assumptions):
    // Display starts at 0xD0000.
    // Each character is 1 byte.
    // Each line is 0x10 bytes long (16 characters wide).
    // VRAM_Addr = BASE_VRAM_ADDR + (line - 1) * 0x10 + column_offset

    // 1. Calculate the character code into ER0.
    evaluateExpressionIntoR0(*node.char_code_expr); // char_code into ER0
    pushGadget(GadgetFunction::MOV_ER2_ER0_RET); // Move char_code to ER2 (temporary)

    // 2. Calculate VRAM address into ER0.
    // Part a: Calculate `(line - 1)` into ER0.
    evaluateExpressionIntoR0(*node.line_expr); // 'line' value into ER0
    pushGadget(GadgetFunction::POP_ER4);      // Pop 1 into ER4
    pushData(1);
    pushGadget(GadgetFunction::SUB_ER0_ER4_RET); // ER0 = ER0 - ER4 (line - 1)

    // Part b: Multiply `(line - 1)` by 0x10 (i.e., left shift by 4 bits) into ER0.
    pushGadget(GadgetFunction::SLL_ER0_4_RET); // ER0 = ER0 << 4

    // Part c: Add 'column' value to ER0.
    // Need to save current ER0 (line_offset) temporarily, calculate column, then add.
    pushGadget(GadgetFunction::MOV_ER8_ER0_RET); // Save (line_offset) to ER8

    evaluateExpressionIntoR0(*node.column_expr); // 'column' value into ER0

    // Add saved line_offset (in ER8) to current ER0 (column)
    pushGadget(GadgetFunction::MOV_ER4_ER0_RET); // Move column to ER4
    pushGadget(GadgetFunction::MOV_ER0_ER8_RET); // Move line_offset from ER8 to ER0
    pushGadget(GadgetFunction::ADD_ER0_ER4_RET); // ER0 = ER0 + ER4 (now ER0 has line_offset + column)

    // 
