#include "state.h"

namespace DarkChess {

uint64_t hashArray[SQUARE_NB][PIECE_NB+1];
uint64_t hashTurn;

Board::Board(int seed) : rng(seed) {
  // Initialize magic bitboard
  initCannonMasks();
  initCannonMagicTable();

  init_hash();
}

void Board::clear_bitboards() {
  // Initialize square board and bitboard
  for (Color c = RED; c < COLOR_NB; ++c) {
    byColorBB[c] = Bitboard(0);
  }

  for (PieceType pt = PAWN; pt < PIECE_TYPE_NB; ++pt) {
    byTypeBB[pt] = Bitboard(0);
  }

  for (Square s = SQ_A1; s < SQUARE_NB; ++s) {
    board[s] = NO_PIECE;
  }

  for (Piece p = R_PAWN; p <= B_KING; ++p) {
    pieceCount[get_piece(p)] = 0;
  }
  pieceCount[get_piece(PIECE_DARK)] = 0;
  pieceCount[get_piece(NO_PIECE)] = 32;

  hash_ = 0;
}

void Board::init_hash() {
  for (Square s = SQ_A1; s < SQUARE_NB; ++s) {
    for (Piece p = R_PAWN; p <= PIECE_DARK; ++p) {
      hashArray[s][get_piece(p)] = 0;
      for (int k = 0; k < 64; ++k) {
        if ((rng() / (RAND_MAX + 1.0)) > 0.5) {
          hashArray[s][get_piece(p)] |= (1ULL << k);
        }
      }
    }
  }
  
  hashTurn = 0;
  for (int k = 0; k < 64; k++) {
    if ((rng() / (RAND_MAX + 1.0)) > 0.5) {
      hashTurn |= (1ULL << k);
    }
  }
}

void Board::init() {
  sideToMove = COLOR_NONE;
  status_ = Status::RedPlay;
  gameLength = 0;

  clear_bitboards();

  for (Square s = SQ_A1; s < SQUARE_NB; ++s) {
    put_piece(PIECE_DARK, s);
  }
}

void Board::set_from_FEN(std::string FEN) {
  std::istringstream fenStream(FEN);
  std::string token;
  
  clear_bitboards();

  //std::cerr << "After clear bitboards\n";
  //std::cerr << std::bitset<32>(pieces(EMPTY)) << std::endl;
  Square s = SQ_A8; // Fen string starts at a8 = index 28
  fenStream >> token;
  for (auto curChar : token) {
    switch(curChar) {
      case 'p': put_piece(make_piece(RED, PAWN), s);
                ++s; break;
      case 'c': put_piece(make_piece(RED, CANNON), s);
                ++s; break;
      case 'n': put_piece(make_piece(RED, KNIGHT), s);
                ++s; break;
      case 'r': put_piece(make_piece(RED, ROOK), s);
                ++s; break;
      case 'm': put_piece(make_piece(RED, MINISTER), s);
                ++s; break;
      case 'g': put_piece(make_piece(RED, GUARD), s);
                ++s; break;
      case 'k': put_piece(make_piece(RED, KING), s);
                ++s; break;
      case 'P': put_piece(make_piece(BLACK, PAWN), s);
                ++s; break;
      case 'C': put_piece(make_piece(BLACK, CANNON), s);
                ++s; break;
      case 'N': put_piece(make_piece(BLACK, KNIGHT), s);
                ++s; break;
      case 'R': put_piece(make_piece(BLACK, ROOK), s);
                ++s; break;
      case 'M': put_piece(make_piece(BLACK, MINISTER), s);
                ++s; break;
      case 'G': put_piece(make_piece(BLACK, GUARD), s);
                ++s; break;
      case 'K': put_piece(make_piece(BLACK, KING), s);
                ++s; break;
      case 'd': put_piece(PIECE_DARK, s);
                ++s; break;
      case '/': s -= Square(8); // Go down one rank
                break;
      default: s += Square(curChar - '0');
    }
  }

  fenStream >> token;
  if (token == "r") {
    sideToMove = RED;
    status_ = Status::RedPlay;
  } else if (token == "b") {
    sideToMove = BLACK;
    status_ = Status::BlackPlay;
  }

  fenStream >> token;
  gameLength = stoi(token);
}

template <Color Us>
int Board::legal_normal_actions(MoveList &mL, int idx) {
  Bitboard dest;
  
  //std::cout << std::bitset<32>(pieces(ALL_PIECES)) << std::endl;
  //std::cout << std::bitset<32>(~pieces(ALL_PIECES)) << std::endl;
  for (PieceType p = PAWN; p <= KING; ++p) {
    Bitboard b = pieces(Us, p);
    while (b) {
      Bitboard mask = LS1B(b);
      b ^= mask;
      Square src = popLsb(mask);
      assert(type_of(board[src]) == p);
      dest = pMoves[src] & (~pieces(ALL_PIECES));
      //std::cout << src << " " << std::bitset<32>(dest) << std::endl;

      while (dest) {
        Bitboard mask2 = LS1B(dest);
        dest ^= mask2;
        Square result = popLsb(mask2);
        if (type_of(board[result]) != EMPTY) {
          std::cout << result << " type " << type_of(board[result]);
          assert(type_of(board[result]) == EMPTY);
        }
        mL[idx++] = make_move(src, result);
      }
    }
  }
  return idx;
}

template <Color Us>
int Board::legal_capture_actions(MoveList &mL, int idx) {
  Bitboard dest, can_be_captured;
  constexpr Color Op = ~Us;
  for (PieceType p = PAWN; p <= KING; ++p) {
    Bitboard b = pieces(Us, p);
    if (p == PAWN) {
      can_be_captured = pieces(Op, KING, PAWN);
    } else if (p == KNIGHT) {
      can_be_captured = pieces(Op, KNIGHT, CANNON, PAWN);
    } else if (p == ROOK) {
      can_be_captured = pieces(Op, ROOK, KNIGHT, CANNON, PAWN);
    } else if (p == MINISTER) {
      can_be_captured = pieces(Op) ^ pieces(Op, KING, GUARD);
    } else if (p == GUARD) {
      can_be_captured = pieces(Op) ^ pieces(Op, KING);
    } else if (p == KING) {
      can_be_captured = pieces(Op) ^ pieces(Op, PAWN);
    } else if (p == CANNON) {
      can_be_captured = 0x0;
    } else{
      assert(false);
    }
    while (b) {
      Bitboard mask = LS1B(b);
      b ^= mask;
      Square src = popLsb(mask);
      assert(type_of(board[src]) == p);
      if (p == CANNON) {
        dest = getCannonAttackSlow(src, pieces(ALL_PIECES), 2) & pieces(Op);
        //std::cout << "Legal cannon actions\n";
        //std::cout << std::bitset<32>(pieces(ALL_PIECES)) << std::endl;
        //std::cout << std::bitset<32>(dest) << std::endl;
      } else {
        dest = pMoves[src] & can_be_captured;
      }

      while (dest) {
        Bitboard mask2 = LS1B(dest);
        dest ^= mask2;
        Square result = popLsb(mask2);
        assert(color_of(board[result]) == Op);
        mL[idx++] = make_move(src, result);
      }
    }
  }
  return idx;
}

int Board::legal_flip_actions(MoveList &mL, int idx) {
  Bitboard b = pieces(DARK);

  while (b) {
    Bitboard mask = LS1B(b);
    b ^= mask;
    Square src = popLsb(mask);
    mL[idx++] = make_move(src, src);
  }
  
  return idx;
}

void Board::flip_move(Move m, Piece p, Color c) {
  if (m != MOVE_PASS) {
    if (!is_move_ok(m)) {
      assert(board[from_sq(m)] == PIECE_DARK);
      remove_piece(PIECE_DARK, from_sq(m));
      put_piece(p, from_sq(m));

      // first flip determines player's color
      if (sideToMove == COLOR_NONE) {
        sideToMove = c;
      }
      sideToMove = ~sideToMove;
      hash_ ^= hashTurn;
      gameLength++;
      //std::cout << print_board() << std::endl;
    } else {
      std::cerr << "Move not ok\n";
    }
  } else {
    std::cerr << "MOVE PASS\n";
  }
  update_material_score(RED);
  update_material_score(BLACK);
}

void Board::do_move(Move m, Piece &captured) {
  if (m != MOVE_PASS) {
    if (!is_move_ok(m)) {
      std::cerr << "from " << from_sq(m) << " to " << to_sq(m) << std::endl;
      assert(is_move_ok(m));
    }

    // Key k = st->key ^ Zobrist::side;
    Color us = sideToMove;
    Square from = from_sq(m);
    Square to = to_sq(m);
    Piece pc = piece_on(from);
    captured = piece_on(to);
    Color c = color_of(pc);
    
    if (c != us) {
      std::cerr << color_of(pc) << std::endl;
      assert(c != COLOR_NONE);
      assert(c == us);
    }
    
    if (captured != NO_PIECE) {
      Square capsq = to;
      remove_piece(captured, capsq);
      // k ^= Zobrist::psq[captured][capsq];
      update_material_score(RED);
      update_material_score(BLACK);
    }
    // k ^= Zobrist::psq[pc][from] ^ Zobrist::psq[pc][to];
    move_piece(pc, from, to);
  }
  sideToMove = ~sideToMove;
  hash_ ^= hashTurn;
  gameLength++;
}

void Board::undo_move(Move m, Piece captured) {
  Square from = from_sq(m);
  Square to = to_sq(m);
  Piece pc = piece_on(to);

  // k ^= Zobrist::psq[pc][from] ^ Zobrist::psq[pc][to];
  move_piece(pc, to, from);

  if (captured != NO_PIECE) {
    put_piece(captured, to);
    // k ^= Zobrist::psq[captured][capsq];
    update_material_score(RED);
    update_material_score(BLACK);
  }

  sideToMove = ~sideToMove;
  gameLength--;
}

int Board::get_legal_moves(MoveList &mList) {
  int size = 0;
  if (sideToMove == RED) {
    size = legal_normal_actions<RED>(mList, 0);
    size = legal_capture_actions<RED>(mList, size);
  } else if (sideToMove == BLACK) {
    size = legal_normal_actions<BLACK>(mList, 0);
    size = legal_capture_actions<BLACK>(mList, size);
  }

  //size = legal_flip_actions(mList, size);
  return size;
}

bool Board::genmove(Move &m) {
  MoveList mList;
  int size = 0, normal = 0, capture = 0, flip = 0;

  if (sideToMove == RED) {
    normal = legal_normal_actions<RED>(mList, 0);
    capture = legal_capture_actions<RED>(mList, normal);
  } else if (sideToMove == BLACK) {
    normal = legal_normal_actions<BLACK>(mList, 0);
    capture = legal_capture_actions<BLACK>(mList, normal);
  }
  
  flip = legal_flip_actions(mList, 0);
  
  size = flip;
  if (size == 0) return false;
  /*std::cout << "Legal moves\n";
  for (int i = 0; i < size; i++) {
    std::cout << i << " " << print_move(mList[i]) << std::endl;
  }*/

  std::uniform_int_distribution<size_t> distr(0, size - 1);
  size_t i = distr(rng);
  m = mList[i];

  //std::cout << print_board() << std::endl; 
  return true;
}

Color Board::side_to_move() const { return sideToMove; }

int Board::get_gameLength() const { return gameLength; }

std::minstd_rand Board::getRng() const { return rng; }

uint64_t Board::getHash() const { return hash_; }

void Board::update_status() {
  if (pieces(RED) == 0) {
    status_ = Status::BlackWin;
  } else if (pieces(BLACK) == 0) {
    status_ = Status::RedWin;
  }
}

void Board::update_basic_value(Color Us) {
  Color Op = ~Us;
  BV[get_piece(Us, PAWN)] = 1 + 4 * popCount(pieces(Op, KING)) + popCount(pieces(Op, PAWN));
  BV[get_piece(Us, KNIGHT)] = 1 + 4 * popCount(pieces(Op, PAWN)) + popCount(pieces(Op, CANNON, KNIGHT));
  BV[get_piece(Us, ROOK)] = 1 + 4 * popCount(pieces(Op, KNIGHT, PAWN)) + popCount(pieces(Op, ROOK, CANNON));
  BV[get_piece(Us, MINISTER)] = 1 + 4 * popCount(pieces(Op, ROOK, KNIGHT, PAWN)) + popCount(pieces(Op, MINISTER, CANNON));
  BV[get_piece(Us, GUARD)] = 1 + 4 * popCount(pieces(Op, MINISTER, ROOK, KNIGHT, PAWN)) + popCount(pieces(Op, GUARD, CANNON));
  BV[get_piece(Us, KING)] = 1 + 4 * popCount(pieces(Op, GUARD, MINISTER, ROOK, KNIGHT)) + popCount(pieces(Op, PAWN, CANNON, KING));
  BV[get_piece(Us, CANNON)] = 4 * popCount(pieces(Op)) + popCount(pieces(Op));
}

void Board::update_material_score(Color Us) {
  Color c = ~Us; // c = Opponent
  update_basic_value(c);
  score[Us] = 0;
  score[Us] += pieceCount[get_piece(Us, PAWN)] * PawnValueMg * (BV[get_piece(c, PAWN)] + BV[get_piece(c, KING)]);
  score[Us] += pieceCount[get_piece(Us, KNIGHT)] * KnightValueMg * (BV[get_piece(c, PAWN)] + BV[get_piece(c, CANNON)] + BV[get_piece(c, KNIGHT)]);
  score[Us] += pieceCount[get_piece(Us, ROOK)] * RookValueMg * (BV[get_piece(c, PAWN)] + BV[get_piece(c, CANNON)] + BV[get_piece(c, KNIGHT)] + BV[get_piece(c, ROOK)]);
  score[Us] += pieceCount[get_piece(Us, MINISTER)] * MinisterValueMg * (BV[get_piece(c, PAWN)] + BV[get_piece(c, CANNON)] + BV[get_piece(c, KNIGHT)] + BV[get_piece(c, ROOK)] + BV[get_piece(c, MINISTER)]);
  score[Us] += pieceCount[get_piece(Us, GUARD)] * GuardValueMg * (BV[get_piece(c, PAWN)] + BV[get_piece(c, CANNON)] + BV[get_piece(c, KNIGHT)] + BV[get_piece(c, ROOK)] + BV[get_piece(c, MINISTER)] + BV[get_piece(c, GUARD)]);
  score[Us] += pieceCount[get_piece(Us, KING)] * KingValue * (BV[get_piece(c, PAWN)] + BV[get_piece(c, CANNON)] + BV[get_piece(c, KNIGHT)] + BV[get_piece(c, ROOK)] + BV[get_piece(c, MINISTER)] + BV[get_piece(c, GUARD)] + BV[get_piece(c, KING)]);
  score[Us] += pieceCount[get_piece(Us, CANNON)] * CannonValueMg * (BV[get_piece(c, PAWN)] + BV[get_piece(c, CANNON)] + BV[get_piece(c, KNIGHT)] + BV[get_piece(c, ROOK)] + BV[get_piece(c, MINISTER)] + BV[get_piece(c, GUARD)] + BV[get_piece(c, KING)]);
}

bool Board::is_terminal() const {
  if (status_ == Status::RedWin || status_ == Status::BlackWin) {
    return true;
  }
  return false;
}

Color Board::who_won() const {
  if (status_ == Status::RedWin) return RED;
  else if (status_ == Status::BlackWin) return BLACK;
  return COLOR_NONE;
}

int Board::num_of_dark_pieces() const {
  return popCount(pieces(DARK));
}

int Board::get_score(Color c) const {
  return score[c];
}

/*
 * The evaluation function must be sensitive to the sideToMove
 * For a position with MAX node to move, return score
 * For a position with MIN node to move, return -score
 */
int Board::evaluate(Color Us) const {
  //std::cout << "Evaluate...\n";
  //std::cout << "sideToMove " << Us << std::endl;
  Color Op = ~Us;
  int score = get_score(Us) - get_score(Op);
  if (Us != sideToMove) {
    return -score;
  }
  //std::cout << "Score: " << score << std::endl;
  return score;
}

std::string Board::print_move(Move m) const {
  std::stringstream ss;
  char buff[6];

  Square from = from_sq(m);
  Square to = to_sq(m);
  sprintf(buff, "%c%c %c%c", file_of(from)+'a', rank_of(from)+'1', file_of(to)+'a', rank_of(to)+'1');
  ss << buff;

  return ss.str();
}

char Board::print_piece(Piece p) const {
  Color c = color_of(p);
  PieceType pt = type_of(p);
  char ans;
  switch (pt) {
    case KING       : ans = 'k'; break;
    case GUARD      : ans = 'g'; break;
    case MINISTER   : ans = 'm'; break;
    case ROOK       : ans = 'r'; break;
    case KNIGHT     : ans = 'n'; break;
    case CANNON     : ans = 'c'; break;
    case PAWN       : ans = 'p'; break;
    case DARK       : ans = 'X'; break;
    case EMPTY      : ans = ' '; break;
    default         : ans = ' '; break;
  }
  if (c == BLACK) return toupper(ans);
  return ans;
}

std::string Board::print_board() const {
  std::stringstream ss;
  Piece pc;
  ss << "Chinese Dark Chess Board\n";
  ss << "   a | b | c | d |\n";
  for (Rank r = RANK_8; r >= RANK_1; --r) {
    ss << r+1 << " ";
    for (File f = FILE_A; f < FILE_NB; ++f) {
      pc = board[make_square(f,r)];
      ss << " " << print_piece(pc) << " |";
    }
    ss << "\n";
  }
  ss << "\n";
  return ss.str();
}

} // namespace DarkChess