#pragma once

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
using ZobristHash = uint64_t;

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
class EvalAndBestMove;
class BitboardAndPieceInfo;

class PieceTypeCurrPositions;
class OneColorCurrPositions;
class AllCurrPositions;

//vector<Bitboard> main2();
MoveCapAndPinnedBBs genBitboard(char piece, int x, int y, AllCurrPositions allCurrPositions, bool pseudo, bool currentColor, bool invertPawns = false, Bitboard oppColorAttackingSquares = 0);
array<Bitboard, 2> dirToBitboard(MoveMag dir, Bitboard oppColorPosBB, Bitboard thisColorPosBB, int x, int y, bool pseudo = false);
array<Bitboard, 2> genKingLegalMoves(Bitboard kingPseudoCapBitboard, Bitboard kingPseudoMoveBitboard, Bitboard oppColorPseudoAttackBB);
AllPosMoves fullMoveGenLoop(bool currentColor, AllCurrPositions& allPositionBitboards, ZobristHash& currZobristHash);
AttackingAndPinnedBBs firstPseudoMoves(AllCurrPositions allCurrPositions, bool currColor);
AllPosMoves secondPseudoMoves(int numOfCheck, vector<PinnedPieceData> pinnedPieces, AllCurrPositions allCurrPositions, bool currColor, CheckData checkData, int kingPos);
MoveMag kingOppDir(MoveMag dir, int kingPos);
CheckData checkChecks(AllCurrPositions allCurrPositions, bool currColor);
array<Bitboard, 2> pieceToPieceBitboard(MoveMag dir, int x, int y);

void calcCombinedPos(AllCurrPositions& allCurrPositions);
void calcCombinedMoves(AllPosMoves& posMoves);

bool checkBounds(int x, int y);
vector<Bitboard> arrayToVector(array<Bitboard, 2> arr);