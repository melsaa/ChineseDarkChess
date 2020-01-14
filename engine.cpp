#include "engine.h"

Engine::Engine() {}

Engine::~Engine() {}

bool Engine::protocol_version(const char* data[], char* response) {
  strcpy(response, "1.0.0");
  return 0;
}

bool Engine::name(const char* data[], char* response) {
  strcpy(response, "NegaScout");
  return 0;
}

bool Engine::version(const char* data[], char* response) {
  strcpy(response, "1.0.0");
  return 0;
}

bool Engine::known_command(const char* data[], char* response) {
  for (int i = 0; i < COMMAND_NUM; i++) {
    if (!strcmp(data[0], commands_name[i])) {
      strcpy(response, "true");
      return 0;
    }
  }
  strcpy(response, "false");
  return 0;
}

bool Engine::list_commands(const char* data[], char* response) {
  for (int i = 0; i < COMMAND_NUM; i++) {
    strcat(response, commands_name[i]);
    if (i < COMMAND_NUM - 1) {
      strcat(response, "\n");
    }
  }
  return 0;
}

bool Engine::quit(const char* data[], char* response) {
  fprintf(stderr, "Bye\n");
  return 0;
}

bool Engine::boardsize(const char* data[], char* response) {
  fprintf(stderr, "BoardSize: %s x %s\n", data[0], data[1]);
  return 0;
}

bool Engine::reset_board(const char* data[], char* response) {
  board.init();
  // TODO: time
  return 0;
}

bool Engine::num_repetition(const char* data[], char* response) {
  return 0;
}

bool Engine::num_moves_to_draw(const char* data[], char* response) {
  return 0;
}

bool Engine::move(const char* data[], char* response) {
  Square s1 = toSquare(data[0]);
  Square s2 = toSquare(data[1]);
  Move m = make_move(s1, s2);
  Piece captured;
  board.do_move(m, captured);
  std::cout << board.print_board() << std::endl;
  return 0;
}

bool Engine::flip(const char* data[], char* response) {
  Color c;
  Piece p = toPiece(data[1], c);
  Square s = toSquare(data[0]);
  Move m = make_move(s, s);
  board.flip_move(m, p, c);
  std::cout << board.print_board() << std::endl;
  return 0;
}

bool Engine::genmove(const char* data[], char* response) {
  Move m;
  //if (board.genmove(m)) {
  Search::iterDeep(board);
  if (Search::_bestMove != MOVE_NULL) {
    m = Search::_bestMove;
    strcpy(response, board.print_move(m).c_str());
  } else {
    strcpy(response, "no legal moves");
  }
  std::cout << board.print_board() << std::endl;
  return 0;
}

bool Engine::searchMove(Move &m) {
  Search::iterDeep(board);
  MoveList mList;
  if (Search::_bestMove == MOVE_NULL || Search::_bestScore < Search::MIN_SCORE) {
    std::cout << "bestMove = NULL or bestScore < 100 " << Search::_bestScore << std::endl;
    int size = board.legal_flip_actions(mList, 0);

    if (size == 0) return false;

    //std::uniform_int_distribution<size_t> distr(0, size - 1);
    //size_t i = distr(board.getRng());
    m = mList[rand() % size];
  } else {
    m = Search::_bestMove;
  }
  return true;
}

bool Engine::game_over(const char* data[], char* response) {
  fprintf(stderr, "Game Results: %s\n", data[0]);
  return 0;
}

bool Engine::ready(const char* data[], char* response) {
  return 0;
}

bool Engine::time_settings(const char* data[], char* response) {
  return 0;
}

bool Engine::time_left(const char* data[], char* response) {
  /*if (!strcmp(data[0], "red")) {
    sscanf(data[1], "%d", &Red_Time);
  } else {
    sscanf(data[1], "%d", &Black_Time);
  }*/
  fprintf(stderr, "Time Left(%s): %s\n", data[0], data[1]);
  return 0;
}

bool Engine::showboard(const char* data[], char* response) {
  std::cout << board.print_board() << std::endl;
  return 0;
}