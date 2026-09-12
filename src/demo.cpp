#include "kv/store.hpp"

#include <cassert>
#include <filesystem>
#include <iostream>

int main() {
  const std::string dir = "/tmp/cpp-wal-kv-demo";
  std::filesystem::remove_all(dir);
  {
    kv::Store s(dir);
    s.put("user:1", "alice");
    s.put("user:2", "bob");
    s.put("user:1", "alice-updated");
    s.del("user:2");
    auto v = s.get("user:1");
    std::cout << "live get user:1=" << (v ? *v : "<miss>") << " size=" << s.size() << "\n";
  }
  {
    kv::Store s(dir);
    auto a = s.get("user:1");
    auto b = s.get("user:2");
    std::cout << "reopen user:1=" << (a ? *a : "<miss>")
              << " user:2=" << (b ? *b : "<miss>") << " size=" << s.size() << "\n";
    s.put("k", std::string(1024, 'x'));
    s.compact();
    std::cout << "after compact size=" << s.size() << "\n";
  }
  {
    kv::Store s(dir);
    auto k = s.get("k");
    std::cout << "reopen after compact k.len=" << (k ? k->size() : 0) << "\n";
  }
}
