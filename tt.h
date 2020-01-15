#pragma once

#include "types.h"
#include <unordered_map>

using namespace DarkChess;

namespace Trans {

enum Flag {
  EXACT,
  LOWER_BOUND,
  UPPER_BOUND
};

struct TTEntry {
  TTEntry(int _score, int _depth, DarkChess::Move _bestMove, Flag _flag)
    : score(_score), depth(_depth), bestMove(_bestMove), flag(_flag) {}

  int score;
  int depth;
  DarkChess::Move bestMove;
  Flag flag;
};

class TranspTable {
  public:
    TranspTable() = default;
    void set(uint64_t Zkey, TTEntry entry) {
      auto insertResult = table.emplace(Zkey, entry);

      // If key already exist, overwrite the value
      if (!insertResult.second) {
        insertResult.first->second = entry;
      }
    }

    const TTEntry *getEntry(const uint64_t Zkey) {
      auto got = table.find(Zkey);
      if (got != table.end()) {
        return &table.at(Zkey);
      } else {
        return nullptr;
      }
    }
    
    void clear() { table.clear(); }

  private:
    std::unordered_map<uint64_t, TTEntry> table;
};

} // namespace Trans