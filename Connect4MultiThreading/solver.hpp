#ifndef solver_h
#define solver_h
#include "PositionClass.hpp"
#include "TransPositionTable.hpp"
#include <thread>
#include <future>
#include <mutex>
#include <random>
#include <chrono>
#include <unordered_map>
#include <string>
#include <iostream>
constexpr char movePrio[7] = { 3,2,4,1,5,0,6 };

class solver {
	std::unordered_map<uint64_t, int> openingBook;
	Connect4::TranspositionTable<49, 8, 29> positions;
	bool openingBookLoaded = false;
	

public:

	bool makeOpeningBook = false;
	std::mutex mtx;
	static const char INFI = 43;


	uint64_t nodeCounter = 0;
	float totalTime = 0;
	clock_t deltaTime = 0;

	int negaMax(Position& P, int alpha = -INFI, int beta = +INFI, bool returnBestMove = false) {
		int tempEval;
		int bestMove;

		nodeCounter++;

		if (nodeCounter % 10'000'000 == 0) {
			std::cout << "NodesExplored: " << nodeCounter / 1'000'000 << "M\n";
		}

		if (P.nextMoveWin()) {
			return (Position::WIDTH * Position::HEIGHT + 4 - P.moves) / 2;
		}

		if (P.moves == 42) {
			return 0; // draw
		}
		uint64_t nonLosingMoves = P.possibleNonLosingMoves();

		if (!nonLosingMoves) {
			return -(Position::WIDTH * Position::HEIGHT + 2 - P.moves) / 2;
		}

		if (P.moves < 10 && openingBookLoaded) {
			if (openingBook.count(P.key)) {
				return openingBook.at(P.key);
			}
			if (openingBook.count(P.key2)) {
				return openingBook.at(P.key2);
			}
		}

		int min = -(Position::WIDTH * Position::HEIGHT - 2 - P.moves) / 2;	// lower bound of score as opponent cannot win next move
		if (alpha < min) {
			alpha = min;                     // there is no need to keep alpha below our max possible score.
			//if (makeOpeningBook && P.moves <= 10) openingBook.emplace(P.key, alpha - Position::MIN_SCORE + 1);
			if (P.moves > 8)
			if (alpha >= beta) return alpha;  // prune the exploration if the [alpha;beta] window is empty.
		}

		int max = (42 - 1 - P.moves) / 2;
		if (beta > max) {
			beta = max;
			if (P.moves > 8)
			if (alpha >= beta) return beta;
		}

		int val = positions.at(P.key);
		if (!val) {
			val = positions.at(P.key2);
		}

		if (val) {    // fetch potential stored lower or upper bound of the score
			if (val > Position::MAX_SCORE - Position::MIN_SCORE + 1) { // we have an lower bound
				min = val + 2 * Position::MIN_SCORE - Position::MAX_SCORE - 2;
				if (alpha < min) {
					alpha = min;                     // there is no need to keep alpha below our min possible score.
					//if (makeOpeningBook && P.moves <= 10) openingBook.emplace(P.key, alpha - Position::MIN_SCORE + 1);
					if (P.moves > 8)
					if (alpha >= beta) return alpha;  // prune the exploration if the [alpha;beta] window is empty.
				}
			}
			else { // we have an upper bound
				max = val + Position::MIN_SCORE - 1;
				if (beta > max) {
					beta = max;
					if (P.moves > 8)// there is no need to keep beta above our max possible score.
					if (alpha >= beta) return beta;  // prune the exploration if the [alpha;beta] window is empty.
				}
			}
		}

		constexpr uint64_t columnMask = 0b111111;
		for (int i : movePrio) {
			if (P.canPlay(i) && (nonLosingMoves & (columnMask << (i * 7)))) {

				Position P2(P);
				P2.play(i);

				tempEval = -negaMax(P2, -beta, -alpha, false);
				if (tempEval >= beta) {

					mtx.lock();
					positions.emplace(P.key, tempEval + Position::MAX_SCORE - 2 * Position::MIN_SCORE + 2);
					mtx.unlock();
					if(P.moves > 8)
					return tempEval;  // prune the exploration if we find a possible move better than what we were looking for.
				}
				if (tempEval > alpha) alpha = tempEval, bestMove = i; // reduce the [alpha;beta] window for next exploration, as we only
				// need to search for a position that is better than the best so far.
			}
		}

		mtx.lock();
		positions.emplace(P.key, alpha - Position::MIN_SCORE + 1);
		if (makeOpeningBook && P.moves <= 10) 
		{
			std::ofstream file("OpeningBook.txt", std::ios::app);
			file << P.key << " " << (alpha - Position::MIN_SCORE + 1) << "\n";
		}
		mtx.unlock();

		//if (returnBestMove) {
		//	return bestMove;
		//}
		return alpha;
	}

	void umbrellaFunc(std::promise<int>&& p, Position P2) {
		int tempValue;
		int alpha = -21;
		int beta = 21;
		p.set_value(-negaMax(P2, alpha, beta));
	}

	auto solve(Position& P) {

		struct E {
			int evals[7] = { -9999 };
			int bestMove = 0;
		};
		E eval;
		bool firstMove = true;
		int tempEval = 0;

		std::vector<std::thread> threads;
		std::future<int> futures[7];
		int tempMoves[1] = { 0 };
		for (int i : movePrio) {
			if (P.canPlay(i)) {

				Position P2(P);
				P2.play(i);

				std::promise<int> p;
				futures[i] = (p.get_future());

				threads.emplace_back(std::thread(&solver::umbrellaFunc, this, std::move(p), P2));
			}
		}
		for (auto& thread : threads) {
			thread.join();
		}

		int counter = 0;
		for (int i = 0; i < 7; i++) {
			if (P.canPlay(i)) {
				eval.evals[i] = futures[i].get();

				srand(time(NULL));

				if (eval.evals[i] > eval.evals[eval.bestMove]) {
					eval.bestMove = i;
				}
				else if (eval.evals[i] == eval.evals[eval.bestMove] && (rand() % 2)) {
					eval.bestMove = i;
				}
				std::cout << "|" << eval.evals[i];
			}
		}
		std::cout << "|\n";
		return eval;
	}

	void aiPlay(Position& P) {
		int tempEval;
		int bestMove;
		int alpha = -22;
		int beta = 22;
		bool firstValidMove = true;
		uint64_t nonLosingMoves = 0b0100000010000001000000100000010000001000000100000;
		nodeCounter++;

		if(!P.nextMoveWin()) {
			nonLosingMoves = P.possibleNonLosingMoves();
			if (!nonLosingMoves) {
				for (int i : movePrio) {
					if (P.canPlay(i))
					{
						P.play(i);
						return;
					}
				}
			}
		}

		int val = positions.at(P.key);
		if (!val) {
			val = positions.at(P.key2);
		}

		constexpr uint64_t columnMask = 0b111111;
		for (int i : movePrio) {
			if (P.canPlay(i) && (nonLosingMoves & (columnMask << (i * 7)))) {

				Position P2(P);
				P2.play(i);

				tempEval = -negaMax(P2, -beta, -alpha, false);

				if (tempEval > alpha) alpha = tempEval, bestMove = i; // reduce the [alpha;beta] window for next exploration, as we only
				// need to search for a position that is better than the best so far.
				if (firstValidMove) {
					firstValidMove = false;
					bestMove = i;
				}
			}
		}
		P.play(bestMove);
	}

	void writeOpeningBook() {
		std::ofstream file("OpeningBook.txt");
		for (auto i : openingBook) {
			file << i.first << " " << i.second << "\n";
		}
	}

	void loadOpeningBook() {
		std::ifstream file("OpeningBook.txt");
		uint64_t key;
		int value;
		int counter = 0;
		while (file >> key >> value) {
			openingBook.emplace(key, value);
			counter++;
		}
		std::cout << counter;
		openingBookLoaded = true;
	}
};

#endif
