#pragma once

#include "iostream"
#include "string"
#include <vector>
#include <array>
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
struct MoveCapPinnedAndMoves;
struct MoveAndCapBitboards;
struct MovesVectAndPawnAtt;

class AllCurrPositions;

using Bitboard = uint64_t;
using MoveMag = array<int, 3>;
using ZobristHash = uint64_t;
using MovesByPos = array<vector<MoveDesc>, 64>;
using DirectionBitboards = array<Bitboard, 4>;

MovesVectAndPawnAtt fullMoveGenLoop(bool colorToMove, AllCurrPositions& allCurrPositions, ZobristHash& currZobristHash);
AttackingAndPinnedBBs genAttackingAndPinned(AllCurrPositions allCurrPositions, bool currColor, int kingPos);
vector<MoveDesc> genAllLegalMoves(int numOfCheck, vector<PinnedPieceData> pinnedPieces, AllCurrPositions allCurrPositions, bool colorToMove, CheckData checkData, int kingPos);
MoveMag kingOppDir(MoveMag dir, int kingPos);
CheckData checkChecks(AllCurrPositions allCurrPositions, bool currColor);
array<Bitboard, 2> pieceToPieceBitboard(MoveMag dir, int x, int y);

MoveCapAndMoveDescs genPawnBitboard(AllCurrPositions allCurrPositions, bool colorToMove, bool pseudo);
MoveCapAndMoveDescs genKnightBitboard(AllCurrPositions allCurrPositions, bool colorToMove, bool pseudo, MovesByPos& moves);
MoveAndCapBitboards genPseudoKingBitboard(AllCurrPositions allCurrPositions, bool colorToMove, const Bitboard& kingPosBitboard, bool pseudo);
Bitboard genCastlingMoves(AllCurrPositions allCurrPositions, bool colorToMove, const Bitboard& kingPosBitboard, Bitboard oppAttacking);
MoveCapPinnedAndMoves genSlidingBitboard(AllCurrPositions allCurrPositions, bool colorToMove, bool pseudo, const DirectionBitboards(&PreCalculatedRays)[8][8], int pieceType, MovesByPos& moves, int oppKingPos = -1);

void bitboardToMoves(Bitboard bitboard, bool pieceMovingColor, int pieceType, bool moveOrCapture, int dXApplied, int dYApplied, MovesByPos& moves);
void bitboardToMoves(Bitboard bitboard, bool pieceMovingColor, int pieceType, bool moveOrCapture, int pos, MovesByPos& moves);
void bitboardToPromotionMoves(Bitboard bitboard, bool pieceMovingColor, bool moveOrCapture, int dXApplied, int dYApplied, MovesByPos& moves);
vector<MoveDesc> bitboardToMoveVector(Bitboard bitboard, bool pieceMovingColor, int pieceType, bool moveOrCapture, int pos);
bool goesIntoCheck(AllCurrPositions allCurrPositions, MoveDesc move, bool colorToMoveBeforeThisMove);

MoveAndCapBitboards genKingLegalMoves(Bitboard kingPseudoCapBitboard, Bitboard kingPseudoMoveBitboard, Bitboard oppColorPseudoAttackBB);
void calcCombinedPos(AllCurrPositions& allCurrPositions);
void calcCombinedMoves(AllPosMoves& posMoves);

bool checkBounds(int x, int y);
vector<Bitboard> arrayToVector(array<Bitboard, 2> arr);

inline void addVectors(vector<MoveDesc>& v1, vector<MoveDesc>& v2) {
	v2.insert(v2.end(), v1.begin(), v1.end());
}
inline void addVectorsCopyFirst(vector<MoveDesc> v1, vector<MoveDesc>& v2) {
	v2.insert(v2.end(), v1.begin(), v1.end());
}
inline void addVectors(vector<PinnedPieceData>& v1, vector<PinnedPieceData>& v2) {
	v2.insert(v2.end(), v1.begin(), v1.end());
}

