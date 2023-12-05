#include <arpa/inet.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "settings.hpp"

void handle_client(int client_sock);
void send_msg(const std::string &msg);
int output(const char *arg, ...);
int error_output(const char *arg, ...);
void error_handling(const std::string &message);

int client_count = 0;
std::mutex mtx;
std::unordered_map<std::string, int> client_socks;

int main(int argc, const char **argv, const char **envp) {
  int serv_sock, client_sock;
  struct sockaddr_in serv_addr, client_addr;
  socklen_t client_addr_size;

  serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (serv_sock == -1) {
    error_handling("socket() failed!");
  }

  memset(&serv_addr, 0, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(SERVER_PORT);

  if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
    error_handling("bind() failed!");
  }
  printf("the server is running on port %d\n", SERVER_PORT);

  if (listen(serv_sock, MAX_CLIENT) == -1) {
    error_handling("listen() error!");
  }

  while (1) {
    client_addr_size = sizeof(client_addr);
    client_sock =
        accept(serv_sock, (struct sockaddr *)&client_addr, &client_addr_size);
    if (client_sock == -1) {
      error_handling("accept() failed!");
    }

    mtx.lock();
    client_count++;
    mtx.unlock();

    std::thread th(handle_client, client_sock);
    th.detach();

    output("Connected client IP: %s \n", inet_ntoa(client_addr.sin_addr));
  }
  close(serv_sock);
  return 0;
}

void handle_client(int client_sock) {
  char msg[BUF_SIZE];
  int flag = 0;

  char tell_name[13] = "#new client:";
  while (recv(client_sock, msg, sizeof(msg), 0) != 0) {
    if (std::strlen(msg) > std::strlen(tell_name)) {
      char pre_name[13];
      std::strncpy(pre_name, msg, 12);
      pre_name[12] = '\0';
      if (std::strcmp(pre_name, tell_name) == 0) {
        char name[20];
        std::strcpy(name, msg + 12);
        if (client_socks.find(name) == client_socks.end()) {
          output("the name of socket %d: %s\n", client_sock, name);
          client_socks[name] = client_sock;
        } else {
          std::string error_msg =
              std::string(name) +
              " exists already. Please quit and enter with another name!";
          send(client_sock, error_msg.c_str(), error_msg.length() + 1, 0);
          mtx.lock();
          client_count--;
          mtx.unlock();
          flag = 1;
        }
      }
    }

    if (flag == 0) send_msg(std::string(msg));
  }
  if (flag == 0) {
    std::string leave_msg;
    std::string name;
    mtx.lock();
    for (auto it = client_socks.begin(); it != client_socks.end(); ++it) {
      if (it->second == client_sock) {
        name = it->first;
        client_socks.erase(it->first);
      }
    }
    client_count--;
    mtx.unlock();
    leave_msg = "client " + name + " leaves the chat room";
    send_msg(leave_msg);
    output("client %s leaves the chat room\n", name.c_str());
    close(client_sock);
  } else {
    close(client_sock);
  }
}

void send_msg(const std::string &msg) {
  mtx.lock();
  std::string pre = "@";
  int first_space = msg.find_first_of(" ");
  if (msg.compare(first_space + 1, 1, pre) == 0) {
    int space = msg.find_first_of(" ", first_space + 1);
    std::string receive_name =
        msg.substr(first_space + 2, space - first_space - 2);
    std::string send_name = msg.substr(1, first_space - 2);
    if (client_socks.find(receive_name) == client_socks.end()) {
      std::string error_msg =
          "[error] there is no client named " + receive_name;
      send(client_socks[send_name], error_msg.c_str(), error_msg.length() + 1,
           0);
    } else {
      send(client_socks[receive_name], msg.c_str(), msg.length() + 1, 0);
      send(client_socks[send_name], msg.c_str(), msg.length() + 1, 0);
    }
  } else {
    for (auto it = client_socks.begin(); it != client_socks.end(); it++) {
      send(it->second, msg.c_str(), msg.length() + 1, 0);
    }
  }
  mtx.unlock();
}

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