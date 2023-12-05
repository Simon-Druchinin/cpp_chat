#include <iostream>
#include <string>
#include <stdarg.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <unistd.h>


#include <cstdlib>
#include <cstring>
#include <thread>

#include <mutex>
#include <unordered_map>

int output(const char *arg, ...);
int error_output(const char *arg, ...);
void error_handling(const std::string &message);
