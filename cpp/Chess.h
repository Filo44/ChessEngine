#pragma once
#ifndef Chess_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define Chess_H

#include <iostream>
#include "httplib.h"
#include <string>

//#include "Eigen/Core"

//using Bitboard = Eigen::Matrix<bool, 8, 8>;
using Bitboard = uint64_t;
using namespace std;

char** fenToMatrix(std::string fen);
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

#endif