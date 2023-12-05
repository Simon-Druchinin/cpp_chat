#include "common.hpp"



int output(const char *arg, ...) {
  int res;
  va_list ap;
  va_start(ap, arg);
  res = vfprintf(stdout, arg, ap);
  va_end(ap);
  return res;
}

int error_output(const char *arg, ...) {
  int res;
  va_list ap;
  va_start(ap, arg);
  res = vfprintf(stderr, arg, ap);
  va_end(ap);
  return res;
}

void error_handling(const std::string &message) {
  std::cerr << message << std::endl;
  exit(1);
}

