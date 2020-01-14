#pragma once

#include <chrono>
#include <ctime>

#include "state.h"

#define INF 999999999

namespace Search {

struct Location {
  int alpha, beta, score;
};

const int MAX_DEPTH = 4;
const int MIN_SCORE = 100;

extern DarkChess::Move _bestMove;
extern int _bestScore;
extern DarkChess::Color Us;

void iterDeep(DarkChess::Board initialBoard);
void rootMax(DarkChess::Board &board, int depth);
int negaMax(DarkChess::Board &board, int depth, int alpha, int beta);

} // namespace Search