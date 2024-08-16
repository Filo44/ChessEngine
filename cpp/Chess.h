#pragma once

#include <iostream>
#include "httplib.h"
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <unordered_map>
#include <ctime>

using namespace nlohmann;

enum piece {
	whiteRook = 0,
	whiteKnight = 1,
	whiteBishop = 2,
	whiteQueen = 3,
	whiteKing = 4,
	whitePawn = 5,
	blackRook = 6,
	blackKnight = 7,
	blackBishop = 8,
	blackQueen = 9,
	blackKing = 10,
	blackPawn = 11,
};

using namespace std;
using Bitboard = uint64_t;
using ZobristHash = uint64_t;

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

// char** fenToMatrix(std::string fen);
void delete2DArray(char **arr, int rows);
string convertToString(char **arr, int cols, int rows);
string convertToJSArr(char **arr, int cols, int rows);
string convertVofBBJS(vector<Bitboard> matrixVector);
stringstream convertBBJS(Bitboard curBB);
string allPosMovesToMatrix(AllPosMoves posMoves);
ZobristHash genInitZobristHash(AllCurrPositions currPositions);
EvalAndBestMove iterativeSearch(AllCurrPositions allCurrPositions, bool color, ZobristHash currZobristHash, double timeAvailable);
string convertMoveToJS(MoveDesc move);
EvalAndBestMove getMoveAndApplyFromPos(AllCurrPositions& allPositionBitboards, ZobristHash& currZobristHash, double timeLeft, bool color);
string posAndGameStateToJS(AllCurrPositions allPositionBitboards, EvalAndBestMove resultOfMinMaxSearch); 

PosAndColor fenToPosBitboards(std::string fen);
char **allPositionBitboardsToMatrix(AllCurrPositions allPositionBitboardsL);
MoveDesc parseMove(json moveStr, AllCurrPositions allCurrPositions);
double timeManagementFunction(double timeRemaining);

inline void setBitTo(Bitboard *initBB, int posX, int posY, bool value)
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
	} else
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