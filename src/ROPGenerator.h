#ifndef ROP_GENERATOR_H
#define ROP_GENERATOR_H

#include "Parser.h" // Cần các định nghĩa AST và SymbolTable
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

// Định nghĩa các loại Gadget mà bạn đã có
enum class GadgetFunction {
    POP_R0_RET,     // pop R0; ret;
    POP_R1_RET,     // pop R1; ret;
    POP_R2_RET,     // pop R2; ret;
    ADD_R0_R1_RET,  // R0 = R0 + R1; ret;
    SUB_R0_R1_RET,  // R0 = R0 - R1; ret;
    LOAD_R0_FROM_R1_RET, // R0 = [R1]; ret; (Load from memory pointed by R1 into R0)
    STORE_R0_TO_R1_RET,  // [R1] = R0; ret; (Store R0 to memory pointed by R1)
    // Có thể thêm các gadget cho so sánh, nhảy, syscall, v.v.
    SYSCALL_PRINT_CHAR, // Giả định một gadget đặc biệt để gọi syscall in ký tự (nếu có)
    UNKNOWN_GADGET
};

// Cấu trúc thông tin về một gadget
struct Gadget {
    GadgetFunction function;
    unsigned int address; // Địa chỉ của gadget
};

// Database chứa các gadget
class GadgetDB {
public:
    std::map<GadgetFunction, unsigned int> gadget_map; // Ánh xạ chức năng -> địa chỉ

    // Hàm load gadget từ file (ví dụ: nx_u8_gadgets.txt)
    void loadFromFile(const std::string& filepath);

    // Lấy địa chỉ của một gadget theo chức năng
    unsigned int getAddress(GadgetFunction func) const;
};

// Lớp ROP Generator
class ROPGenerator {
public:
    explicit ROPGenerator(const GadgetDB& db, const SymbolTable& sym_table);
    std::vector<unsigned int> generateROPChain(const ProgramNode& program_node);

private:
    const GadgetDB& gadget_db;
    const SymbolTable& symbol_table;
    std::vector<unsigned int> rop_chain; // Chuỗi ROP (các địa chỉ và dữ liệu)

    // Hàm sinh ROP cho từng loại ASTNode
    void generateForNode(const ASTNode& node);
    void generateForAssignment(const AssignmentNode& node);
    void generateForBinaryOp(const BinaryOpNode& node);
    unsigned int generateForExpression(const ASTNode& node); // Trả về giá trị của biểu thức vào R0
    void generateForMemWrite(const MemWriteNode& node);
    unsigned int generateForMemRead(const MemReadNode& node);
    void generateForPrintChar(const PrintCharNode& node);

    // Hàm tiện ích để push địa chỉ gadget và dữ liệu vào ROP chain
    void pushGadget(GadgetFunction func);
    void pushData(unsigned int data);

    // Hàm trợ giúp để xử lý biểu thức và đưa kết quả vào thanh ghi tạm (ví dụ R0)
    // Sẽ trả về thanh ghi mà kết quả được lưu
    // R0 là thanh ghi kết quả mặc định.
    unsigned int evaluateExpressionIntoRegister(const ASTNode& expr_node);
};

#endif // ROP_GENERATOR_H
