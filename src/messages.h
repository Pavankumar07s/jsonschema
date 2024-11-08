#ifndef MESSAGES_H
#define MESSAGES_H

#include <iostream>
#include <string>
#include "../include/termcolor/termcolor.hpp"

// Declare color control as an external variable
extern bool use_colors;

void print_error(const std::string &message);
void print_success(const std::string &message);

#endif // MESSAGES_H
