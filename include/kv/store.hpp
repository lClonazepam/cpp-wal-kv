#pragma once

#include "kv/wal.hpp"

#include <map>
#include <mutex>
#include <optional>
#include <string>

namespace kv {

class Store {
 public:
  explicit Store(std::string dir);
  ~Store();

  void put(std::string key, std::string value);
  void del(const std::string& key);
  std::optional<std::string> get(const std::string& key) const;

  // Rewrite snapshot + truncate WAL. Safe to call anytime.
  void compact();

  std::size_t size() const;
  const std::string& dir() const { return dir_; }

 private:
  void load();
  void write_snapshot_unlocked();

  std::string dir_;
  std::string snap_path_;
  std::string wal_path_;
  mutable std::mutex mu_;
  std::map<std::string, std::string> mem_;
  Wal wal_;
};

}  // namespace kv
