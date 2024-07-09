#pragma once
#ifndef Chess_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define Chess_H

#include <iostream>
#include "httplib.h"
#include <string>
#include <unordered_map>
#include <cstdint>


const char pieces[6] = { 'r','n','b','q', 'k','p' };
unordered_map<char, int> pieceToNumber = {
		{'r', 0},
		{'n', 1},
		{'b', 2},
		{'q', 3},
		{'k', 4},
		{'p', 5}
};

using Bitboard = uint64_t;
using namespace std;

//char** fenToMatrix(std::string fen);
void delete2DArray(char** arr, int rows);
string convertToString(char** arr, int cols, int rows);
string convertToJSArr(char** arr, int cols, int rows);
string convertVofBBJS(vector<Bitboard> matrixVector);

inline void setBitTo(Bitboard* initBB, int posX, int posY, bool value) {
	if (value == 0) {
		*initBB &= ~(1ULL << posX + (posY * 8));
	}
	else {
		*initBB |= (1ULL << posX + (posY * 8));
	}
}
inline bool getBit(Bitboard bb, int posX, int posY) {
	return (bb >> posX + (posY * 8)) & 1;
}
class PieceTypeCurrPositions;
class OneColorCurrPositions;
class AllCurrPositions;

class PieceTypeCurrPositions {
public:
	char pieceType;
	vector<Bitboard> posBB;
	//Maybe don't calculate this every time? Thus don't include it here. Don't know, may be worse.
	Bitboard pieceTypeCombinedBB;
};


class OneColorCurrPositions {
public:
	PieceTypeCurrPositions pieceTypes[6];
	//Maybe don't calculate this every time? Thus don't include it here. Don't know, may be worse.
	Bitboard colorCombinedBB;
};


class AllCurrPositions {
	public:
		OneColorCurrPositions colorBitboards[2];
};
AllCurrPositions fenToPosBitboards(std::string fen);
char** allPositionBitboardsToMatrix(AllCurrPositions allPositionBitboardsL);

#endif