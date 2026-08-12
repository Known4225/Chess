# Chess

- Piece images sourced from [chess.com](https://www.chess.com/)
- Color scheme sourced from [chess.com](https://www.chess.com/)
- UI behaviour copied from [chess.com](https://www.chess.com/)

# How to use

Play chess manually or make use of the custom engines by clicking the `Mohamed` or `Ryan` buttons which will use the engine to make a single move.

# Funny Rules
- Pawn promotion
- En passant can only be done in the very next turn
- Cannot castle out of or THROUGH check
- Ensure that moves are crossed off if they result in a check
- Castle only highlights king (and can only be done via king)
- 50 moves without capture triggers draw
- Moving back and forth 3 times triggers draw

# How to build

You'll need gcc. I recommend [w64devkit](https://github.com/skeeto/w64devkit).

Build the chess.exe gui:
```
gcc chess.c -L./Windows -lturtle -lglfw3 -lopengl32 -lgdi32 -lglad -lole32 -luuid -lwsock32 -lWs2_32 -lMf -lMfplat -lmfreadwrite -lmfuuid -DOS_WINDOWS -O3 -o chess.exe
```
Use `make winrel` to automate.

Build the ryan.exe engine:
```
gcc ryan.c -O3 -o ryan.exe
```
Use `make ryan` to automate.

Build the mohamed.exe engine:
```
gcc mohamed.c -O3 -o mohamed.exe
```
Use `make mohamed` to automate.