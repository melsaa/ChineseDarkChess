#pragma once

#include <cassert>
#include <iostream>
#include <sstream>
#include <ctype.h>
#include <random>
#include <string.h>
#include <queue>
#include <bitset>

#include "types.h"
#include "magic.h"
#include "move_ordering.h"

namespace DarkChess {

class Board {
  public:
    Board(int seed = 9);
    void clear_bitboards();
    void init_hash();
    void init();
    void set_from_FEN(std::string FEN);

    template <Color Us> int legal_normal_actions(MoveList &mL, ScoreList &sL, int idx);
    Bitboard CGen(Bitboard src);
    template <Color Us> int legal_capture_actions(MoveList &mL, ScoreList &sL, int idx);
    int legal_flip_actions(MoveList &mL, int idx);
    void flip_move(Move m, Piece p, Color c);
    void undo_flip(Move m, Piece p, Color c);
    void do_move(Move m, Piece &captured);
    void undo_move(Move m, Piece captured);
    int get_legal_moves(MoveList &mList, ScoreList &sList);
    bool genmove(Move &m);

    Color side_to_move() const;
    int get_gameLength() const;
    std::minstd_rand getRng() const;
    uint64_t getHash() const;
    int getRepetition() const;
    int getNoCFMoves() const;
    int get_score(Color c) const;
    int get_pieceCount(Piece p) const;
    bool is_dark(Square s) const;

    void update_status(int legalMoves);
    void update_history();
    void update_basic_value(Color Us);
    void update_material_score(Color Us);
    void update_attack_score(Piece p, Square src);
    
    bool is_terminal() const;
    Color who_won() const;
    int num_of_dark_pieces() const;
    int evaluate(Color Us) const;

    std::string print_move(Move m) const;
    char print_piece(Piece p) const;
    std::string print_board() const;

  private:
    std::minstd_rand rng;
    uint64_t hash_;
    std::queue<uint64_t> history;
    int repetition;
    int noCaptureFlipMoves;
    Color sideToMove;
    Status status_;
    int gameLength;
    int pieceCount[PIECE_NB+2]; // + PIECE_DARK, NO_PIECE
    Piece board[SQUARE_NB];
    Bitboard byTypeBB[PIECE_TYPE_NB]; // 0-6, 7: Dark, 8: Empty, 9: ALL_PIECES
    Bitboard byColorBB[COLOR_NB+1]; // RED, BLACK, DARK
    int score[COLOR_NB];
    int aScore[COLOR_NB];
    int BV[PIECE_NB]; // Basic Value
    int MV[PIECE_NB]; // Material Value
    int TV[PIECE_NB][PIECE_NB]; // Threat Value

    inline Bitboard pieces(Color c) const {
      return byColorBB[c];
    }

    inline Bitboard pieces(PieceType pt) const {
      return byTypeBB[pt];
    }

    inline Bitboard pieces(Color c, PieceType pt) const {
      return byColorBB[c] & byTypeBB[pt];
    }

    inline Bitboard pieces(Color c, PieceType pt1, PieceType pt2) const {
      return byColorBB[c] & (byTypeBB[pt1] | byTypeBB[pt2]);
    }

    inline Bitboard pieces(Color c, PieceType pt1, PieceType pt2, PieceType pt3) const {
      return byColorBB[c] & (byTypeBB[pt1] | byTypeBB[pt2] | byTypeBB[pt3]);
    }

    inline Bitboard pieces(Color c, PieceType pt1, PieceType pt2, PieceType pt3, PieceType pt4) const {
      return byColorBB[c] & (byTypeBB[pt1] | byTypeBB[pt2] | byTypeBB[pt3] | byTypeBB[pt4]);
    }

    inline Piece piece_on(Square s) const {
      return board[s];
    }

    inline void move_piece(Piece pc, Square from, Square to) {
      Bitboard fromTo = (1UL << from) | (1UL << to);
      byTypeBB[ALL_PIECES] ^= fromTo;
      byTypeBB[type_of(pc)] ^= fromTo;
      byColorBB[color_of(pc)] ^= fromTo;
      board[from] = NO_PIECE;
      board[to] = pc;
      hash_ ^= hashArray[from][get_piece(pc)];
      hash_ ^= hashArray[to][get_piece(pc)];
    }

    inline void put_piece(Piece pc, Square s) {
      //std::cerr << "put_piece\n";
      Bitboard mask = (1UL << s);
      board[s] = pc;
      byTypeBB[ALL_PIECES] ^= mask;
      byTypeBB[type_of(pc)] ^= mask;
      byColorBB[color_of(pc)] ^= mask;
      pieceCount[get_piece(pc)]++;
      hash_ ^= hashArray[s][get_piece(pc)];
      //std::cerr << std::bitset<32>(mask) << std::endl;
      //std::cerr << std::bitset<32>(byTypeBB[EMPTY]) << std::endl;
    }

    inline void remove_piece(Piece pc, Square s) {
      //std::cerr << "remove piece\n";
      Bitboard mask = (1UL << s);
      board[s] = NO_PIECE;
      byTypeBB[ALL_PIECES] ^= mask;
      byTypeBB[type_of(pc)] ^= mask;
      byColorBB[color_of(pc)] ^= mask;
      pieceCount[get_piece(pc)]--;
      if (pc != NO_PIECE) {
        hash_ ^= hashArray[s][get_piece(pc)];
      }
      //std::cerr << std::bitset<32>(mask) << std::endl;
      //std::cerr << std::bitset<32>(byTypeBB[EMPTY]) << std::endl;
    }
};

} // namespace DarkChess