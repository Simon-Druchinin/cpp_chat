#include <arpa/inet.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>

#include "settings.hpp"

void send_message(int sock);
void recv_message(int sock);
int error_output(const char *arg, ...);
void error_handling(const std::string &message);

std::string name = "DEFAULT";
std::string message;

int main(int argc, const char **argv, const char **envp) {
  int sock;
  struct sockaddr_in server_addr;

  if (argc != 2) {
    error_output("Usage : %s <Name> \n", argv[0]);
    exit(1);
  }

  name = "[" + std::string(argv[1]) + "]";

  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock == -1) {
    error_handling("socket() failed!");
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(HOST_IP);
  server_addr.sin_port = htons(SERVER_PORT);

  if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
      -1) {
    error_handling("connect() failed!");
  }

  std::string my_name = "#new client:" + std::string(argv[1]);
  send(sock, my_name.c_str(), my_name.length() + 1, 0);

  std::thread send(send_message, sock);
  std::thread recv(recv_message, sock);

  send.join();
  recv.join();

  close(sock);

  return 0;
}

void send_message(int sock) {
  while (1) {
    getline(std::cin, message);

    if (message == "Quit" || message == "quit") {
      close(sock);
      exit(0);
    }

    std::string name_message = name + " " + message;
    send(sock, name_message.c_str(), name_message.length() + 1, 0);
  }
}

void recv_message(int sock) {
  char name_message[BUF_SIZE + name.length() + 1];
  while (1) {
    int str_len = recv(sock, name_message, BUF_SIZE + name.length() + 1, 0);
    if (str_len == -1) {
      exit(-1);
    }
    std::cout << std::string(name_message) << std::endl;
  }
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
