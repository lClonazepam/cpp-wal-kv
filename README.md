# cpp-wal-kv

A small embedded KV that actually survives process death:

1. Every `put`/`del` is appended to a binary WAL and flushed
2. Reads hit an in-memory `std::map` (memtable)
3. On open, load `snapshot.bin` then replay `wal.bin`
4. `compact()` writes a new snapshot atomically (`*.tmp` + rename) and truncates the WAL

This is the Bitcask / Redis AOF / LevelDB memtable+WAL skeleton. Interview talking points: durability vs fsync, torn writes (replay stops on a short record), snapshot atomicity, why the WAL is the source of truth between compactions.

## Layout

```
include/kv/wal.hpp
include/kv/store.hpp
src/wal.cpp
src/store.cpp
src/demo.cpp   # crash-recovery walkthrough under /tmp
src/cli.cpp    # interactive REPL
```

## Build

```bash
cmake -S . -B build && cmake --build build -j
./build/kv_demo
./build/kv_cli ./kvdata
```
