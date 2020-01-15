#include "search.h"

namespace Search {

DarkChess::Move _bestMove;
int _bestScore;
DarkChess::Color Us;
Trans::TranspTable tt;
std::chrono::time_point<std::chrono::system_clock> start, end;
std::chrono::duration<double> elapsed_seconds;

void iterDeep(DarkChess::Board initialBoard) {
  for (int currDepth = 4; currDepth <= MAX_DEPTH; currDepth += 1) {
    start = std::chrono::system_clock::now(); 
    rootMax(initialBoard, currDepth);
    end = std::chrono::system_clock::now();
    elapsed_seconds = end - start;
    std::cout << "bestScore " << _bestScore << " " << initialBoard.print_move(_bestMove) << std::endl;
    std::cout << "Depth " << currDepth << " time: " << elapsed_seconds.count() << std::endl;
  }
}
/*
int exhaustive(DarkChess::Board &board, int depth_limit) {
  DarkChess::MoveList legalMoves;
  DarkChess::Piece captured;
  int c = board.evaluate();
  int size = board.get_legal_moves(legalMoves);
  int flip = board.num_of_dark_pieces();

  if (size == 0 && flip == 0) {
    return c;
  }
  int s = alpha_beta(board, depth_limit, alpha, beta);
  if (s <= c && flip > 0) {
    for (int)
  }
}*/

void rootMax(DarkChess::Board &board, int depth) {
  DarkChess::MoveList legalMoves;
  DarkChess::Piece captured;
  int size = board.get_legal_moves(legalMoves);
  int flip = board.num_of_dark_pieces();

  // No legal moves available
  if (size == 0 && flip == 0) {
    _bestMove = DarkChess::MOVE_NULL; // not the best choice
    _bestScore = -INF;
    std::cout << "LOSE no legal moves, bestScore = -INF\n";
    return;
  };

  int alpha = -INF;
  int beta = INF;
  int currScore;
  DarkChess::Move bestMove = DarkChess::MOVE_NULL;
  Us = board.side_to_move();

  for (int i = 0; i < size; i++) {
    board.do_move(legalMoves[i], captured);
    
    currScore = -negaMax(board, depth - 1, -beta, -alpha);
    board.undo_move(legalMoves[i], captured);
    if (currScore > alpha) {
      bestMove = legalMoves[i];
      alpha = currScore;
    }
    if (alpha >= beta) break;
  }
  currScore = board.evaluate(Us);
  if (flip > 0 && alpha <= currScore) {
    DarkChess::MoveList mList;
    int size = board.legal_flip_actions(mList, 0);
    bestMove = mList[rand() % size];
    alpha = currScore;
    // TODO: choose best flipping action
  }

  // If the best move was not set in the main search loop
  // just pick the first move available
  if (bestMove == DarkChess::MOVE_NULL) {
    bestMove = legalMoves[0];
  }

  std::cout << "bestScore = alpha " << alpha << std::endl;
  _bestMove = bestMove;
  _bestScore = alpha;
}

// Negamax with alpha-beta cut-off
int negaMax(DarkChess::Board &board, int depth, int alpha, int beta) {
  int score;
  int alphaOrig = alpha;
  DarkChess::Piece captured;
  end = std::chrono::system_clock::now();
  elapsed_seconds = end - start;

  if (depth == 0 || elapsed_seconds.count() >= 6) {
    score = board.evaluate(Us);
    std::cout << "depth 0 return evaluate\n";
    return score;
  }

  // TODO: Check for threefold repetition draws
  // TODO: Check for no flip or capture move draws (50 moves)

  const Trans::TTEntry *ttEntry = tt.getEntry(board.getHash());
  // Check transposition table cache
  if (ttEntry && (ttEntry->depth >= depth)) {
    switch(ttEntry->flag) {
      case Trans::EXACT: return ttEntry->score;
      case Trans::UPPER_BOUND: beta = std::min(beta, ttEntry->score);
                        break;
      case Trans::LOWER_BOUND: alpha = std::max(alpha, ttEntry->score);
                        break;
    }
    if (alpha >= beta) {
      return ttEntry->score;
    }
  }

  //int m = alpha;
  DarkChess::MoveList legalMoves;
  int size = board.get_legal_moves(legalMoves);
  int flip = board.num_of_dark_pieces();

  if (size == 0 && flip == 0) {
    board.update_status();
    // INF = win, -INF = lose
    score = board.who_won() == board.side_to_move() ? INF : -INF;
    std::cout << "no legal moves return game score\n";
    return score;
  } else if (size == 0 && flip > 0) {
    std::cout << "only flip moves return evaluate\n";
    return board.evaluate(Us);
  }

  // TODO: extend search if king is in danger
  // TODO: quiescent Search if depth is 0

  DarkChess::Move bestMove = DarkChess::MOVE_NULL;

  for (int i = 0; i < size; i++) {
    board.do_move(legalMoves[i], captured);
    
    score = -negaMax(board, depth - 1, -beta, -alpha);
    board.undo_move(legalMoves[i], captured);
    if (score >= beta) return beta; // beta cut-off
    if (score > alpha) {
      bestMove = legalMoves[i];
      alpha = score;
    }
  }

  if (bestMove == DarkChess::MOVE_NULL) {
    bestMove = legalMoves[0];
  }
  // Store bestScore in transposition table
  Trans::Flag _flag;
  if (alpha <= alphaOrig) {
    _flag = Trans::UPPER_BOUND;
  } else {
    _flag = Trans::EXACT;
  }
  Trans::TTEntry newTTEntry(alpha, depth, bestMove, _flag);
  tt.set(board.getHash(), newTTEntry);

  std::cout << "return alpha\n";
  return alpha;
}

} // namespace Search