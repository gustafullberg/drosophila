# Pawned
A chess engine by Gustaf Ullberg.

## Introduction
Pawned is an open source chess engine. You can use it with any chess GUI supporting the Chess Engine Communication Protocol, such as Xboard or Arena.

If you are developing a chess GUI, you also can compile the engine as a library and use it within your application.

## Releases
**1.2** - 2014-12-27

* Stability improvements (resolving illegal moves)
* New openging book
* Support for Polyglot books
* Support for incremental time controls

**1.1** - 2014-12-08

* Pondering (thinking during the opponent's turn)
* Full principal variation in thinking output
* Support for the "setboard" command
* Pawn shield
* Performance improvements

**1.0** - 2014-02-28

* First release

## How to build from source code
You need a **Git** client, **CMake** and a **C compiler** (GCC, Clang or Visual Studio) to obtain and build the source code.

### Linux, Mac OS, etc
```
git clone https://github.com/gustafullberg/pawned.git
cd pawned
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" 
make
```
### Windows (assuming a 64-bit build with Visual Studio 2012)
```
git clone https://github.com/gustafullberg/pawned.git
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
* Futility pruning
* Late move reductions
* Pondering
* Opening book

Evaluation:

* Material
* Piece-square tables
* Pawns defending minor pieces
* Pawn shield

## License
Pawned is released under the MIT License. 
