#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <iostream>

#include "types.h"
#include "state.h"

using namespace DarkChess;

#define COMMAND_NUM 18

class Engine {
  const char* commands_name[COMMAND_NUM] = {
		"protocol_version",
		"name",
		"version",
		"known_command",
		"list_commands",
		"quit",
		"boardsize",
		"reset_board",
		"num_repetition",
		"num_moves_to_draw",
		"move",
		"flip",
		"genmove",
		"game_over",
		"ready",
		"time_settings",
		"time_left",
  	"showboard"
	};

  public:
    Engine();
    ~Engine();
    // commands
    bool protocol_version(const char* data[], char* response);// 0
    bool name(const char* data[], char* response);// 1
    bool version(const char* data[], char* response);// 2
    bool known_command(const char* data[], char* response);// 3
    bool list_commands(const char* data[], char* response);// 4
    bool quit(const char* data[], char* response);// 5
    bool boardsize(const char* data[], char* response);// 6
    bool reset_board(const char* data[], char* response);// 7
    bool num_repetition(const char* data[], char* response);// 8
    bool num_moves_to_draw(const char* data[], char* response);// 9
    bool move(const char* data[], char* response);// 10
    bool flip(const char* data[], char* response);// 11
    bool genmove(const char* data[], char* response);// 12
    bool game_over(const char* data[], char* response);// 13
    bool ready(const char* data[], char* response);// 14
    bool time_settings(const char* data[], char* response);// 15
    bool time_left(const char* data[], char* response);// 16
    bool showboard(const char* data[], char* response);// 17
  
  private:
    Board board;
    
    PieceType strToPieceType(const char in) {
      switch (in) {
        case 'k': return KING;
        case 'g': return GUARD;
        case 'm': return MINISTER;
        case 'r': return ROOK;
        case 'n': return KNIGHT;
        case 'c': return CANNON;
        case 'p': return PAWN;
        default: return EMPTY;
      }
    }

    Square toSquare(const char in[2]) {
      File f = File(tolower(in[0]) - 'a');
      Rank r = Rank(in[1] - '1');
      return make_square(f, r); // File f, Rank r
    }

    Piece toPiece(const char* in, Color &c) {
      c = islower(in[0]) ? RED : BLACK;
      PieceType pt = strToPieceType(tolower(in[0]));
      return make_piece(c, pt); // Color c, PieceType pt
    }
};