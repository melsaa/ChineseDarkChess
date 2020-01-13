#include "state.h"

namespace DarkChess {

Board::Board(int seed) : rng(seed) {
  // Initialize magic bitboard
  initCannonMasks();
  initCannonMagicTable();
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

// TODO: Check the correctness of pMoves table
template <Color Us>
int Board::legal_normal_actions(MoveList &mL, int idx) {
  Bitboard dest;
  
  std::cout << std::bitset<32>(pieces(ALL_PIECES)) << std::endl;
  std::cout << std::bitset<32>(~pieces(ALL_PIECES)) << std::endl;
  for (PieceType p = PAWN; p <= KING; ++p) {
    Bitboard b = pieces(Us, p);
    while (b) {
      Bitboard mask = LS1B(b);
      b ^= mask;
      Square src = popLsb(mask);
      assert(type_of(board[src]) == p);
      dest = pMoves[src] & (~pieces(ALL_PIECES));
      std::cout << src << " " << std::bitset<32>(dest) << std::endl;

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
        dest = getCannonAttack(src, pieces(ALL_PIECES)) & pieces(Op);
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
      gameLength++;
      std::cout << print_board() << std::endl;
    } else {
      std::cerr << "Move not ok\n";
    }
  } else {
    std::cerr << "MOVE PASS\n";
  }
}

void Board::do_move(Move m) {
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
    Piece captured = piece_on(to);
    
    if (color_of(pc) != us) {
      std::cerr << color_of(pc) << std::endl;
      assert(color_of(pc) != COLOR_NONE);
      assert(color_of(pc) == us);
    }
    
    if (captured != NO_PIECE) {
      Square capsq = to;
      remove_piece(captured, capsq);
      // k ^= Zobrist::psq[captured][capsq];
    }
    // k ^= Zobrist::psq[pc][from] ^ Zobrist::psq[pc][to];
    move_piece(pc, from, to);
  }
  sideToMove = ~sideToMove;
  gameLength++;
  std::cout << print_board() << std::endl; 
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

  flip = legal_flip_actions(mList, capture);
  size = flip;
  assert(size > 0);
  if (size == 0) return false;
  std::cout << "Legal moves\n";
  for (int i = 0; i < size; i++) {
    std::cout << i << " " << print_move(mList[i]) << std::endl;
  }

  std::uniform_int_distribution<size_t> distr(0, size - 1);
  size_t i = distr(rng);
  m = mList[i];
  
  std::cout << print_board() << std::endl; 
  return true;
}

Color Board::side_to_move() const { return sideToMove; }

int Board::get_gameLength() const { return gameLength; }

void Board::update_status() {
  if (pieces(RED) == 0) {
    status_ = Status::BlackWin;
  } else if (pieces(BLACK) == 0) {
    status_ = Status::RedWin;
  }
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