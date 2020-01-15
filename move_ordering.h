#pragma once

#include "types.h"

using namespace DarkChess;

extern int sq_distance[SQUARE_NB][SQUARE_NB];

void move_ordering(MoveList &legalMoves, ScoreList scoreMoves, int size);
void init_distance();