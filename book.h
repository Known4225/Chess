#include <stdlib.h>

#define BLANK_SPACE                     0x30
#define WHITE_PAWN                      0x31
#define WHITE_ROOK                      0x32
#define WHITE_KNIGHT                    0x33
#define WHITE_BISHOP                    0x34
#define WHITE_QUEEN                     0x35
#define WHITE_KING                      0x36
#define WHITE_PAWN_TWO_SQUARES          0x37
#define WHITE_ROOK_UNMOVED              0x38
#define WHITE_KING_UNMOVED              0x39
#define BLACK_PAWN                      0x41
#define BLACK_ROOK                      0x42
#define BLACK_KNIGHT                    0x43
#define BLACK_BISHOP                    0x44
#define BLACK_QUEEN                     0x45
#define BLACK_KING                      0x46
#define BLACK_PAWN_TWO_SQUARES          0x47
#define BLACK_ROOK_UNMOVED              0x48
#define BLACK_KING_UNMOVED              0x49

char queens_gambit[66] = {'b',
                          'H', 'C', 'D', 'E', 'I', 'D', 'C', 'H',
                          'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A',
                          '0', '0', '0', '0', '0', '0', '0', '0',
                          '0', '0', '0', '0', '0', '0', '0', '0',
                          '0', '0', '0', '7', '0', '0', '0', '0',
                          '0', '0', '0', '0', '0', '0', '0', '0',
                          '1', '1', '1', '0', '1', '1', '1', '1',
                          '8', '3', '4', '5', '9', '4', '3', '8'};