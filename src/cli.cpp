#include "kv/store.hpp"

#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char** argv) {
  std::string dir = argc > 1 ? argv[1] : "./kvdata";
  kv::Store s(dir);
  std::cout << "kv dir=" << dir << "  commands: put k v | get k | del k | size | compact | quit\n";
  std::string line;
  while (std::cout << "> " && std::getline(std::cin, line)) {
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;
    if (cmd == "quit" || cmd == "exit") break;
    if (cmd == "put") {
      std::string k, v;
      iss >> k;
      std::getline(iss, v);
      if (!v.empty() && v[0] == ' ') v.erase(0, 1);
      s.put(k, v);
      std::cout << "ok\n";
    } else if (cmd == "get") {
      std::string k;
      iss >> k;
      auto v = s.get(k);
      std::cout << (v ? *v : "(nil)") << "\n";
    } else if (cmd == "del") {
      std::string k;
      iss >> k;
      s.del(k);
      std::cout << "ok\n";
    } else if (cmd == "size") {
      std::cout << s.size() << "\n";
    } else if (cmd == "compact") {
      s.compact();
      std::cout << "compacted\n";
    } else if (!cmd.empty()) {
      std::cout << "unknown\n";
    }
  }
}
