#include <contracts>
#include <iostream>
#include <source_location>

void handle_contract_violation(const std::contracts::contract_violation &violation) noexcept {
    const auto loc = violation.location();
    std::cerr << "=======================================\n"
              << "C++26 CONTRACT VIOLATION DETECTED!\n"
              << "  File:     " << loc.file_name() << "\n"
              << "  Line:     " << loc.line() << "\n"
              << "  Function: " << loc.function_name() << "\n"
              << "  Comment:  " << violation.comment() << "\n"
              << "=======================================\n"
              << std::endl;
}
