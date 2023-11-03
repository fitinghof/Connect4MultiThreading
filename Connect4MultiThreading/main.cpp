#include <string>
#include <iostream>
#include <map>
#include <bitset>
#include <time.h>
#include <math.h>
#include "TransPositionTable.hpp"
#include "PositionClass.hpp"
#include "solver.hpp"
#include <chrono>

/*
.  .  .  .  .  .  .
5 12 19 26 33 40 47
4 11 18 25 32 39 46
3 10 17 24 31 38 45
2  9 16 23 30 37 44
1  8 15 22 29 36 43
0  7 14 21 28 35 42
*/


int main() {
    solver s1;
    //s1.loadOpeningBook();
    //s1.makeOpeningBook = true;
    Position Current;
    int temp = 0;
    //Current.play(3);

    while (true) {

        Current.drawBoard();
        if ((Current.moves + 1) % 2) {
            /*int input;
            std::cin >> input;
            Current.play(input - 1);*/

            auto start1{ std::chrono::steady_clock::now() };
            s1.aiPlay(Current);
            auto end1{ std::chrono::steady_clock::now() };
            std::cout << std::chrono::duration<double>(end1 - start1);
        }
        else {
            auto start{ std::chrono::steady_clock::now()};
            s1.aiPlay(Current);
            auto end{ std::chrono::steady_clock::now()};
            std::cout << std::chrono::duration<double>(end - start);
        }

        if (Current.hasWon(Current.player ^ Current.mask)) {
            std::cout << "Player1 Won!";
            Current.drawBoard();
            break;
        }
        else if (Current.moves == 42) {
            std::cout << "Draw";
            Current.drawBoard();
            break;
        }
    }
    //s1.writeOpeningBook();
}