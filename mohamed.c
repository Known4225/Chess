#define UNITYPE_LIST_IMPLEMENTATION
#include "turtle.h"
#include <time.h>

#include "book.h"

// defines
#define MAX_LEGAL_MOVES 218
#define GAME_STATE_SIZE 66
#define DEBUG 0

// global variables
char legal_moves[MAX_LEGAL_MOVES][GAME_STATE_SIZE] = {0};
char current_state[GAME_STATE_SIZE] = {0};
char final_state[GAME_STATE_SIZE] = {0};

char starting_state[GAME_STATE_SIZE] = {'w',
                                        'H', 'C', 'D', 'E', 'I', 'D', 'C', 'H',
                                        'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A',
                                        '0', '0', '0', '0', '0', '0', '0', '0',
                                        '0', '0', '0', '0', '0', '0', '0', '0',
                                        '0', '0', '0', '0', '0', '0', '0', '0',
                                        '0', '0', '0', '0', '0', '0', '0', '0',
                                        '1', '1', '1', '1', '1', '1', '1', '1',
                                        '8', '3', '4', '5', '9', '4', '3', '8'};

double get_state_value(char *state);
double get_state_piece_value(char *state);
double get_state_vulnerability_value(char *state);
double get_state_control_value(char *state);

void set_final_state(char *state, char *file_name);
void get_best_move(char *current_state, char legal_moves[][GAME_STATE_SIZE], int legal_state_count, char *final_state);

bool is_starting_position(char *state);

int main(int argc, char *argv[]) {
  char state[256];
  int legal_state_count = 0;

  if (argc < 3) {
    fprintf(stderr, "Usage: <input_file> <output_file>\n");
    return 1;
  }

  FILE *input_file = fopen(argv[1], "r");
  if (input_file == NULL) {
    perror("Error opening file");
    return 1;
  }

  if (fgets(state, sizeof(state), input_file) == NULL) {
    perror("Error reading from file");
    fclose(input_file);
    return 1;
  }
  
  size_t len = strnlen(state, GAME_STATE_SIZE - 1);
  memcpy(current_state, state, len);
  current_state[len] = '\0';

  if (is_starting_position(current_state)) {
    memcpy(final_state, queens_gambit, GAME_STATE_SIZE);
    set_final_state(final_state, argv[2]);
    fclose(input_file);
    return 0;
  }

  memcpy(final_state, current_state, GAME_STATE_SIZE);
  
  while (fgets(state, sizeof(state), input_file) != NULL) {
    size_t len = strnlen(state, GAME_STATE_SIZE - 1);
    memcpy(legal_moves[legal_state_count], state, len);
    legal_moves[legal_state_count][len] = '\0';
    legal_state_count++;
  }

  if (legal_state_count > 0) {
    get_best_move(current_state, legal_moves, legal_state_count, final_state);
  } else {
    printf("No legal moves found.\n");
  }

  set_final_state(final_state, argv[2]);
  fclose(input_file);
  return 0;
}

void get_best_move(char *current_state, char legal_moves[][GAME_STATE_SIZE], int legal_state_count, char *final_state) {
  char state[256];
  char state_second[256];
  int8_t turn = CHESS_WHITE;
  double best_average_value = 0;
  if (current_state[0] == 'b') {
    turn = CHESS_BLACK;
  }
#if DEBUG
  printf("Current state: %s\n", current_state);
  printf("Available legal moves: %d\n", legal_state_count);
  printf("Legal moves:\n");
  for (int i = 0; i < legal_state_count; i++) {
    printf("%s\n", legal_moves[i]);
  }
#endif
  for (int i = 0; i < legal_state_count; i++) {
    memcpy(state, legal_moves[i], GAME_STATE_SIZE);
    double state_value = get_state_value(state);
    int8_t next_turn = CHESS_WHITE;
    if (state[0] == 'b') {
      next_turn = CHESS_BLACK;
    }
    list_t *moves = generateAllMoves(state, turn);

    double average_value = 0;
    double best_value = 0;
    if (moves->length > 0) {
      for (int32_t movesIndex = 0; movesIndex < moves->length; movesIndex += MOVES_NUMBER_OF_FIELDS) {
        memcpy(state_second, moves->data[movesIndex + MOVES_STRING].s, GAME_STATE_SIZE);
        double state_value_second = get_state_value(state_second);
        average_value += state_value_second;
        if (next_turn == CHESS_WHITE) {
          if (state_value_second > best_value) {
            best_value = state_value_second;
          }
        } else {
          if (state_value_second < best_value) {
            best_value = state_value_second;
          }
        }
      }
      average_value /= (moves->length / MOVES_NUMBER_OF_FIELDS);
    }
    list_free(moves);

#if DEBUG
    printf("State value for move %s: %f\n", state, state_value);
#endif

    if (turn == CHESS_WHITE && state_value >= get_state_value(final_state)) {
      if (average_value < best_average_value || best_average_value == 0) {
        best_average_value = average_value;
        memcpy(final_state, state, GAME_STATE_SIZE);
      }
    } else if (turn == CHESS_BLACK && state_value <= get_state_value(final_state)) {
      if (average_value > best_average_value || best_average_value == 0) {
        best_average_value = average_value;
        memcpy(final_state, state, GAME_STATE_SIZE);
      }
    }
  }
}


#warning "continue looking ahead"
#if DEBUG
    printf("State value for move %s: %f\n", state, state_value);
#endif
    if (turn == CHESS_WHITE && state_value >= get_state_value(final_state)) {
#if DEBUG
      printf("Updating final state to: %s with value: %f\n", state, state_value);
#endif
      memcpy(final_state, state, GAME_STATE_SIZE);
    } else if (turn == CHESS_BLACK && state_value <= get_state_value(final_state)) {
#if DEBUG
      printf("Updating final state to: %s with value: %f\n", state, state_value);
#endif
      memcpy(final_state, state, GAME_STATE_SIZE);
    }
  }
}

double get_state_value(char *state) {
  double piece_value = get_state_piece_value(state);
  double control_value = get_state_control_value(state);

  return piece_value + control_value;
}

double get_square_value(char square) {
  switch (square) {
    case WHITE_PAWN:
    case WHITE_PAWN_EN_PASSANT:
      return 1.0;
    break;
    case WHITE_ROOK:
    case WHITE_ROOK_NO_MOVE:
      return 5.0;
    break;
    case WHITE_KNIGHT:
      return 3.0;
    break;
    case WHITE_BISHOP:
      return 3.0;
    break;
    case WHITE_QUEEN:
      return 9.0;
    break;
    case WHITE_KING:
    case WHITE_KING_NO_MOVE:
      //return 1000; // high value for white king
      return 0.0;
    break;
    case BLACK_PAWN:
    case BLACK_PAWN_EN_PASSANT:
      return -1.0;
    break;
    case BLACK_ROOK:
    case BLACK_ROOK_NO_MOVE:
      return -5.0;
    break;
    case BLACK_KNIGHT:
      return -3.0;
    break;
    case BLACK_BISHOP:
      return -3.0;
    break;
    case BLACK_QUEEN:
      return -9.0;
    break;
    case BLACK_KING:
    case BLACK_KING_NO_MOVE:
      //return -1000; // high negative value for black king
      return 0.0;
    break;
    case BLANK_SPACE:
    default:
      return 0.0; // empty square or unrecognized piece
  }
}

double get_state_piece_value(char *state) {
  double value = 0.0;
  for (int i = 1; i < GAME_STATE_SIZE - 1; i++) {
    value += get_square_value(state[i]);
  }

  return value;
}

double get_state_control_value(char *state) {
  double value = 0.0;
  for (int i = 1; i < GAME_STATE_SIZE - 1; i++) {
    if ((i >= 27 && i <= 28) || (i >= 35 && i <= 36)) {
      value += get_square_value(state[i]) * 0.9;
    }
    if ((i >= 25 && i <= 30) || (i >= 33 && i <= 38)) {
      value += get_square_value(state[i]) * 0.5;
    }
    if ((i >= 24 && i <= 39)) {
      value += get_square_value(state[i]) * 0.2;
    }
    if (i >= 16 && i <= 47) {
      value += get_square_value(state[i]) * 0.1;
    }
  }

  return value;
}

double get_state_vulnerability_value(char *state) {
  // placeholder
  return 0;
}

bool is_starting_position(char *state) {
  return (memcmp(state, starting_state, GAME_STATE_SIZE) == 0);
}

void set_final_state(char *state, char *file_name) {
  FILE *output_file = fopen(file_name, "w");
  if (output_file == NULL) {
    perror("Error opening output file");
    return;
  } else{
#if DEBUG
    printf("Final state: %s\n", state);
#endif
    fputs(state, output_file);
  }
  
  fclose(output_file);
}

/* from chess.c */

/* generate all moves and augmented board positions, returns a list in the MOVES_X format */
list_t *generateAllMoves(char *board, int8_t turn) {
  /* initialisation */
  list_t *moves = list_init();
  /* computation */
  for (int32_t position = 0; position < 64; position++) {
      if (board[position] == BLANK_SPACE) {
          continue;
      }
      list_t *naive = generateNaiveMoves(board, position, turn);
      char boardCopy[66];
      boardCopy[65] = '\0';
      for (int32_t i = 0; i < naive -> length; i++) {
          if (turn == CHESS_WHITE) {
              boardCopy[0] = 'b';
          } else {
              boardCopy[0] = 'w';
          }
          memcpy(boardCopy + 1, board, 64);
          int32_t move = movePiece(boardCopy + 1, position, naive -> data[i].c);
          /* check results */
          if (move == MOVE_PIECE_PAWN_PROMOTION_WHITE) {
              char promotionOptions[4] = {WHITE_QUEEN, WHITE_KNIGHT, WHITE_ROOK, WHITE_BISHOP};
              for (int32_t j = 0; j < 4; j++) {
                  boardCopy[naive -> data[i].c + 1] = promotionOptions[j];
                  list_append(moves, (unitype) boardCopy, 's'); // MOVES_STRING
                  list_append(moves, (unitype) position, 'c'); // MOVES_FROM
                  list_append(moves, naive -> data[i], 'c'); // MOVES_TO
                  list_append(moves, (unitype) NULL, 'l'); // MOVES_EXTRA_CHECKS
              }
          } else if (move == MOVE_PIECE_PAWN_PROMOTION_BLACK) {
              char promotionOptions[4] = {BLACK_QUEEN, BLACK_KNIGHT, BLACK_ROOK, BLACK_BISHOP};
              for (int32_t j = 0; j < 4; j++) {
                  boardCopy[naive -> data[i].c + 1] = promotionOptions[j];
                  list_append(moves, (unitype) boardCopy, 's'); // MOVES_STRING
                  list_append(moves, (unitype) position, 'c'); // MOVES_FROM
                  list_append(moves, naive -> data[i], 'c'); // MOVES_TO
                  list_append(moves, (unitype) NULL, 'l'); // MOVES_EXTRA_CHECKS
              }
          } else if (move == MOVE_PIECE_CASTLE_WHITE) {
              list_append(moves, (unitype) boardCopy, 's'); // MOVES_STRING
              list_append(moves, (unitype) position, 'c'); // MOVES_FROM
              list_append(moves, naive -> data[i], 'c'); // MOVES_TO
              list_t *extraChecks = list_init();
              if (naive -> data[i].c == 62) {
                  /* white right castle */
                  boardCopy[63 + 1] = WHITE_ROOK; // revert rook
                  boardCopy[62 + 1] = BLANK_SPACE; // revert king
                  boardCopy[61 + 1] = WHITE_KING; // king walk
                  list_append(extraChecks, (unitype) boardCopy, 's');
              } else {
                  /* white left castle */
                  boardCopy[56 + 1] = WHITE_ROOK; // revert rook
                  boardCopy[57 + 1] = BLANK_SPACE; // revert king
                  boardCopy[59 + 1] = WHITE_KING; // king walk
                  list_append(extraChecks, (unitype) boardCopy, 's');
                  boardCopy[59 + 1] = BLANK_SPACE; // revert king
                  boardCopy[58 + 1] = WHITE_KING; // king walk
                  list_append(extraChecks, (unitype) boardCopy, 's');
              }
              list_append(moves, (unitype) extraChecks, 'r'); // MOVES_EXTRA_CHECKS
          } else if (move == MOVE_PIECE_CASTLE_BLACK) {
              list_append(moves, (unitype) boardCopy, 's'); // MOVES_STRING
              list_append(moves, (unitype) position, 'c'); // MOVES_FROM
              list_append(moves, naive -> data[i], 'c'); // MOVES_TO
              list_t *extraChecks = list_init();
              if (naive -> data[i].c == 6) {
                  /* black right castle */
                  boardCopy[7 + 1] = WHITE_ROOK; // revert rook
                  boardCopy[6 + 1] = BLANK_SPACE; // revert king
                  boardCopy[5 + 1] = WHITE_KING; // king walk
                  list_append(extraChecks, (unitype) boardCopy, 's');
              } else {
                  /* black left castle */
                  boardCopy[0 + 1] = WHITE_ROOK; // revert rook
                  boardCopy[1 + 1] = BLANK_SPACE; // revert king
                  boardCopy[3 + 1] = WHITE_KING; // king walk
                  list_append(extraChecks, (unitype) boardCopy, 's');
                  boardCopy[3 + 1] = BLANK_SPACE; // revert king
                  boardCopy[2 + 1] = WHITE_KING; // king walk
                  list_append(extraChecks, (unitype) boardCopy, 's');
              }
              list_append(moves, (unitype) extraChecks, 'r'); // MOVES_EXTRA_CHECKS
          } else if (move == MOVE_PIECE_SUCCESSFUL) {
              list_append(moves, (unitype) boardCopy, 's'); // MOVES_STRING
              list_append(moves, (unitype) position, 'c'); // MOVES_FROM
              list_append(moves, naive -> data[i], 'c'); // MOVES_TO
              list_append(moves, (unitype) NULL, 'l'); // MOVES_EXTRA_CHECKS
          } else {
              printf("ERROR: movePiece returned MOVE_PIECE_ERROR\n");
          }
      }
      list_free(naive);
  }
  /* second pass - validate legal moves are legal (don't put king in check, etc) */
  for (int32_t movesIndex = 0; movesIndex < moves -> length; movesIndex += MOVES_NUMBER_OF_FIELDS) {
      for (int32_t position = 0; position < 64; position++) {
          if (moves -> data[movesIndex + MOVES_STRING].s[position + 1] == BLANK_SPACE) {
              continue;
          }
          list_t *simulated = generateNaiveMoves(moves -> data[movesIndex + MOVES_STRING].s + 1, position, !turn);
          for (int32_t i = 0; i < simulated -> length; i++) {
              char capturedPiece = moves -> data[movesIndex + MOVES_STRING].s[simulated -> data[i].c + 1];
              if ((turn == CHESS_WHITE && (capturedPiece == WHITE_KING || capturedPiece == WHITE_KING_NO_MOVE)) || (turn == CHESS_BLACK && (capturedPiece == BLACK_KING || capturedPiece == BLACK_KING_NO_MOVE))) {
                  /* this moves captures the king, invalidate the legal move */
                  for (int32_t j = 0; j < MOVES_NUMBER_OF_FIELDS; j++) {
                      list_delete(moves, movesIndex);
                  }
                  movesIndex -= MOVES_NUMBER_OF_FIELDS;
                  list_free(simulated);
                  goto SECOND_PASS_NEXT;
              }
          }
          list_free(simulated);
      }
      /* extra moves to validate - castling cannot have the king "walk" through check */
      if (moves -> data[movesIndex + MOVES_EXTRA_CHECKS].r != NULL) {
          for (int32_t extra = 0; extra < moves -> data[movesIndex + MOVES_EXTRA_CHECKS].r -> length; extra++) {
              for (int32_t position = 0; position < 64; position++) {
                  if (moves -> data[movesIndex + MOVES_EXTRA_CHECKS].r -> data[extra].s[position + 1] == BLANK_SPACE) {
                      continue;
                  }
                  list_t *simulated = generateNaiveMoves(moves -> data[movesIndex + MOVES_EXTRA_CHECKS].r -> data[extra].s + 1, position, !turn);
                  for (int32_t i = 0; i < simulated -> length; i++) {
                      char capturedPiece = moves -> data[movesIndex + MOVES_EXTRA_CHECKS].r -> data[extra].s[simulated -> data[i].c + 1];
                      if ((turn == CHESS_WHITE && (capturedPiece == WHITE_KING || capturedPiece == WHITE_KING_NO_MOVE)) || (turn == CHESS_BLACK && (capturedPiece == BLACK_KING || capturedPiece == BLACK_KING_NO_MOVE))) {
                          /* this moves captures the king, invalidate the legal move */
                          for (int32_t j = 0; j < MOVES_NUMBER_OF_FIELDS; j++) {
                              list_delete(moves, movesIndex);
                          }
                          movesIndex -= MOVES_NUMBER_OF_FIELDS;
                          list_free(simulated);
                          goto SECOND_PASS_NEXT;
                      }
                  }
                  list_free(simulated);
              }
          }
      }
      SECOND_PASS_NEXT:;
  }
  return moves;
}

/* simulate a move - returns one of MOVE_PIECE_X */
int32_t movePiece(char *board, int8_t positionFrom, int8_t positionTo) {
    if (positionFrom < 0 || positionTo < 0) {
        return MOVE_PIECE_ERROR;
    }
    char piece = board[positionFrom];
    /* check if piece is pawn (with or without en passant), rook (that hasn't moved), or king (that hasn't moved) */
    if (piece == WHITE_PAWN && positionFrom / 8 == 6 && positionTo / 8 == 4) {
        /* change normal pawn to en passant pawn */
        piece = WHITE_PAWN_EN_PASSANT;
    } else if (piece == WHITE_PAWN_EN_PASSANT) {
        /* revert en passant to normal pawn */
        piece = WHITE_PAWN;
    } else if (piece == WHITE_ROOK_NO_MOVE) {
        /* revert rook to has moved */
        piece = WHITE_ROOK;
    } else if (piece == WHITE_KING_NO_MOVE) {
        /* revert king to has moved */
        piece = WHITE_KING;
    } else if (piece == BLACK_PAWN && positionFrom / 8 == 1 && positionTo / 8 == 3) {
        /* change normal pawn to en passant pawn */
        piece = BLACK_PAWN_EN_PASSANT;
    } else if (piece == BLACK_PAWN_EN_PASSANT) {
        /* revert en passant to normal pawn */
        piece = BLACK_PAWN;
    } else if (piece == BLACK_ROOK_NO_MOVE) {
        /* revert rook to has moved */
        piece = BLACK_ROOK;
    } else if (piece == BLACK_KING_NO_MOVE) {
        /* revert king to has moved */
        piece = BLACK_KING;
    }
    /* check: pawn promotion */
    if (piece == WHITE_PAWN && positionTo / 8 == 0) {
        board[positionFrom] = BLANK_SPACE;
        board[positionTo] = piece;
        return MOVE_PIECE_PAWN_PROMOTION_WHITE;
    } else if (piece == BLACK_PAWN && positionTo / 8 == 7) {
        board[positionFrom] = BLANK_SPACE;
        board[positionTo] = piece;
        return MOVE_PIECE_PAWN_PROMOTION_BLACK;
    } else {
        /* simulate piece moving */
        board[positionFrom] = BLANK_SPACE;
        board[positionTo] = piece;
        /* check en passant */
        if (piece == WHITE_PAWN) {
            int8_t enpassant = down(positionTo);
            if (enpassant != -1 && board[enpassant] == BLACK_PAWN_EN_PASSANT) {
                board[enpassant] = BLANK_SPACE;
            }
        } else if (piece == BLACK_ROOK) {
            int8_t enpassant = up(positionTo);
            if (enpassant != -1 && board[enpassant] == WHITE_PAWN_EN_PASSANT) {
                board[enpassant] = BLANK_SPACE;
            }
        }
        /* check castle */
        if (piece == WHITE_KING && positionFrom == 60 && positionTo == 62) {
            /* white right castle */
            board[63] = BLANK_SPACE;
            board[61] = WHITE_ROOK;
            return MOVE_PIECE_CASTLE_WHITE;
        } else if (piece == WHITE_KING && positionFrom == 60 && positionTo == 57) {
            /* white left castle */
            board[56] = BLANK_SPACE;
            board[58] = WHITE_ROOK;
            return MOVE_PIECE_CASTLE_WHITE;
        } else if (piece == BLACK_KING && positionFrom == 4 && positionTo == 6) {
            /* black right castle */
            board[7] = BLANK_SPACE;
            board[5] = BLACK_ROOK;
            return MOVE_PIECE_CASTLE_BLACK;
        } else if (piece == BLACK_KING && positionFrom == 4 && positionTo == 1) {
            /* black left castle */
            board[0] = BLANK_SPACE;
            board[2] = BLACK_ROOK;
            return MOVE_PIECE_CASTLE_BLACK;
        }
    }
    return MOVE_PIECE_SUCCESSFUL;
}

/* returns a list of positions where this piece can move to. Naive generation - does not cull moves that would put the king in check */
list_t *generateNaiveMoves(char *board, int8_t position, int8_t turn) {
    /* initialisation */
    list_t *output = list_init();
    /* validation */
    if (position == -1) {
        return output;
    }
    chess_piece_t type = getPieceType(board[position]);
    chess_color_t color = getPieceColor(board[position]);
    if (type == -1 || color == -1) {
        return output;
    }
    if (color != turn) {
        return output;
    }
    /* computation */
    if (type == CHESS_PIECE_PAWN) {
        /* pawn */
        int8_t (*direction[3]) (int8_t position);
        int8_t pawnPowerRow;
        if (color == CHESS_WHITE) {
            int8_t (*directionCopy[3]) (int8_t position) = {
                up, upLeft, upRight,
            };
            memcpy(direction, directionCopy, sizeof(void *) * 3);
            pawnPowerRow = 6;
        } else {
            int8_t (*directionCopy[3]) (int8_t position) = {
                down, downLeft, downRight,
            };
            memcpy(direction, directionCopy, sizeof(void *) * 3);
            pawnPowerRow = 1;
        }
        /* go up */
        int8_t spaceEmpty = 0; // ensure that pawn cannot move twice if a piece is in the way
        int8_t wanderingPosition = direction[0](position);
        if (wanderingPosition != -1) {
            if (board[wanderingPosition] == BLANK_SPACE) {
                list_append(output, (unitype) wanderingPosition, 'c');
                spaceEmpty = 1;
            }
        }
        /* possible: go up again */
        if (spaceEmpty && position / 8 == pawnPowerRow) {
            wanderingPosition = direction[0](wanderingPosition);
            if (wanderingPosition != -1) {
                if (board[wanderingPosition] == BLANK_SPACE) {
                    list_append(output, (unitype) wanderingPosition, 'c');
                }
            }
        }
        /* check for capture (and en passant) */
        wanderingPosition = direction[1](position);
        int8_t enpassantPosition = left(position);
        if (wanderingPosition != -1) {
            if ((board[wanderingPosition] != BLANK_SPACE && getPieceColor(board[wanderingPosition]) != color) || (board[wanderingPosition] == BLANK_SPACE && getPieceColor(board[enpassantPosition]) != color && (board[enpassantPosition] == WHITE_PAWN_EN_PASSANT || board[enpassantPosition] == BLACK_PAWN_EN_PASSANT))) {
                list_append(output, (unitype) wanderingPosition, 'c');
            }
        }
        wanderingPosition = direction[2](position);
        enpassantPosition = right(position);
        if (wanderingPosition != -1) {
            if ((board[wanderingPosition] != BLANK_SPACE && getPieceColor(board[wanderingPosition]) != color) || (board[wanderingPosition] == BLANK_SPACE && getPieceColor(board[enpassantPosition]) != color && (board[enpassantPosition] == WHITE_PAWN_EN_PASSANT || board[enpassantPosition] == BLACK_PAWN_EN_PASSANT))) {
                list_append(output, (unitype) wanderingPosition, 'c');
            }
        }
        /* Note: pawn promotion handled in movePiece */
    } else if (type == CHESS_PIECE_ROOK) {
        int8_t (*direction[4]) (int8_t position) = {
            up, right, down, left,
        };
        for (int32_t j = 0; j < 4; j++) {
            int8_t wanderingPosition = position;
            while (1) {
                wanderingPosition = direction[j](wanderingPosition);
                if (wanderingPosition == -1) {
                    break;
                }
                if (board[wanderingPosition] == BLANK_SPACE) {
                    list_append(output, (unitype) wanderingPosition, 'c');
                } else {
                    if (getPieceColor(board[wanderingPosition]) != color) {
                        list_append(output, (unitype) wanderingPosition, 'c');
                    }
                    break;
                }
            }
        }
    } else if (type == CHESS_PIECE_KNIGHT) {
        int8_t (*direction[12]) (int8_t position) = {
            up, upLeft, upRight, right, upRight, downRight, down, downRight, downLeft, left, downLeft, upLeft,
        };
        for (int32_t j = 0; j < 12; j += 3) {
            int8_t wanderingPosition = position;
            wanderingPosition = direction[j](wanderingPosition);
            int8_t checkPosition = direction[j + 1](wanderingPosition);
            if (checkPosition != -1 && (board[checkPosition] == BLANK_SPACE || getPieceColor(board[checkPosition]) != color)) {
                list_append(output, (unitype) checkPosition, 'c');
            }
            checkPosition = direction[j + 2](wanderingPosition);
            if (checkPosition != -1 && (board[checkPosition] == BLANK_SPACE || getPieceColor(board[checkPosition]) != color)) {
                list_append(output, (unitype) checkPosition, 'c');
            }
        }
    } else if (type == CHESS_PIECE_BISHOP) {
        int8_t (*direction[4]) (int8_t position) = {
            upRight, downRight, downLeft, upLeft,
        };
        for (int32_t j = 0; j < 4; j++) {
            int8_t wanderingPosition = position;
            while (1) {
                wanderingPosition = direction[j](wanderingPosition);
                if (wanderingPosition == -1) {
                    break;
                }
                if (board[wanderingPosition] == BLANK_SPACE) {
                    list_append(output, (unitype) wanderingPosition, 'c');
                } else {
                    if (getPieceColor(board[wanderingPosition]) != color) {
                        list_append(output, (unitype) wanderingPosition, 'c');
                    }
                    break;
                }
            }
        }
    } else if (type == CHESS_PIECE_QUEEN) {
        int8_t (*direction[8]) (int8_t position) = {
            up, upRight, right, downRight, down, downLeft, left, upLeft,
        };
        for (int32_t j = 0; j < 8; j++) {
            int8_t wanderingPosition = position;
            while (1) {
                wanderingPosition = direction[j](wanderingPosition);
                if (wanderingPosition == -1) {
                    break;
                }
                if (board[wanderingPosition] == BLANK_SPACE) {
                    list_append(output, (unitype) wanderingPosition, 'c');
                } else {
                    if (getPieceColor(board[wanderingPosition]) != color) {
                        list_append(output, (unitype) wanderingPosition, 'c');
                    }
                    break;
                }
            }
        }
    } else if (type == CHESS_PIECE_KING) {
        int8_t (*direction[8]) (int8_t position) = {
            up, upRight, right, downRight, down, downLeft, left, upLeft,
        };
        for (int32_t j = 0; j < 8; j++) {
            int8_t wanderingPosition = position;
            wanderingPosition = direction[j](wanderingPosition);
            if (wanderingPosition != -1 && (board[wanderingPosition] == BLANK_SPACE || getPieceColor(board[wanderingPosition]) != color)) {
                list_append(output, (unitype) wanderingPosition, 'c');
            }
        }
        /* castling */
        if (board[position] == WHITE_KING_NO_MOVE && board[63] == WHITE_ROOK_NO_MOVE && board[62] == BLANK_SPACE && board[61] == BLANK_SPACE) {
            /* white right castle */
            list_append(output, (unitype) 62, 'c');
        }
        if (board[position] == WHITE_KING_NO_MOVE && board[56] == WHITE_ROOK_NO_MOVE && board[57] == BLANK_SPACE && board[58] == BLANK_SPACE && board[59] == BLANK_SPACE) {
            /* white left castle */
            list_append(output, (unitype) 57, 'c');
        }
        if (board[position] == BLACK_KING_NO_MOVE && board[7] == BLACK_ROOK_NO_MOVE && board[6] == BLANK_SPACE && board[5] == BLANK_SPACE) {
            /* black right castle */
            list_append(output, (unitype) 6, 'c');
        }
        if (board[position] == BLACK_KING_NO_MOVE && board[0] == BLACK_ROOK_NO_MOVE && board[1] == BLANK_SPACE && board[2] == BLANK_SPACE && board[3] == BLANK_SPACE) {
            /* black left castle */
            list_append(output, (unitype) 1, 'c');
        }
    }
    return output;
}