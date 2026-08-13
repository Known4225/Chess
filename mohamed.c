#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

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
#if DEBUG
      printf("Checking move: %s\n", state);
#endif
      double state_value = get_state_value(state);
#if DEBUG
      printf("State value for move %s: %f\n", state, state_value);
#endif
      if (current_state[0] == 'w') {
        if (state_value >= get_state_value(final_state)) {
#if DEBUG
        printf("Updating final state to: %s with value: %f\n", state, state_value);
#endif
        memcpy(final_state, state, GAME_STATE_SIZE);
        }
      } else {
        if (state_value <= get_state_value(final_state)) {
#if DEBUG
        printf("Updating final state to: %s with value: %f\n", state, state_value);
#endif
        memcpy(final_state, state, GAME_STATE_SIZE);
        }
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
    case WHITE_PAWN_TWO_SQUARES:
      return 1.0;
    break;
    case WHITE_ROOK:
    case WHITE_ROOK_UNMOVED:
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
    case WHITE_KING_UNMOVED:
      //return 1000; // high value for white king
      return 0.0;
    break;
    case BLACK_PAWN:
    case BLACK_PAWN_TWO_SQUARES:
      return -1.0;
    break;
    case BLACK_ROOK:
    case BLACK_ROOK_UNMOVED:
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
    case BLACK_KING_UNMOVED:
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