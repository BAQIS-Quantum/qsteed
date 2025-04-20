#pragma once
#include <vector>
    
namespace qarser {
    

    class SemanticError {
    public:
        int line;
        std::string message;

    public:
        SemanticError(int line ,const std::string& message) 
            : line(line), message(message) {}
    };


    class ErrorCollector {
    private:
        std::vector<SemanticError> errors;
        
    public:
        ErrorCollector() = default;       

        void add_error(int line, const std::string& message) {
            errors.emplace_back(line, message);
        }

        void report() {
            if (!empty())
                this->print();
            else {
                // std::cout << "SA Done!" << std::endl;
            }
        }


        bool empty() {
            return errors.empty();
        }

        const std::vector<SemanticError>& get_errors() const {
            return errors;
        }

        void print() {
            for (const auto& err : errors) {
                std::cout << "Error at line " << err.line << ": " << err.message << std::endl;
            }
        }




    };


}; // namespace qarser