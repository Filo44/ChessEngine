#pragma once

#include <iostream>
#include "httplib.h"
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <ctime>
#include <intrin.h>
#include <algorithm>

using namespace nlohmann;
using namespace std;

enum Piece {
	blackRook = 0,
	blackKnight = 1,
	blackBishop = 2,
	blackQueen = 3,
	blackKing = 4,
	blackPawn = 5,
	whiteRook = 6,
	whiteKnight = 7,
	whiteBishop = 8,
	whiteQueen = 9,
	whiteKing = 10,
	whitePawn = 11,
};
enum PieceWithoutColor {
	rook = 0,
	knight = 1,
	bishop = 2,
	queen = 3,
	king = 4,
	pawn = 5
};

using Bitboard = uint64_t;
using MoveMag = array<int, 3>;
using ZobristHash = uint64_t;
using DirectionBitboards = array<Bitboard, 4>;
using int8 = uint8_t;

class MoveDesc;
class PieceTypeCurrPositions;
class OneColorCurrPositions;
class AllCurrPositions;

class SinglePiecePosMoves;
class PieceTypePosMoves;
class AllPosMoves;
class PinnedPieceData;
class AttackingAndPinnedBBs;
class MoveCapAndPinnedBBs;
class CheckData;
class LeafNodesAndCurrPos;
class PosAndColor;
class EvalAndBestMove;
class BitboardAndPieceInfo;

using MovesByPos = array<vector<MoveDesc>, 64>;

// char** fenToMatrix(std::string fen);
void delete2DArray(char** arr, int rows);
string convertToString(char** a, int cols = 8, int rows = 8);
string convertToJSArr(char** arr, int cols, int rows);
string convertVofBBJS(vector<Bitboard> matrixVector);
stringstream convertBBJS(Bitboard curBB);
string allPosMovesToMatrix(AllPosMoves posMoves);
ZobristHash genInitZobristHash(AllCurrPositions currPositions, bool colorToMove);
EvalAndBestMove iterativeSearch(AllCurrPositions allCurrPositions, bool color, ZobristHash currZobristHash, double timeAvailable);
string convertMoveToJS(MoveDesc move);
EvalAndBestMove getMoveAndApplyFromPos(AllCurrPositions& allPositionBitboards, ZobristHash& currZobristHash, double timeLeft, bool color);
string posAndGameStateToJS(AllCurrPositions allPositionBitboards, EvalAndBestMove resultOfMinMaxSearch);

PosAndColor fenToPosBitboards(std::string fen);
char** allPositionBitboardsToMatrix(AllCurrPositions allPositionBitboardsL);
MoveDesc parseMove(json moveStr, AllCurrPositions allCurrPositions);
string convertVectorOfMovesToJs(vector<MoveDesc> moves);
double timeManagementFunction(double timeRemaining);
string convertMovesByPosToUCIMoves(MovesByPos moves);
string convertMovesVectorToUCIMoves(vector<MoveDesc> movesVector);

string squareToUCI(int pos);
string moveToUCI(const MoveDesc& move);

inline void setBitTo(Bitboard* initBB, int posX, int posY, bool value)
{
	if (value == 0)
	{
		*initBB &= ~(1ULL << posX + (posY * 8));
	}
	else
	{
		*initBB |= (1ULL << posX + (posY * 8));
	}
}
inline void setBitTo(Bitboard* initBB, int pos, bool value)
{
	if (value == 0)
	{
		*initBB &= ~(1ULL << pos);
	}
	else
	{
		*initBB |= (1ULL << pos);
	}
}

inline bool getBit(Bitboard bb, int posX, int posY)
{
	return (bb >> posX + (posY * 8)) & 1;
}
inline bool getBit(Bitboard bb, int pos)
{
	return (bb >> pos) & 1;
}

inline int amOfSetBits(Bitboard bitboard) {
	return __popcnt(bitboard);
}