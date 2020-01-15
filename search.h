#pragma once

#include <chrono>
#include <ctime>

#include "state.h"
#include "tt.h"

#define INF 2147483647

namespace Search {

struct Location {
  int alpha, beta, score;
};

const int MAX_DEPTH = 4;
const int MIN_SCORE = 100;

extern DarkChess::Move _bestMove;
extern int _bestScore;
extern DarkChess::Color Us;
extern Trans::TranspTable tt;

void iterDeep(DarkChess::Board initialBoard);
void rootMax(DarkChess::Board &board, int depth);
int negaMax(DarkChess::Board &board, int depth, int alpha, int beta);

} // namespace Search