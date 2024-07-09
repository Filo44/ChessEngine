#pragma once
#ifndef MoveGen_H
#define MoveGen_H

#include "iostream"
#include "string"
#include <vector>
#include <array>
//#include "Eigen/Core"
#include <cctype>

using namespace std;

//using Bitboard = Eigen::Matrix<bool, 8, 8>;
using Bitboard = uint64_t;
using MoveMag = array<int, 3>;

//vector<Bitboard> main2();
MoveCapAndPinnedBBs genBitboard(char piece, int x, int y, AllCurrPositions allCurrPositions, bool pseudo, bool currentColor);
array<Bitboard, 2> dirToBitboard(MoveMag dir, Bitboard oppColorPosBB, Bitboard thisColorPosBB, int x, int y);
array<Bitboard, 2> genKingLegalMoves(Bitboard kingPseudoCapBitboard, Bitboard kingPseudoMoveBitboard, Bitboard oppColorPseudoAttackBB);
AllPosMoves fullMoveGenLoop(bool currentColor, AllCurrPositions allPositionBitboards);
AttackingAndPinnedBBs firstPseudoMoves(AllCurrPositions allCurrPositions, bool currColor);
AllPosMoves secondPseudoMoves(int numOfCheck, vector<PinnedPieceData> pinnedPieces, AllCurrPositions allCurrPositions, bool currColor, CheckData checkData, int kingPos);
MoveMag kingOppDir(MoveMag dir, int kingPos);
CheckData checkChecks(AllCurrPositions allCurrPositions, bool currColor);

void calcCombinedPos(AllCurrPositions& allCurrPositions);
void calcCombinedMoves(AllPosMoves& posMoves);

bool checkBounds(int x, int y);
vector<Bitboard> arrayToVector(array<Bitboard, 2> arr);

class singlePiecePosMoves;
class PieceTypePosMoves;
class AllPosMoves;

class SinglePiecePosMoves {
public:
	Bitboard capBitboard;
	Bitboard moveBitboard;
	Bitboard posBitboard;
};

class PieceTypePosMoves {
public:
	char pieceType;
	vector<SinglePiecePosMoves> posBB;
	//Maybe don't calculate this every time? Thus don't include it here. Don't know, may be worse.
	Bitboard pieceTypeCombinedCapBB;
	Bitboard pieceTypeCombinedMoveBB;

};



class AllPosMoves {
public:
	PieceTypePosMoves pieceTypes[6];
	//Maybe don't calculate this every time? Thus don't include it here. Don't know, may be worse.
	Bitboard combinedCapBB;
	Bitboard combinedMoveBB;
};

//Stores position of the pinned piece(pos) and the mask of the possible moves(poMoveMask)
class PinnedPieceData {
public:
	int pos;
	Bitboard posMoveMask;
};
//Stores a bitboard which "tells" you which squares the opponent is attacking(attacking) and lists pinned pieces(pinnedPieces)
class AttackingAndPinnedBBs {
public:
	Bitboard attacking; 
	vector<PinnedPieceData> pinnedPieces;
};
//MoveCapAndPinnedBBs
class MoveCapAndPinnedBBs {
public:
	Bitboard capBitboard;
	Bitboard moveBitboard;
	vector<PinnedPieceData> pinnedPieces;
};

class CheckData {
public: 
	int numOfChecks;
	vector<Bitboard> checkerLocations;
};

#endif