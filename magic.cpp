#include "magic.h"

namespace DarkChess {

uint64_t cannonMasks[SQUARE_NB] = {0};
uint64_t cannonHTable[SQUARE_NB][256] = {{0}};
uint64_t cannonVTable[SQUARE_NB][256] = {{0}};

uint64_t random_uint64() {
  uint64_t u1, u2, u3, u4;
  u1 = (uint64_t)(rand()) & 0xFFFF; u2 = (uint64_t)(rand()) & 0xFFFF;
  u3 = (uint64_t)(rand()) & 0xFFFF; u4 = (uint64_t)(rand()) & 0xFFFF;
  return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

uint64_t random_uint64_fewbits() {
  return random_uint64() & random_uint64() & random_uint64();
}

uint64_t index_to_uint64(int index, int bits, uint64_t mask) {
  uint64_t blockers = 0ULL;
  for (int i = 0; i < bits; i++) {
    int bitPos = popLsb(mask);
    if (index & (1 << i))
      blockers |= (1UL << bitPos);
  }
  return blockers;
}

uint64_t getBlockersFromIndex(int index, uint64_t mask) {
  int bits = count_1s(mask);
  return index_to_uint64(index, bits, mask);
}

uint64_t cmask(Square sq) {
  uint64_t result = 0ULL;
  File fl = file_of(sq);
  Rank rk = rank_of(sq);

  for (Rank r = rk+RANK_2; r <= RANK_7; ++r) result |= (1UL << (fl + r*4));
  for (Rank r = rk-RANK_2; r >= RANK_2; --r) result |= (1UL << (fl + r*4));
  for (File f = fl+FILE_B; f <= FILE_C; ++f) result |= (1UL << (f + rk*4));
  for (File f = fl-FILE_B; f >= FILE_B; --f) result |= (1UL << (f + rk*4));
  return result;
}

uint64_t getCannonAttack(Square sq, uint64_t blockers) {
  blockers &= cannonMasks[sq];
  uint64_t keyH = (blockers & cannonHMagics[sq]) >> (64 - cBits[sq]);
  uint64_t keyV = (blockers & cannonVMagics[sq]) >> (64 - cBits[sq]);
  return cannonHTable[sq][keyH] | cannonVTable[sq][keyV];
}

uint64_t getCannonAttackSlow(Square sq, uint64_t blockers, int dir) {
  uint64_t attack = Bitboard(0);
  std::vector<Direction> directions;
  
  if (dir == 0) {
    directions = {EAST, WEST};
  } else {
    directions = {NORTH, SOUTH};
  }

  for (Direction d: directions) {
    bool hurdle = false;
    for (Square s = sq + d; is_ok(s) && distance(s, s-d) == 1; s += d) {
      if (hurdle)
        attack |= (1UL << s);
      
      if (blockers & (1UL << s)) {
        if (!hurdle)
          hurdle = true;
        else
          break;
      }
    }
  }

  return attack;
}

int transform(uint64_t b, uint64_t magic, int bits) {
  return (int)((b * magic) >> (64- bits));
}

#ifdef COMPUTE_MAGICS
uint64_t find_magic(Square sq, int m, int dir) {
  uint64_t mask, b[256], a[256], used[256], magic;
  int i, j, k, n, fail;

  mask = cannonMasks[sq];
  n = count_1s(mask);

  for (i = 0; i < (1 << n); i++) {
    b[i] = index_to_uint64(i, n, mask);
    a[i] = getCannonAttackSlow(sq, b[i], dir);
  }
  for (k = 0; k < 100000000; k++) {
    magic = random_uint64_fewbits();
    if (count_1s((mask * magic) & 0xFF00000000000000ULL) < 5) continue;
    for (i = 0; i < 256; i++) used[i] = 0UL;
    for (i = 0, fail = 0; !fail && i < (1 << n); i++) {
      j = transform(b[i], magic, m);

      if (used[j] == 0UL) used[j] = a[i];
      else if (used[j] != a[i]) fail = 1;
    }
    if (!fail) return magic;
  }
  assert(false);
  return 0UL;
}
#endif

void initCannonMasks() {
  for (Square s = SQ_A1; s < SQUARE_NB; ++s) {
    cannonMasks[s] = cmask(s);
  }
}

#ifdef COMPUTE_MAGICS
void initCannonMagic() {
  for (Square s = SQ_A1; s < SQUARE_NB; ++s) {
    cannonHMagics[s] = find_magic(s, cBits[s], 0); // horizontal
    cannonVMagics[s] = find_magic(s, cBits[s], 1); // vertical
  }
}
#endif

void initCannonMagicTable() {
  uint64_t blockers, keyH, keyV;
  for (Square s = SQ_A1; s < SQUARE_NB; ++s) {
    // For all possible blockers for this square
    for (uint64_t blockerIndex = 0; blockerIndex < (1UL << cBits[s]); blockerIndex++) {
      blockers = getBlockersFromIndex(blockerIndex, cannonMasks[s]);
      keyH = (blockers * cannonHMagics[s]) >> (64 - cBits[s]);
      keyV = (blockers * cannonVMagics[s]) >> (64 - cBits[s]);
      cannonHTable[s][keyH] = getCannonAttackSlow(s, blockers, 0);
      cannonVTable[s][keyV] = getCannonAttackSlow(s, blockers, 1);
    }
  }
}

} // namespace DarkChess
