#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace kv {

enum class Op : std::uint8_t { Put = 1, Del = 2 };

struct Record {
  Op op{};
  std::string key;
  std::string value;  // empty on Del
};

class Wal {
 public:
  explicit Wal(std::string path);
  ~Wal();

  void append(const Record& r);
  void flush();
  void close();

  static std::vector<Record> replay(const std::string& path);

 private:
  std::string path_;
  std::ofstream out_;
};

}  // namespace kv
