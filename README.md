# Pawned
A chess engine by Gustaf Ullberg.

Pawned is an open source chess engine. You can use it with any chess GUI supporting the Xboard/Winboard protocol.
If you are developing a chess GUI you can compile the engine as a library and use it within your application.

## How to build from source code
You need a **Git** client, **CMake** and a **C compiler** (GCC, Clang or Visual Studio) to obtain and build the source code.

### Linux, Mac OS, etc
```
git clone https://github.com/docentdamp/pawned.git
cd pawned
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" 
make
```
### Windows (assuming a 64-bit build with Visual Studio 2012)
```
git clone https://github.com/docentdamp/pawned.git
cd pawned
mkdir build
cd build
cmake .. -G "Visual Studio 11 Win64"
```
Now open the newly created pawned.sln file and build from within Visual Studio.

## Technical features

State representation:

* Bit-boards

Search:

* MTD(f)
* Iterative deepening
* Quiescence search
* Transposition table
* Move ordering
* Null move pruning
* Late move reductions
* Opening book

Evaluation:

* Material
* Piece-square tables
* Pawn structure 
   * Doubled / trippled pawns
   * Pawns defending each other

## License
Pawned is released under the MIT License. 
