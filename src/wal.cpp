#include "kv/wal.hpp"

#include <stdexcept>

namespace kv {
namespace {

void write_u32(std::ostream& o, std::uint32_t v) {
  char b[4];
  b[0] = static_cast<char>(v & 0xff);
  b[1] = static_cast<char>((v >> 8) & 0xff);
  b[2] = static_cast<char>((v >> 16) & 0xff);
  b[3] = static_cast<char>((v >> 24) & 0xff);
  o.write(b, 4);
}

void write_u8(std::ostream& o, std::uint8_t v) {
  char c = static_cast<char>(v);
  o.write(&c, 1);
}

std::uint32_t read_u32(std::istream& i) {
  char b[4];
  i.read(b, 4);
  if (i.gcount() != 4) return 0;
  return static_cast<std::uint8_t>(b[0]) |
         (static_cast<std::uint32_t>(static_cast<std::uint8_t>(b[1])) << 8) |
         (static_cast<std::uint32_t>(static_cast<std::uint8_t>(b[2])) << 16) |
         (static_cast<std::uint32_t>(static_cast<std::uint8_t>(b[3])) << 24);
}

std::uint8_t read_u8(std::istream& i) {
  char c{};
  i.read(&c, 1);
  return static_cast<std::uint8_t>(c);
}

}  // namespace

Wal::Wal(std::string path) : path_(std::move(path)) {
  out_.open(path_, std::ios::binary | std::ios::app);
  if (!out_) throw std::runtime_error("open wal failed: " + path_);
}

Wal::~Wal() { close(); }

void Wal::append(const Record& r) {
  write_u8(out_, static_cast<std::uint8_t>(r.op));
  write_u32(out_, static_cast<std::uint32_t>(r.key.size()));
  out_.write(r.key.data(), static_cast<std::streamsize>(r.key.size()));
  write_u32(out_, static_cast<std::uint32_t>(r.value.size()));
  if (!r.value.empty()) {
    out_.write(r.value.data(), static_cast<std::streamsize>(r.value.size()));
  }
}

void Wal::flush() { out_.flush(); }

void Wal::close() {
  if (out_.is_open()) {
    out_.flush();
    out_.close();
  }
}

std::vector<Record> Wal::replay(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  std::vector<Record> out;
  if (!in) return out;
  while (in.peek() != EOF) {
    Record r;
    auto op = read_u8(in);
    if (!in) break;
    r.op = static_cast<Op>(op);
    auto klen = read_u32(in);
    if (!in) break;
    r.key.resize(klen);
    in.read(r.key.data(), klen);
    if (!in) break;
    auto vlen = read_u32(in);
    if (!in) break;
    r.value.resize(vlen);
    if (vlen) in.read(r.value.data(), vlen);
    if (!in) break;
    if (r.op != Op::Put && r.op != Op::Del) break;
    out.push_back(std::move(r));
  }
  return out;
}

}  // namespace kv
