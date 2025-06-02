#ifndef ROP_GENERATOR_H
#define ROP_GENERATOR_H

#include "Parser.h" // Cần các định nghĩa AST và SymbolTable
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <stdexcept> // For std::runtime_error

// --- Định nghĩa các loại Gadget ---
// Liệt kê chi tiết tất cả các chức năng của gadget mà bạn đã cung cấp
enum class GadgetFunction {
    UNKNOWN_GADGET,

    // Stack/Register manipulation (pop)
    SETLR,
    DI_RT,
    SP_ER14_POP_ER14_RT,
    SP_ER14_POP_QR8,
    SP_ER14_POP_QR8_POP_QR0,
    SP_ER14_POP_ER14,
    SP_ER6_POP_ER8,
    SP_ER14_POP_XR12,
    SP_ER14_POP_QR8_POP_ER6,
    ER14_SP_RT,
    NOP,
    POP_EA,
    POP_ER14_RT,
    POP_ER0_RT,
    POP_ER2,
    POP_ER4,
    POP_ER8,
    POP_ER12_RT,
    POP_QR0,
    POP_QR8,
    POP_R0,
    POP_R8,
    POP_XR0,
    POP_XR4,
    POP_XR8,
    POP_ER10,
    POP_R12,
    POP_ER0,
    POP_ER12,
    POP_ER14,
    POP_ER6,
    POP_ER6_RT,
    POP_XR12,
    POP_ER4_RT,
    POP_ER8_RT,
    POP_QR0_RT,
    POP_QR8_RT,
    POP_R4,
    POP_R4_RT,
    POP_R9,
    POP_XR12_RT,
    POP_XR4_RT,
    POP_XR8_RT,

    // ADD gadgets
    ADD_ER0_ER4_RET,
    ADD_ER4_ER0_R8_RET,
    ADD_ER0_ER8_RET,
    ADD_ER2_ER8_RET,
    ADD_ER0_ER2_RET,
    ADD_ER0_ONE_RET,
    ADD_R0_ONE_RET,

    // Move/Copy registers
    MOV_ER6_ER0_ER0_ER8_POP_QR8,
    MOV_ER8_ER0_RET,
    MOV_ER2_ER0_ER0_ER2_POP_ER8_RET,
    MOV_ER2_ER0_ADD_ER0_ER4_RET,
    MOV_ER0_ER2_RET,
    MOV_ER0_ER4_POP_ER4,
    MOV_ER0_ER8_POP_ER8_RET,
    MOV_ER0_ER8_RET,
    MOV_ER0_ER6_POP_ER8_POP_XR4,
    MOV_ER2_ER0_R0_R4_R1_ZERO_POP_XR4_RET,
    MOV_R0_R5_POP_ER4,
    MOV_R0_R2_ZERO,
    MOV_R0_R2,
    MOV_R2_R0_POP_ER0,
    MOV_R2_R0_POP_R6_POP_ER12,
    MOV_R2_ZERO_R7_FOUR,
    MOV_R0_ZERO,
    MOV_R0_ZERO_RET,
    MOV_R0_ONE_RET,
    MOV_R0_ZERO_POP_ER2,
    MOV_R1_ZERO_RET,
    MOV_R5_ZERO_RET,
    MOV_ER14_ER0_POP_XR0,
    MOV_ER0_ER12_POP_ER12_RET,
    MOV_ER10_ER2_RET,
    MOV_ER0_ER10_POP_XR8,
    MOV_ER0_ONE_RET,
    MOV_ER2_ZERO_ER4_ZERO_ER6_ZERO_ER8_ONE_RET,
    MOV_ER2_ZERO_R0_TWO_STORE_ER8_ER2_POP_XR8,
    MOV_ER2_ONE_R0_ER2_RET,
    MOV_R0_ZERO_STORE_ER8_ER2_POP_XR8,
    MOV_R2_ONE_R0_R2_POP_ER4_POP_ER8_RET,
    MOV_R0_R1_RET,

    // Store (ST) gadgets
    STORE_ER2_ER0_R2_ZERO_POP_ER4_RET,
    STORE_ER0_ER2_RET,
    STORE_ER0_R2_RET,
    STORE_ER0_R2,
    STORE_ER2_R0_R2_ZERO,
    STORE_ER8_ER2_POP_XR8,
    STORE_ER4_ER0_POP_ER0_RET,
    STORE_EA_QR0,
    STORE_ER12_ER14_POP_XR4_POP_QR8,

    // Load (L) gadgets
    LOAD_ER4_FROM_ER8_POP_ER8_RET,
    LOAD_ER0_FROM_ER2_R2_NINE_RET,
    LOAD_ER8_FROM_ER0_RET,
    LOAD_R0_FROM_ER2,
    LOAD_R0_FROM_ER0,
    LOAD_ER0_FROM_ER0_POP_XR8_RET,
    LOAD_R0_FROM_EA_RET,
    LOAD_SP_FROM_ER8_POP_ER8,
    LOAD_QR0_FROM_EA_LEA_D002H_EA_QR0,

    // Subtract (SUB) gadgets
    SUB_ER0_ER2_RET,
    SUB_ER0_ER12_POP_ER8_POP_ER12_RET,
    SUB_R0_ONE_RET,
    SUB_R0_R8_POP_ER8_RET,

    // OR gadgets
    OR_R0_R1,
    OR_QR0_QR8,

    // Shift gadgets
    SRL_R0_4_RET,
    SRL_QR0_4_RET,
    SLL_R0_4_RET,
    SLL_R1_4_RET,
    SLL_ER0_4_RET,
    SLL_XR0_4_RET,
    SLL_QR0_4_RET,

    // Compare (CMP) gadgets
    CMP_ER0_ER2_GT_R0_ZERO_OR_ONE_RET,
    CMP_ER0_ER2_EQ_R0_ONE_RET,
    CMP_ER2_ER0_GT_R0_ZERO_OR_ONE_RET,
    CMP_ER0_ER2_LE_ER0_ER2_RET,
    CMP_ER8_ER0_LT_POP_XR8,
    CMP_R0_ZERO_LT_RET,
    CMP_R1_ZERO_LT_RET,

    // Multiply (MUL) gadgets
    MUL_ER0_R2_ER2_ER0_ADD_ER0_ER4_RET,
    MUL_ER0_R2_ADD_ER0_ER6_ER10_ER0_RET,

    // Divide (DIV) gadgets
    DIV_ER0_R2_RET,

    // Control Flow (Jumps/Calls)
    BRK,
    BL_MEMCPY_POP_ER0,
    BL_STRCPY,
    BL_STRCAT,
    BL_MEMSET_POP_ER2,
    BL_DELAY_POP_XR0,
    BL_LINE_PRINT,
    BL_PRINTLINE,
    BL_HEX_BYTE_ER6_ER0_ER0_ER8_POP_QR8,
    BL_SMART_STRCPY_POP_ER8,
    BL_ZERO_KO,
    BL_LINE_DRAW,
    BL_RENDER_DDD4,

    // Other gadgets
    INC_EA_R0_THREE,
    DEC_EA_POP_XR4,
    B_LEAVE,

    // Special Casio VRAM-related internal gadget (if any exist in your DB or need to be faked)
    // For now, we'll implement PRINT_CHAR using basic store operations to VRAM addresses.
};

// --- Cấu trúc thông tin về một gadget ---
// Chúng ta sẽ chỉ lưu địa chỉ, vì chức năng đã được mã hóa trong enum.
struct Gadget {
    unsigned int address; // Địa chỉ của gadget
};

// --- Database chứa các gadget ---
class GadgetDB {
public:
    // Ánh xạ chuỗi tên gadget (như trong file) đến enum GadgetFunction
    std::map<std::string, GadgetFunction> name_to_enum_map;
    // Ánh xạ enum GadgetFunction đến địa chỉ thực
    std::map<GadgetFunction, unsigned int> gadget_address_map;

    GadgetDB(); // Constructor để khởi tạo name_to_enum_map

    // Hàm load gadget từ file (ví dụ: nx_u8_gadgets.txt)
    void loadFromFile(const std::string& filepath);

    // Lấy địa chỉ của một gadget theo chức năng
    unsigned int getAddress(GadgetFunction func) const;
};

// --- Lớp ROP Generator ---
class ROPGenerator {
public:
    explicit ROPGenerator(const GadgetDB& db, const SymbolTable& sym_table);
    std::vector<unsigned int> generateROPChain(const ProgramNode& program_node);

private:
    const GadgetDB& gadget_db;
    const SymbolTable& symbol_table;
    std::vector<unsigned int> rop_chain; // Chuỗi ROP (các địa chỉ và dữ liệu)

    // --- Các hàm hỗ trợ sinh mã cho từng loại ASTNode ---
    void generateForNode(const ASTNode& node);
    void generateForVarDeclaration(const VarDeclarationNode& node);
    void generateForAssignment(const AssignmentNode& node);
    void generateForBinaryOp(const BinaryOpNode& node);
    void generateForMemWrite(const MemWriteNode& node);
    void generateForPrintChar(const PrintCharNode& node);

    // --- Hàm trợ giúp cho biểu thức ---
    // Hàm này sẽ sinh ra các gadget để đánh giá một biểu thức và đưa kết quả vào thanh ghi R0.
    // Trả về giá trị của biểu thức, nhưng quan trọng là ROP chain sẽ đặt nó vào R0.
    void evaluateExpressionIntoR0(const ASTNode& expr_node);

    // --- Hàm tiện ích để push địa chỉ gadget và dữ liệu vào ROP chain ---
    void pushGadget(GadgetFunction func);
    void pushData(unsigned int data);

    // --- Chiến lược quản lý thanh ghi đơn giản ---
    // Với nhiều thanh ghi, chúng ta có thể cần một chiến lược tốt hơn.
    // Tạm thời, giả định các gadget Pop/Move có sẵn để di chuyển giá trị.
    // Ví dụ: pushValueIntoR0(value), pushValueIntoR1(value)
    // Để làm phức tạp hơn nữa, chúng ta có thể thêm một "trình phân bổ thanh ghi" (register allocator) đơn giản.
};

#endif // ROP_GENERATOR_H
