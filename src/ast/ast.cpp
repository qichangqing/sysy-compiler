#include "ast.hpp"

fstream *BaseAST::oir=nullptr;

// 静态函数实现，用于打开文件并初始化指针
void BaseAST::initOdir(const std::string& filename, std::ios_base::openmode mode) {
    if (oir == nullptr) {
        // 首次调用时创建fstream对象
        oir = new std::fstream(filename, mode);

        // 检查文件是否成功打开
        if (!oir->is_open()) {
            // 处理错误，例如删除指针并抛出异常
            delete oir;
            oir = nullptr;
            throw std::runtime_error("Failed to open file.");
        }
    }
}