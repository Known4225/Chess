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

// functions
int get_state_piece_value(char *state);
int get_state_vulnerability_value(char *state);
int get_state_control_value(char *state);

void set_final_state(char *state, char *file_name);
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
  
  while (fgets(state, sizeof(state), input_file) != NULL) {
    size_t len = strnlen(state, GAME_STATE_SIZE - 1);
    memcpy(legal_moves[legal_state_count], state, len);
    legal_moves[legal_state_count][len] = '\0';
    legal_state_count++;
  }

  if (legal_state_count > 0) {
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
      int state_value = get_state_piece_value(state);
      if (state_value >= get_state_piece_value(final_state)) {
        memcpy(final_state, state, GAME_STATE_SIZE);
      }
    }
  } else {
    printf("No legal moves found.\n");
  }

  set_final_state(final_state, argv[2]);
  fclose(input_file);
  return 0;
}

int get_state_piece_value(char *state) {
  int value = 0;
  for (int i = 1; i < GAME_STATE_SIZE - 1; i++) {
    switch (state[i]) {
      case WHITE_PAWN:
        value += 1;
        break;
      case WHITE_ROOK:
        value += 5;
        break;
      case WHITE_KNIGHT:
        value += 3;
        break;
      case WHITE_BISHOP:
        value += 3;
        break;
      case WHITE_QUEEN:
        value += 9;
        break;
      case WHITE_KING:
        // value += 1000; // high value for white king
        break;
      case BLACK_PAWN:
        value -= 1;
        break;
      case BLACK_ROOK:
        value -= 5;
        break;
      case BLACK_KNIGHT:
        value -= 3;
        break;
      case BLACK_BISHOP:
        value -= 3;
        break;
      case BLACK_QUEEN:
        value -= 9;
        break;
      case BLACK_KING:
        // value -= 1000; // high negative value for black king
        break;
      default:
        break;
    }
  }

  if (state[0] == 'b') {
    value = value * -1;
  }

  return value;
}

int get_state_vulnerability_value(char *state) {
  // placeholder
  return 0;
}

int get_state_control_value(char *state) {
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
    fputs(state, output_file);
  }
  
  fclose(output_file);
}