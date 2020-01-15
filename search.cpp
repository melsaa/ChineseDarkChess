#include "search.h"

namespace Search {

DarkChess::Move _bestMove;
int _bestScore;
DarkChess::Color Us;
Trans::TranspTable tt;
std::chrono::time_point<std::chrono::system_clock> start, end;
std::chrono::duration<double> elapsed_seconds;

void iterDeep(DarkChess::Board initialBoard) {
  //for (int currDepth = 6; currDepth <= MAX_DEPTH; currDepth += 1) {
    int currDepth = 6;
    if (initialBoard.get_gameLength() > 50) {
      currDepth = 12;
    }
    start = std::chrono::system_clock::now(); 
    rootMax(initialBoard, currDepth);
    end = std::chrono::system_clock::now();
    elapsed_seconds = end - start;
    //std::cout << "bestScore " << _bestScore << " " << initialBoard.print_move(_bestMove) << std::endl;
    //std::cout << "Depth " << currDepth << " time: " << elapsed_seconds.count() << std::endl;
  //}
}

void rootMax(DarkChess::Board &board, int depth) {
  DarkChess::MoveList legalMoves;
  DarkChess::ScoreList scoreMoves;
  DarkChess::Piece captured;
  int size = board.get_legal_moves(legalMoves, scoreMoves);
  int flip = board.num_of_dark_pieces();

  // No legal moves available
  if (size == 0 && flip == 0) {
    _bestMove = DarkChess::MOVE_NULL; // not the best choice
    _bestScore = -INF;
    //std::cout << "LOSE no legal moves, bestScore = -INF\n";
    return;
  };

  int alpha = -INF;
  int beta = INF;
  int currScore;
  DarkChess::Move bestMove = DarkChess::MOVE_NULL;
  Us = board.side_to_move();
  //std::cout << size << " legalMoves\n";
  for (int i = 0; i < size; i++) {
    Board temp = board;
    temp.do_move(legalMoves[i], captured);
    
    currScore = -negaScout(temp, depth - 1, -beta, -alpha);
    //std::cout << i << " " << board.print_move(legalMoves[i]) << " score " << currScore << std::endl;
    //board.undo_move(legalMoves[i], captured);
    if (currScore > alpha) {
      bestMove = legalMoves[i];
      alpha = currScore;
    }
    if (alpha >= beta) break;
  }
  currScore = board.evaluate(Us);
  if (flip > 0 && (alpha <= currScore || size == 0)) {
    DarkChess::MoveList mList;
    int fsize = board.legal_flip_actions(mList, 0);
    bestMove = mList[rand() % fsize];
    for (int i = 0; i < fsize; i++) {
      int v = 0, n = 0;
      for (Piece p = R_PAWN; p <= B_KING; ++p) {
        if (board.is_dark(from_sq(mList[i]))) {
          Board temp = board;
          temp.flip_move(mList[i], p, color_of(p));
          v = v - negaScout(board, depth - 1, -beta, -alpha);
          //board.undo_flip(mList[i], p, color_of(p));
          n += temp.get_pieceCount(p);
        }
      }
      if (v/n > currScore) {
        currScore = v/n;
      }
      if (currScore > beta) {
        bestMove = mList[i];
        break;
      }
    }
    
    alpha = currScore;
  }

  // If the best move was not set in the main search loop
  // just pick the first move available
  if (bestMove == DarkChess::MOVE_NULL && size > 0) {
    bestMove = legalMoves[0];
  }

  // << "bestScore = alpha " << alpha << std::endl;
  _bestMove = bestMove;
  _bestScore = alpha;
}

int negaScout(DarkChess::Board &board, int depth, int alpha, int beta) {
  int score;
  int alphaOrig = alpha;
  DarkChess::Piece captured;
  end = std::chrono::system_clock::now();
  elapsed_seconds = end - start;

  if (depth == 0 || elapsed_seconds.count() >= 6) {
    score = board.evaluate(Us);
    //std::cout << "depth 0 return evaluate\n";
    return score;
  }

  // Check for threefold repetition draws
  if (board.getRepetition() >= 9) {
    return 0;
  }

  // Check for 60 moves draws
  if (board.getNoCFMoves() >= 60) {
    return 0;
  }

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
  DarkChess::ScoreList scoreMoves;
  int size = board.get_legal_moves(legalMoves, scoreMoves);
  move_ordering(legalMoves, scoreMoves, size);
  int flip = board.num_of_dark_pieces();
  /*std::cout << depth << " sideToPlay " << board.side_to_move() << std::endl;
  std::cout << board.print_board() << std::endl;
  std::cout << "legalMoves " << size << std::endl;*/

  if (size == 0 && flip == 0) {
    board.update_status(size);
    // INF = win, -INF = lose
    if (board.who_won() == board.side_to_move()) {
      score = INF;
    } else if (board.who_won() == (~board.side_to_move())) {
      score = -INF;
    } else {
      std::cout << "COLOR_NONE\n";
    }
    //std::cout << "no legal moves return game score " << score << std::endl;
    return score;
  } else if (size == 0 && flip > 0) {
    //std::cout << "only flip moves return evaluate\n";
    return board.evaluate(Us);
  }

  // TODO: extend search if king is in danger
  // TODO: quiescent Search if depth is 0

  DarkChess::Move bestMove = DarkChess::MOVE_NULL;

  for (int i = 0; i < size; i++) {
    Board temp = board;
    temp.do_move(legalMoves[i], captured);
    
    score = -negaScout(temp, depth - 1, -beta, -alpha);
    //board.undo_move(legalMoves[i], captured);
    if (score >= beta) {
      //std::cout << "beta cut off " << beta << std::endl;
      return beta; // beta cut-off
    }
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

  //std::cout << "return alpha " << alpha << std::endl;
  return alpha;
}

} // namespace Search