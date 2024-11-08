#include "messages.h"

// Define print functions with color control

void print_error(const std::string &message) {
  if (use_colors) {
    std::cerr << termcolor::red << "error: " << message << termcolor::reset << "\n";
  } else {
    std::cerr << "error: " << message << "\n";
  }
}

void print_success(const std::string &message) {
  if (use_colors) {
    std::cout << termcolor::green << message << termcolor::reset << "\n";
  } else {
    std::cout << message << "\n";
  }
}
