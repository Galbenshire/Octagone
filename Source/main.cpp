// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include "Application.hpp"

int main() {
  try {
    Application app;
    app.run();
  } catch (std::exception& e) {
    std::cout << "\nEXCEPTION: " << e.what() << std::endl;
  }
}
