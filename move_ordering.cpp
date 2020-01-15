#include "move_ordering.h"

int sq_distance[SQUARE_NB][SQUARE_NB];

void move_ordering(MoveList &legalMoves, ScoreList scoreMoves, int size) {
  int temp;
  Move mTemp;
  for (int i = 0; i < size; i++) {
    for (int j = i + 1; j < size; j++) {
      if (scoreMoves[j] > scoreMoves[i]) {
        temp = scoreMoves[j];
        mTemp = legalMoves[j];
        scoreMoves[j] = scoreMoves[i];
        legalMoves[j] = legalMoves[i];
        scoreMoves[i] = temp;
        legalMoves[i] = mTemp;
      }
    }
  }
}

void init_distance() {
  File f1, f2;
  Rank r1, r2;
  for (Square s1 = SQ_A1; s1 < SQUARE_NB; ++s1) {
    for (Square s2 = SQ_A1; s2 < SQUARE_NB; ++s2) {
      f1 = file_of(s1);
      r1 = rank_of(s1);
      f2 = file_of(s2);
      r2 = rank_of(s2);
      sq_distance[s1][s2] = abs(f1 - f2) + abs(r1 - r2);
    }
  }
}