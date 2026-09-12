#include "kv/store.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace kv {
namespace fs = std::filesystem;

Store::Store(std::string dir)
    : dir_(std::move(dir)),
      snap_path_(dir_ + "/snapshot.bin"),
      wal_path_(dir_ + "/wal.bin"),
      wal_(wal_path_) {
  fs::create_directories(dir_);
  // Wal ctor opened the file; reopen after possible first create_directories.
  load();
}

Store::~Store() {
  try {
    compact();
  } catch (...) {
  }
}

void Store::load() {
  std::lock_guard<std::mutex> g(mu_);
  mem_.clear();
  std::ifstream in(snap_path_, std::ios::binary);
  if (in) {
    while (in.peek() != EOF) {
      char tag{};
      in.read(&tag, 1);
      if (!in) break;
      std::uint32_t klen = 0, vlen = 0;
      in.read(reinterpret_cast<char*>(&klen), 4);
      std::string k(klen, '\0');
      in.read(k.data(), klen);
      in.read(reinterpret_cast<char*>(&vlen), 4);
      std::string v(vlen, '\0');
      if (vlen) in.read(v.data(), vlen);
      if (!in) break;
      mem_[std::move(k)] = std::move(v);
    }
  }
  for (auto& r : Wal::replay(wal_path_)) {
    if (r.op == Op::Put) mem_[r.key] = r.value;
    else mem_.erase(r.key);
  }
}

void Store::put(std::string key, std::string value) {
  std::lock_guard<std::mutex> g(mu_);
  Record r{Op::Put, key, value};
  wal_.append(r);
  wal_.flush();
  mem_[std::move(key)] = std::move(value);
}

void Store::del(const std::string& key) {
  std::lock_guard<std::mutex> g(mu_);
  Record r{Op::Del, key, {}};
  wal_.append(r);
  wal_.flush();
  mem_.erase(key);
}

std::optional<std::string> Store::get(const std::string& key) const {
  std::lock_guard<std::mutex> g(mu_);
  auto it = mem_.find(key);
  if (it == mem_.end()) return std::nullopt;
  return it->second;
}

void Store::write_snapshot_unlocked() {
  auto tmp = snap_path_ + ".tmp";
  std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
  if (!out) throw std::runtime_error("snapshot open failed");
  for (auto& [k, v] : mem_) {
    char tag = 1;
    out.write(&tag, 1);
    std::uint32_t klen = static_cast<std::uint32_t>(k.size());
    std::uint32_t vlen = static_cast<std::uint32_t>(v.size());
    out.write(reinterpret_cast<const char*>(&klen), 4);
    out.write(k.data(), klen);
    out.write(reinterpret_cast<const char*>(&vlen), 4);
    out.write(v.data(), vlen);
  }
  out.close();
  fs::rename(tmp, snap_path_);
}

void Store::compact() {
  std::lock_guard<std::mutex> g(mu_);
  write_snapshot_unlocked();
  wal_.close();
  {
    std::ofstream trunc(wal_path_, std::ios::binary | std::ios::trunc);
  }
  wal_ = Wal(wal_path_);
}

std::size_t Store::size() const {
  std::lock_guard<std::mutex> g(mu_);
  return mem_.size();
}

}  // namespace kv
