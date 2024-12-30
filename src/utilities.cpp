#include "utilities.h"

std::string StarryPurple::dtos(double val, int digit) {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(digit) << val;
  return ss.str();
}