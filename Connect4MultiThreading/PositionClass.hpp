#include <bitset>
#include <iostream>
#include <cassert>
#include <cstdint>
#ifndef POSITIONCLASS_HPP
#define POSITIONCLASS_HPP

class Position {
public:
    static const int WIDTH = 7;  // Width of the board
    static const int HEIGHT = 6; // Height of the board
    static const uint64_t bottomMask = 0b1000000100000010000001000000100000010000001000000;
    static constexpr uint64_t topMask = 0b1;
    static constexpr int MIN_SCORE = -(WIDTH * HEIGHT) / 2 + 3;
    static constexpr int MAX_SCORE = (WIDTH * HEIGHT + 1) / 2 - 3;
    static const uint64_t columnMask = 0b111111;

    template<int width, int height> struct bottom { static constexpr uint64_t mask = bottom<width - 1, height>::mask | uint64_t(1) << (width - 1) * (height + 1); };
    template <int height> struct bottom<0, height> { static constexpr uint64_t mask = 0; };

    static constexpr uint64_t bottom_mask = bottom<WIDTH, HEIGHT>::mask;
    static constexpr uint64_t board_mask = bottom_mask * ((1LL << HEIGHT) - 1);

    uint64_t mask = 0;
    uint64_t player = 0;
    uint64_t key = 0;
    uint64_t key2 = 0;
    uint16_t moves = 0;

    uint64_t reverseBitstring49(uint64_t bitstring) {

        uint64_t reversedPos;
        reversedPos = ((bitstring & columnMask) << 6 * 7) | ((bitstring & (columnMask << 7)) << 4 * 7) | ((bitstring & (columnMask << 2 * 7)) << 2 * 7) | (bitstring & (columnMask << 3 * 7)) | ((bitstring & (columnMask << 4 * 7)) >> 2 * 7) | ((bitstring & (columnMask << 5 * 7)) >> 4 * 7) | ((bitstring & (columnMask << 6 * 7)) >> 6 * 7);
        return reversedPos;
    }

    /**
    * Indicates whether a column is playable.
    * @param col: 0-based index of column to play
    * @return true if the column is playable, false if the column is already full.
    */
    bool canPlay(int col) const {
        return (mask & (topMask << (col * 7 + 5))) == 0;
    }
    /*
    .  .  .  .  .  .  .
    5 12 19 26 33 40 47
    4 11 18 25 32 39 46
    3 10 17 24 31 38 45
    2  9 16 23 30 37 44
    1  8 15 22 29 36 43
    0  7 14 21 28 35 42
    */

    uint64_t top_mask(int col) const {
        return topMask << (col * 7);
    }

    /**
     * Plays a playable column.
     * This function should not be called on a non-playable column or
     * in miniMax function.
     * @param column: 0-based index of a playable column.
     */
    void play(int col) {
        player = player ^ mask;
        mask |= mask + bottom_mask_col(col);
        key = player + mask;
        key2 = reverseBitstring49(key);
        moves++;
    }

    static constexpr uint64_t bottom_mask_col(int col) {
        return UINT64_C(1) << col * (HEIGHT + 1);
    }

    //ska tas bort
    bool hasWon(uint64_t player) const {
        uint64_t tempBoardHorizontal = player & (player >> 7);
        uint64_t tempBoarddiagonal = player & (player >> 8);
        uint64_t tempBoarddiagonal2 = player & (player >> 6);
        uint64_t tempBoardvertical = player & (player >> 1);
        return (tempBoardHorizontal & (tempBoardHorizontal >> 2 * 7) || tempBoarddiagonal & (tempBoarddiagonal >> 2 * 8) || tempBoarddiagonal2 & (tempBoarddiagonal2 >> 2 * 6) || tempBoardvertical & (tempBoardvertical >> 2));
    };

    bool nextMoveWin() const {
        return winning_position() & possible();
    } 

    uint64_t winning_position() const {
        return compute_winning_position(player, mask);
    }

    uint64_t losingMoves() const {
        return compute_winning_position(player ^ mask, mask);
    }

    uint64_t possible() const {
        return (mask + bottom_mask) & board_mask;
    }

    /**
    * Return a bitmap of all the possible next moves the do not lose in one turn.
    * A losing move is a move leaving the possibility for the opponent to win directly.
    *
    * Warning this function is intended to test position where you cannot win in one turn
    * If you have a winning move, this function can miss it and prefer to prevent the opponent
    * to make an alignment.
    */
    uint64_t possibleNonLosingMoves() const {
        assert(!nextMoveWin());
        uint64_t possible_mask = possible();
        uint64_t opponent_win = losingMoves();
        uint64_t forced_moves = possible_mask & opponent_win;
        if (forced_moves) {
            if (forced_moves & (forced_moves - 1)) // check if there is more than one forced move
                return 0;                           // the opponnent has two winning moves and you cannot stop him
            else possible_mask = forced_moves;    // enforce to play the single forced move
        }
        return possible_mask & ~(opponent_win >> 1);  // avoid to play below an opponent winning spot
    }

    void drawBoard() {
        std::bitset<49> p1board;
        std::bitset<49> p2board;
        if (moves % 2) {
            p1board = player;
            p2board = player ^ mask;
        }
        else {
            p1board = player ^ mask;
            p2board = player;
        }

        std::cout << "\n-----------------------------\n";

        for (int i = 0; i < 7; i++) {
            std::cout << "\n";

            for (int j = 0; j < 7; j++) {

                if (p1board[6 + 7 * j - i]) {
                    std::cout << "| X ";
                }
                else if (p2board[6 + 7 * j - i])
                    std::cout << "| O ";
                else std::cout << "|   ";
            }
            std::cout << "|";
        }

        std::cout << "\n-----------------------------\n";
    };

    static uint64_t compute_winning_position(uint64_t position, uint64_t mask) {
        // vertical;
        uint64_t r = (position << 1) & (position << 2) & (position << 3);

        //horizontal
        uint64_t p = (position << (HEIGHT + 1)) & (position << 2 * (HEIGHT + 1));
        r |= p & (position << 3 * (HEIGHT + 1));
        r |= p & (position >> (HEIGHT + 1));
        p = (position >> (HEIGHT + 1)) & (position >> 2 * (HEIGHT + 1));
        r |= p & (position << (HEIGHT + 1));
        r |= p & (position >> 3 * (HEIGHT + 1));

        //diagonal 1
        p = (position << HEIGHT) & (position << 2 * HEIGHT);
        r |= p & (position << 3 * HEIGHT);
        r |= p & (position >> HEIGHT);
        p = (position >> HEIGHT) & (position >> 2 * HEIGHT);
        r |= p & (position << HEIGHT);
        r |= p & (position >> 3 * HEIGHT);

        //diagonal 2
        p = (position << (HEIGHT + 2)) & (position << 2 * (HEIGHT + 2));
        r |= p & (position << 3 * (HEIGHT + 2));
        r |= p & (position >> (HEIGHT + 2));
        p = (position >> (HEIGHT + 2)) & (position >> 2 * (HEIGHT + 2));
        r |= p & (position << (HEIGHT + 2));
        r |= p & (position >> 3 * (HEIGHT + 2));

        return r & (board_mask ^ mask);
    }
};

#endif 