# Drosophila
A chess engine by Gustaf Ullberg.

## Introduction
Drosophila is an open source chess engine. You can use it with any chess GUI supporting the Chess Engine Communication Protocol, such as Xboard or Arena.

If you are developing a chess GUI, you also can compile the engine as a library and use it within your application.

Drosophila was earlier named Pawned. 

## Releases

**[Drosophila 1.6](https://github.com/gustafullberg/drosophila/releases/download/v1.6/drosophila-1.6.zip)** - 2020-03-12

* Retuned evaluation
* Deeper futility pruning
* Handle check in quiescence search
* SEE used in move ordering
* King tropism replaced by king pressure
* Improved time management

**[Drosophila 1.5.1](https://github.com/gustafullberg/drosophila/releases/download/v1.5.1/drosophila-1.5.1.zip)** - 2018-03-05

* Bugfix: Avoid unnecessary CPU load while waiting for input (Windows only)

**[Drosophila 1.5](https://github.com/gustafullberg/drosophila/releases/download/v1.5/drosophila-1.5.zip)** - 2018-02-11

* Evaluation improvements
  * Better evaluation of passed pawns
  * Mobility
* Search improvements
  * Improvements to late move reduction
  * More aggressive futility pruning
  * History heuristic
  * Killer moves

**[Drosophila 1.4](https://github.com/gustafullberg/drosophila/releases/download/v1.4/drosophila-1.4.zip)** - 2015-03-06

* Improved evaluation
  * Passed pawns
  * Isolated pawns
  * Rooks on (half-)open files
  * King tropism
  * Smooth transition between game phases
* Better detection of insufficient mating material
* Search considers 50 move rule

**[Drosophila 1.3](https://github.com/gustafullberg/drosophila/releases/download/v1.3/drosophila-1.3.zip)** - 2015-01-24

* New name to avoid confusion with other chess engines
* SEE pruning of quiescence search
* Improved evaluation function
* Make use of null move pruning in the endgame
* Accept xboard commands during search
* Support for the xboard "?" command (move now)
* Report draws due to:
  * the 50 move rule
  * insufficient mating material
  * threefold repetition to the GUI

**[Pawned 1.2](https://github.com/gustafullberg/drosophila/releases/download/v1.2/pawned-1.2.zip)** - 2014-12-27

* Stability improvements (resolving illegal moves)
* New openging book
* Support for Polyglot books
* Support for incremental time controls

**[Pawned 1.1](https://github.com/gustafullberg/drosophila/releases/download/v1.1/pawned-1.1.zip)** - 2014-12-08

* Pondering (thinking during the opponent's turn)
* Full principal variation in thinking output
* Support for the "setboard" command
* Pawn shield
* Performance improvements

**[Pawned 1.0](https://github.com/gustafullberg/drosophila/releases/download/v1.0/pawned-1.0.zip)** - 2014-02-28

* First release

## How to build from source code
You need a **Git** client, **CMake** and a **C compiler** (GCC, Clang or Visual Studio) to obtain and build the source code.

### Linux, Mac OS, etc
```
git clone https://github.com/gustafullberg/drosophila.git
cd drosophila
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" 
make
```
### Windows (assuming a 64-bit build with Visual Studio 2012)
```
git clone https://github.com/gustafullberg/drosophila.git
cd drosophila
mkdir build
cd build
cmake .. -G "Visual Studio 11 Win64"
```
Now open the newly created drosophila.sln file and build from within Visual Studio.

## Technical features

State representation:

* Bit-boards

Search:

* MTD(f)
* Iterative deepening
* Quiescence search with SEE pruning
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
* Mobility
* Pawns defending other pawns and minor pieces
* Pawn structure
* Rooks on (half-)open files
* King pressure
* Tempo

## License
Drosophila is released under the MIT License. 
