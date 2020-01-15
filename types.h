#pragma once

#include <array>
#include <stdint.h>

namespace DarkChess {

enum Move : int {
  MOVE_PASS = 1024,
  MOVE_NULL = 1025
};

enum Color : int {
  RED, BLACK, COLOR_NONE,
  COLOR_NB = 2
};

enum class Status {
  RedPlay,
  BlackPlay,
  RedWin,
  BlackWin,
  Draw
};

enum Value : int {
  VALUE_ZERO      = 0,
  VALUE_DRAW      = 0,
  VALUE_KNOWN_WIN = 10000,
  VALUE_MATE      = 32000,
  VALUE_INFINITE  = 32001,
  VALUE_NONE      = 32002,

  PawnValueMg   = 10,    PawnValueEg   = 213,
  CannonValueMg = 200,    CannonValueEg = 600,
  KnightValueMg = 50,    KnightValueEg = 854,
  RookValueMg   = 75,   RookValueEg   = 1380,
  MinisterValueMg = 100,  MinisterValueEg = 915,
  GuardValueMg  = 250,   GuardValueEg   = 2682,
  KingValue = 320,

  MidgameLimit  = 15258,  EndgameLimit = 3915
};

enum PieceType : int {
  PAWN, CANNON, KNIGHT, ROOK, MINISTER, GUARD, KING, DARK,
  EMPTY, ALL_PIECES,
  PIECE_TYPE_NB = 10
};

/*
 * 帥 (將) > 仕 (士) > 相 (像) > 俥 (車) > 傌 (馬) > 炮 (包) > 兵 (卒)
 * King > Guard > Minister > Rook > Knight > Cannon > Pawn
 */
enum Piece : int {
  R_PAWN, R_CANNON, R_KNIGHT, R_ROOK, R_MINISTER, R_GUARD, R_KING,
  B_PAWN = 16, B_CANNON, B_KNIGHT, B_ROOK, B_MINISTER, B_GUARD, B_KING,
  PIECE_DARK = 39,
  NO_PIECE = 40,
  PIECE_NB = 14
};

enum Square : int {
  SQ_A1, SQ_B1, SQ_C1, SQ_D1,
  SQ_A2, SQ_B2, SQ_C2, SQ_D2,
  SQ_A3, SQ_B3, SQ_C3, SQ_D3,
  SQ_A4, SQ_B4, SQ_C4, SQ_D4,
  SQ_A5, SQ_B5, SQ_C5, SQ_D5,
  SQ_A6, SQ_B6, SQ_C6, SQ_D6,
  SQ_A7, SQ_B7, SQ_C7, SQ_D7,
  SQ_A8, SQ_B8, SQ_C8, SQ_D8,
  SQ_NONE,

  SQUARE_NB = 32
};

enum Direction : int {
  NORTH = 4,
  EAST = 1,
  SOUTH = -NORTH,
  WEST = -EAST,
};

enum File : int {
  FILE_A, FILE_B, FILE_C, FILE_D, FILE_NB
};

enum Rank : int {
  RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NB
};

enum Score : int { SCORE_ZERO };

#define ENABLE_BASE_OPERATORS_ON(T)                                \
constexpr T operator+(T d1, T d2) { return T(int(d1) + int(d2)); } \
constexpr T operator-(T d1, T d2) { return T(int(d1) - int(d2)); } \
constexpr T operator-(T d) { return T(-int(d)); }                  \
inline T& operator+=(T& d1, T d2) { return d1 = d1 + d2; }         \
inline T& operator-=(T& d1, T d2) { return d1 = d1 - d2; }

#define ENABLE_INCR_OPERATORS_ON(T)                                \
inline T& operator++(T& d) { return d = T(int(d) + 1); }           \
inline T& operator--(T& d) { return d = T(int(d) - 1); }

#define ENABLE_FULL_OPERATORS_ON(T)                                \
ENABLE_BASE_OPERATORS_ON(T)                                        \
ENABLE_INCR_OPERATORS_ON(T)

ENABLE_INCR_OPERATORS_ON(PieceType)
ENABLE_INCR_OPERATORS_ON(Piece)
ENABLE_INCR_OPERATORS_ON(Color)

ENABLE_FULL_OPERATORS_ON(Square)
ENABLE_FULL_OPERATORS_ON(File)
ENABLE_FULL_OPERATORS_ON(Rank)

#undef ENABLE_BASE_OPERATORS_ON
#undef ENABLE_INCR_OPERATORS_ON
#undef ENABLE_FULL_OPERATORS_ON

/// Additional operators to add a Direction to a Square
constexpr Square operator+(Square s, Direction d) { return Square(int(s) + int(d)); }
constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d)); }
inline Square& operator+=(Square& s, Direction d) { return s = s + d; }
inline Square& operator-=(Square& s, Direction d) { return s = s - d; }

using Bitboard = uint32_t;

constexpr Bitboard AllSquares = 0xFFFFFFFFUL;
constexpr Bitboard FileDBB = 0x11111111UL;
constexpr Bitboard FileCBB = FileDBB << 1;
constexpr Bitboard FileBBB = FileDBB << 2;
constexpr Bitboard FileABB = FileDBB << 3;
constexpr Bitboard Rank1BB = 0x0000000FUL;
constexpr Bitboard Rank2BB = Rank1BB << 4;
constexpr Bitboard Rank3BB = Rank2BB << 4;
constexpr Bitboard Rank4BB = Rank3BB << 4;
constexpr Bitboard Rank5BB = Rank4BB << 4;
constexpr Bitboard Rank6BB = Rank5BB << 4;
constexpr Bitboard Rank7BB = Rank6BB << 4;
constexpr Bitboard Rank8BB = Rank7BB << 4;

template<Direction D>
constexpr Bitboard shift(Bitboard b) {
  return  D == NORTH      ?  b             << 4 : D == SOUTH      ?  b             >> 4
        : D == EAST       ? (b & ~FileDBB) << 1 : D == WEST       ? (b & ~FileABB) >> 1
        : 0;
}

constexpr Color operator~(Color c) {
  return Color(c ^ BLACK); // Toggle color
}

constexpr Square make_square(File f, Rank r) {
  return Square((r << 2) + f);
}

constexpr Piece make_piece(Color c, PieceType pt) {
  return Piece((c << 4) + pt);
}

inline int get_piece(Piece pc) {
  if (int(pc) > 22) {
    return int(pc)-25;
  } else if (int(pc) > 6) {
    return int(pc)-9;
  }
  return int(pc);
}

inline int get_piece(Color c, PieceType pt) {
  return get_piece(make_piece(c, pt));
}

inline PieceType type_of(Piece pc) {
  return PieceType(pc & 15);
}

inline Color color_of(Piece pc) {
  return Color(pc >> 4);
}

constexpr bool is_ok(Square s) {
  return s >= SQ_A1 && s <= SQ_D8;
}

constexpr File file_of(Square s) {
  return File(s & 3);
}

constexpr Rank rank_of(Square s) {
  return Rank(s >> 2);
}

constexpr Square from_sq(Move m) {
  return Square(m >> 5);
}

constexpr Square to_sq(Move m) {
  return Square(m & 31);
}

constexpr Move make_move(Square from, Square to) {
  return Move((from << 5) + to);
}

constexpr bool is_move_ok(Move m) {
  return from_sq(m) != to_sq(m);
}

inline Bitboard LS1B(Bitboard b) {
  return b & (-b);
}

inline int count_1s(uint64_t b) {
  int n;
  for (n = 0; b; n++, b &= b - 1);
  return n;
}

inline int popCount(Bitboard b) {
  int n;
  for (n = 0; b; n++, b &= b - 1);
  return n;
}

/// popLsb() finds and clears the least significant bit in a non-zero bitboard

inline int popLsb(uint64_t &board) {
  int lsbIndex = __builtin_ffsll(board) - 1;
  board &= board - 1;
  return lsbIndex;
}

inline Square popLsb(Bitboard &board) {
  int lsbIndex = __builtin_ffsll(board) - 1;
  Square s = Square(lsbIndex);
  board &= board - 1;
  return s;
}

inline int distance(Square x, Square y) {
  uint8_t fdist = std::abs(file_of(x) - file_of(y));
  uint8_t rdist = std::abs(rank_of(x) - rank_of(y));
  return std::max(fdist, rdist);
}

const Bitboard pMoves[32] = {
  0x00000012UL, 0x00000025UL, 0x0000004AUL, 0x00000084UL,
  0x00000121UL, 0x00000252UL, 0x000004A4UL, 0x00000848UL,
  0x00001210UL, 0x00002520UL, 0x00004A40UL, 0x00008480UL,
  0x00012100UL, 0x00025200UL, 0x0004A400UL, 0x00084800UL,
  0x00121000UL, 0x00252000UL, 0x004A4000UL, 0x00848000UL,
  0x01210000UL, 0x02520000UL, 0x04A40000UL, 0x08480000UL,
  0x12100000UL, 0x25200000UL, 0x4A400000UL, 0x84800000UL,
  0x21000000UL, 0x52000000UL, 0xA4000000UL, 0x48000000UL
};

#define MAX_MOVES 72 // 16 pieces x 4 directions + 2 * 4 (cannon)
using MoveList = std::array<Move, MAX_MOVES>;

const Square index32[32] = {SQ_D8, SQ_A1, SQ_B1, SQ_B2, SQ_C1, SQ_A5, SQ_D7, SQ_C2, SQ_D1, SQ_C4, SQ_B5, SQ_D5, SQ_A8, SQ_D3, SQ_D2, SQ_B6,
                   SQ_C8, SQ_A2, SQ_D4, SQ_C7, SQ_B4, SQ_C5, SQ_C3, SQ_A6, SQ_B8, SQ_B7, SQ_A4, SQ_B3, SQ_A7, SQ_A3, SQ_D6, SQ_C6};

inline int BitsHash(Bitboard x) {
  return (x * 0x08ED2BE6UL) >> 27;
}

inline Square GetIndex(Bitboard mask) {
  int idx = BitsHash(mask);
  //assert(idx < SQUARE_NB);
  return index32[idx];
}

extern uint64_t hashArray[SQUARE_NB][PIECE_NB+1];
extern uint64_t hashTurn;

} // namespace DarkChess