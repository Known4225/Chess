#include <stdlib.h>

#define BLANK_SPACE                     '0'
#define WHITE_PAWN                      '1'
#define WHITE_ROOK                      '2'
#define WHITE_KNIGHT                    '3'
#define WHITE_BISHOP                    '4'
#define WHITE_QUEEN                     '5'
#define WHITE_KING                      '6'
#define WHITE_PAWN_TWO_SQUARES          '7'
#define WHITE_ROOK_UNMOVED              '8'
#define WHITE_KING_UNMOVED              '9'
#define BLACK_PAWN                      'A'
#define BLACK_ROOK                      'B'
#define BLACK_KNIGHT                    'C'
#define BLACK_BISHOP                    'D'
#define BLACK_QUEEN                     'E'
#define BLACK_KING                      'F'
#define BLACK_PAWN_TWO_SQUARES          'G'
#define BLACK_ROOK_UNMOVED              'H'
#define BLACK_KING_UNMOVED              'I'

char queens_gambit[66] = {'b',
                          'H', 'C', 'D', 'E', 'I', 'D', 'C', 'H',
                          'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A',
                          '0', '0', '0', '0', '0', '0', '0', '0',
                          '0', '0', '0', '0', '0', '0', '0', '0',
                          '0', '0', '0', '7', '0', '0', '0', '0',
                          '0', '0', '0', '0', '0', '0', '0', '0',
                          '1', '1', '1', '0', '1', '1', '1', '1',
                          '8', '3', '4', '5', '9', '4', '3', '8'};