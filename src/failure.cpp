#include "failure.hpp"
#include <iostream>

void fail(const char *failMessage) {
  std::cerr << failMessage << std::endl;
  exit(1);
}
