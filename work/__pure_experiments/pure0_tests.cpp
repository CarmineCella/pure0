// pure0.cpp
//
#include <iostream>
#include <valarray>
#include <vector>
#include <variant>  

using namespace std;

using Node = std::variant<std::string, std::valarray<double>>;
using Stack = std::vector<Node>;

std::string tokenize(const std::string& str) {
    return str; // Placeholder
}

void print_stack (const Stack& stack) {
    for (const auto& node : stack) {
        if (std::holds_alternative<std::string>(node)) {
            std::cout << std::get<std::string>(node) << std::endl;
        } else if (std::holds_alternative<std::valarray<double>>(node)) {
            const auto& arr = std::get<std::valarray<double>>(node);
            std::cout << "( ";
            for (const auto& val : arr) {
                std::cout << val << " ";
            }
            std::cout << ")" << std::endl;
        }
    }
}
int main() {
    Stack stack;
    stack.push_back("Hello, World!");
    stack.push_back(std::valarray<double>{1.0});
    stack.push_back(std::valarray<double>{11.0, 12.0, 13.0});
    stack.push_back(std::get<std::valarray<double>>(stack.at(1))+std::get<std::valarray<double>>(stack.at(2)));
    print_stack(stack);

    return 0;
}