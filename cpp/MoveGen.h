#pragma once

#include "iostream"
#include "string"
#include <vector>
#include <array>
//#include "Eigen/Core"
#include <cctype>

using namespace std;


class MoveDesc;
class SinglePiecePosMoves;
class PieceTypePosMoves;
class AllPosMoves;
class PinnedPieceData;
class AttackingAndPinnedBBs;
class MoveCapAndPinnedBBs;
class CheckData;
class LeafNodesAndCurrPos;
class PosAndColor;
struct EvalAndBestMove;
class BitboardAndPieceInfo;
struct MoveCapAndMoveDescs;
struct MoveCapAndMoveVector;

class AllCurrPositions;

using Bitboard = uint64_t;
using MoveMag = array<int, 3>;
using ZobristHash = uint64_t;
using MovesByPos = array<vector<MoveDesc>, 64>;

vector<MoveDesc> fullMoveGenLoop(bool colorToMove, AllCurrPositions& allCurrPositions, ZobristHash& currZobristHash);
AttackingAndPinnedBBs genAttackingAndPinned(AllCurrPositions allCurrPositions, bool currColor);
vector<MoveDesc> genAllLegalMoves(int numOfCheck, vector<PinnedPieceData> pinnedPieces, AllCurrPositions allCurrPositions, bool colorToMove, CheckData checkData, int kingPos);
MoveMag kingOppDir(MoveMag dir, int kingPos);
CheckData checkChecks(AllCurrPositions allCurrPositions, bool currColor);
array<Bitboard, 2> pieceToPieceBitboard(MoveMag dir, int x, int y);

MoveCapAndMoveDescs genPawnBitboard(AllCurrPositions allCurrPositions, bool colorToMove, bool pseudo);
MoveCapAndMoveDescs genKnightBitboard(AllCurrPositions allCurrPositions, bool colorToMove, bool pseudo, MovesByPos moves);
MoveCapAndMoveVector genPseudoKingBitboard(AllCurrPositions allCurrPositions, bool colorToMove, const Bitboard& kingPosBitboard);
MoveCapPinnedAndMoves genSlidingBitboard(AllCurrPositions allCurrPositions, bool colorToMove, bool pseudo, const DirectionBitboards(&PreCalculatedRays)[8][8], int pieceType, MovesByPos moves);

MovesByPos bitboardToMoves(Bitboard bitboard, bool pieceMovingColor, int pieceType, bool moveOrCapture, int dXApplied, int dYApplied, MovesByPos moves);
MovesByPos bitboardToMoves(Bitboard bitboard, bool pieceMovingColor, int pieceType, bool moveOrCapture, int pos, MovesByPos moves);
vector<MoveDesc> bitboardToMoveVector(Bitboard bitboard, bool pieceMovingColor, int pieceType, bool moveOrCapture, int pos);
bool goesIntoCheck(AllCurrPositions allCurrPositions, MoveDesc move, bool colorToMoveBeforeThisMove);

array<Bitboard, 2> genKingLegalMoves(Bitboard kingPseudoCapBitboard, Bitboard kingPseudoMoveBitboard, Bitboard oppColorPseudoAttackBB);
void calcCombinedPos(AllCurrPositions& allCurrPositions);
void calcCombinedMoves(AllPosMoves& posMoves);

bool checkBounds(int x, int y);
vector<Bitboard> arrayToVector(array<Bitboard, 2> arr);

inline vector<MoveDesc> addVectors(vector<MoveDesc> v1, vector<MoveDesc> v2) {
	if (v1.size() > v2.size()) {
		v1.insert(v1.end(), v2.begin(), v2.end());
		return v1;
	} else {
		v2.insert(v2.end(), v1.begin(), v1.end());
		return v2;
	}
}
inline vector<PinnedPieceData> addVectors(vector<PinnedPieceData> v1, vector<PinnedPieceData> v2) {
	if (v1.size() > v2.size()) {
		v1.insert(v1.end(), v2.begin(), v2.end());
		return v1;
	} else {
		v2.insert(v2.end(), v1.begin(), v1.end());
		return v2;
	}
}

