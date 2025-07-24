#pragma once
#include <string>
#include <memory>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <optional>
#include "AST/ast.hpp"
#include "AST/gate.hpp"
#include "parser.h"

namespace qarser {

class Preprocessor {
private:
    std::filesystem::path base_path_;
    std::vector<std::filesystem::path> include_paths_;
    
public:
    Preprocessor(const std::filesystem::path& base_path = std::filesystem::current_path()) 
        : base_path_(base_path) {

        include_paths_.push_back(base_path);
        include_paths_.push_back(base_path / "include");
    }
    
    void add_include_path(const std::filesystem::path& path) {
        include_paths_.push_back(path);
    }
    
    // 预处理程序，展开include文件
    std::unique_ptr<Program> preprocess(std::unique_ptr<Program> program) {
        auto result = std::make_unique<Program>();
        result->version = program->version;
        
        for (auto& stmt : program->statements) {
            if (stmt->kind() == Statement::Kind::INCLUDE) {
                auto* include_stmt = static_cast<Include*>(stmt.get());
                
                // qelib1.inc 特殊处理 - 不展开，留给SA处理
                if (include_stmt->filename == "qelib1.inc") {
                    result->statements.push_back(std::move(stmt));
                } 
                else {
                    auto expanded = expand_include(include_stmt->filename, include_stmt->line);
                    if (expanded) {
                        for (auto& expanded_stmt : expanded->statements) {
                            result->statements.push_back(std::move(expanded_stmt));
                        }
                    } else {
                        result->statements.push_back(std::move(stmt));
                    }
                }
            } else {
                // 非include语句直接添加
                result->statements.push_back(std::move(stmt));
            }
        }
        
        return result;
    }
    
private:
    // 查找include文件
    std::optional<std::filesystem::path> find_include_file(const std::string& filename) {
        for (const auto& path : include_paths_) {
            auto file_path = path / filename;
            if (std::filesystem::exists(file_path)) {
                return file_path;
            }
        }
        return std::nullopt;
    }
    
    // 展开include文件
    std::unique_ptr<Program> expand_include(const std::string& filename, int line) {
        auto file_path = find_include_file(filename);
        if (!file_path) {
            return nullptr;
        }
        
        // 读取文件内容
        std::ifstream file(*file_path);
        if (!file.is_open()) {
            return nullptr;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();
        
        // 解析include的文件
        Parser parser;
        try {
            auto included_program = parser.parse(content, file_path->string());
            
            // 递归预处理（处理嵌套的include）
            return preprocess(std::move(included_program));
        } catch (...) {
            // 解析失败，返回nullptr
            return nullptr;
        }
    }
};

} // namespace qarser