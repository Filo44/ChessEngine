#pragma once
#include "Chess.h"
#include "MoveGen.h"
#include "ZobristSeed.h"
#include <bitset>

extern int amountOfLeafNodes;
extern int captures;
extern int enPassant;
extern int totPos;
//extern int amOfEnPassantXORAdds;
//extern int amOfEnPassantXORRemovals;
extern int hypos;
extern unordered_map<ZobristHash, LeafNodesAndCurrPos> transpositionTablePerft;
extern unordered_map<ZobristHash, EvalAndBestMove> transpositionTable;


constexpr auto CAPTURE = 1;
constexpr auto MOVE = 0;

constexpr auto PIECE_TYPE = 0;
constexpr auto PIECE = 1;


using namespace std;

const char pieces[12] = { 'r','n','b','q','k','p','R','N','B','Q','K','P' };
extern std::unordered_map<char, int> pieceToNumber;

struct PieceInfo {
	int pieceType;
	int piece;
};

class MoveDesc {
public:
	bool pieceMovingColor;
	int pieceType;
	int posFrom;
	int posOfMove;
	bool moveOrCapture;
	bool nullMove = false;
};

class SinglePiecePosMoves {
public:
	Bitboard capBitboard;
	Bitboard moveBitboard;
	Bitboard posBitboard;
};

class PieceTypePosMoves {
public:
	char pieceType;
	vector<SinglePiecePosMoves> positionBitboard;
	//Maybe don't calculate this every time? Thus don't include it here. Don't know, may be worse.
	Bitboard pieceTypeCombinedCapBB;
	Bitboard pieceTypeCombinedMoveBB;
	vector<Bitboard> fetchBitboards(bool isMove) const {
		vector<Bitboard> res;
		for (const SinglePiecePosMoves singlePiece : positionBitboard) {
			if (isMove) {
				res.push_back(singlePiece.moveBitboard);
			}
			else {
				res.push_back(singlePiece.capBitboard);
			}
		}
		return res;
	}
};



class AllPosMoves {
public:
	PieceTypePosMoves pieceTypes[6];
	//Maybe don't calculate this every time? Thus don't include it here. Don't know, may be worse.
	Bitboard combinedCapBB;
	Bitboard combinedMoveBB;
};

//Stores position of the pinned Piece(pos) and the mask of the possible moves(poMoveMask)
class PinnedPieceData {
public:
	int pos;
	Bitboard pushMask;
	Bitboard captureMask;
};
//Stores a bitboard which "tells" you which squares the opponent is attacking(attacking) and lists pinned pieces(pinnedPieces)
class AttackingAndPinnedBBs {
public:
	Bitboard attacking;
	vector<PinnedPieceData> pinnedPieces;
};
//struct MoveCapAndPinnedBBs {
//	Bitboard moveBitboard;
//	Bitboard capBitboard;
//	vector<PinnedPieceData> pinnedPieces = {};
//	int enPassantCapPos = -1;
//};
struct MoveCapAndMoveDescs {
	Bitboard moveBitboard;
	Bitboard capBitboard;
	MovesByPos moves;
};
struct MoveCapAndMoveVector {
	Bitboard moveBitboard;
	Bitboard capBitboard;
	vector<MoveDesc> moves;
};
struct MoveCapPinnedAndMoves {
	Bitboard moveBitboard;
	Bitboard capBitboard;
	vector<PinnedPieceData> pinnedPieces = {};
	MovesByPos moves;
	int enPassantCapPos = -1;
};


class CheckData {
public:
	int numOfChecks;
	vector<BitboardAndPieceInfo> checkerLocations;
};
class BitboardAndPieceInfo {
public:
	Bitboard checkerBitboard;
	int pos;
	int normalizedPieceType;
};

struct CastlingRights {
	bool canCastleKSide;
	bool canCastleQSide;
};

class AllCurrPositions {
public:
	Bitboard pieceTypePositions[12];
	Bitboard allPiecesCombBitboard;
	Bitboard colorCombinedPosBitboard[2];
	int pawnWhoDoubleMovedPos = -1;
	CastlingRights castlingRights[2];
	int searchPieceType(int pos, bool color) {
		for (int i = color * 6; i < 6 + (color * 6); i++) {
			if (getBit(pieceTypePositions[i], pos)) {
				return i;
			}
		}

		cout << "I hate breakpoints" << endl;
		return -1;
	}

	ZobristHash applyMove(MoveDesc move, ZobristHash currZobristHash) {

		Bitboard& thisPieceTypeBitboard = pieceTypePositions[move.pieceType];
		//Buffers the application of the pawnWhoDoubleMovedPos such that we can delete and set the new pawnWhoDoubleMovedPos at the end while still being able to read it
		int pawnWhoDoubleMovedPosBuffer = -1;


		int moveToX = move.posOfMove % 8;
		int moveToY = move.posOfMove / 8;
		bool color = move.pieceMovingColor;
		bool moveOrCapture = move.moveOrCapture;
		int posFrom = move.posFrom;

		int forwards = (color) ? -1 : 1;

		int posFromX = posFrom % 8;
		int posFromY = posFrom / 8;

		bool enPassantThroughCapture = false;

		//Checks if a pawn has moved two squares, thus opening it up to en passant
		if (move.pieceType == (color ? whitePawn : blackPawn)) {
			if (abs((moveToY - posFromY)) == 2) {
				pawnWhoDoubleMovedPosBuffer = move.posOfMove;
				//cout << "Added(XORed) the number for file: " << moveToX << endl;
				//amOfEnPassantXORAdds++;
				currZobristHash ^= EnPassantFileSeed[moveToX];
			}
			//If it is a capture en passant you should move it one forwards
			else if (posFromY == moveToY) {
				//Can't just add forwards to moveToY because it removes the Piece using moveToY
				//Thus I have to make it when it is a en passant through capture move it adds forward to the Y component of where it adds the Piece
				enPassantThroughCapture = true;
				//cout << "Capture en passant sdfsfsd" << endl;

			}
		}
		else if (move.pieceType == (color ? whiteKing : blackKing)) {
			uint8_t initCastlingKey = 0b00000000;
			//Checks white
			if (castlingRights[1].canCastleKSide) {
				initCastlingKey |= (0b00000100);
			}
			if (castlingRights[1].canCastleQSide) {
				initCastlingKey |= (0b00001000);
			}
			//Checks black
			if (castlingRights[0].canCastleKSide) {
				initCastlingKey |= (0b00000001);
			}
			if (castlingRights[0].canCastleQSide) {
				initCastlingKey |= (0b00000010);
			}
			//Removes the old number
			currZobristHash ^= CastlingSeed[initCastlingKey];
			//Any king move, including castling, removes the right to castle.
			castlingRights[color].canCastleKSide = false;
			castlingRights[color].canCastleQSide = false;

			uint8_t castlingKey = 0b00000000;
			//Checks the opposite color, as this color is obviously going to not have any castling rights anymore
			//YES I am checking this twice, optimize later.
			int colorMult = !color ? 4 : 0;
			if (castlingRights[!color].canCastleKSide) {
				castlingKey |= (0b00000001 * colorMult);
			}
			if (castlingRights[!color].canCastleQSide) {
				castlingKey |= (0b00000010 * colorMult);
			}

			//Adds the new caslting number
			currZobristHash ^= CastlingSeed[castlingKey];

			bool castlingQSide = false;
			bool castlingKSide = false;
			if ((posFromX - moveToX) == 2) {
				castlingQSide = true;
			}
			else if ((posFromX - moveToX) == -2) {
				castlingKSide = true;
			}

			if (castlingQSide || castlingKSide) {
				//The rook moves more towards the middle, if castling queen side, king moves to the left(X dec) thus rook to the right(X inc)
				int rookMovingToX = moveToX + ((castlingQSide) ? 1 : -1);
				int rookMovingFromX = ((castlingQSide) ? 0 : 7);

				Bitboard& rooksBitboard = pieceTypePositions[color ? whiteRook : blackRook];
				setBitTo(&rooksBitboard, rookMovingFromX + (moveToY * 8), 0);
				setBitTo(&rooksBitboard, rookMovingToX + (moveToY * 8), 1);
			}

		}
		else if (move.pieceType == (color ? whiteRook : blackRook)) {
			//White's starting rank is at y=7. Check the pawn starting rank for proof
			int rookStartingRank = color ? 7 : 0;
			//If a rook moves from those squares it must mean that they disable that side's castling
			if (posFromY == rookStartingRank) {
				//Queen and king side have the same x for both sides
				if (posFromX == 0) {
					castlingRights[color].canCastleQSide = false;
				}
				else if (posFromX == 7) {
					castlingRights[color].canCastleKSide = false;
				}
			}
		}

		//Removes Piece in its initial location
		setBitTo(&thisPieceTypeBitboard, posFromX, posFromY, 0);
		currZobristHash ^= ZobristSeed[color ? move.pieceType + 6 : move.pieceType][posFrom];

		//Puts the Piece in its new location
		setBitTo(&thisPieceTypeBitboard, moveToX, moveToY + (enPassantThroughCapture ? forwards : 0), 1);
		currZobristHash ^= ZobristSeed[color ? move.pieceType + 6 : move.pieceType][move.posOfMove];

		//There are two ways of doing an en passant move, the move and the capture.
		//The following statements check whether it was a move en passant and change it to a takes en passent
		//Which the capture check will take
		if (move.pieceType == pieceToNumber['p'] && !moveOrCapture && moveToX != posFromX) {
			//cout << "Move en passant" << endl;
			moveOrCapture = CAPTURE;
			//Because if it is a move en passant you want to remove the Piece that is one square behind it.
			moveToY -= forwards;
		}

		//Removes Piece there
		if (moveOrCapture == CAPTURE) {

			//(moveToX + (moveToY*8))!=move.posOfMove OK?! BECAUSE IF IT EN PASSANT I ONLY CHANGE THE MOVETOY
			int toCapturePieceType = searchPieceType(moveToX + (moveToY * 8), !color);

			setBitTo(&pieceTypePositions[toCapturePieceType], moveToX + (moveToY * 8), 0);
			currZobristHash ^= ZobristSeed[toCapturePieceType][moveToX + (moveToY * 8)];
		}

		if (pawnWhoDoubleMovedPos != -1) {
			currZobristHash ^= EnPassantFileSeed[pawnWhoDoubleMovedPos % 8];
		}
		//Assigns the buffer, if the buffer hasn't been changed since instantiation it is -1 as that is the instantiated value.
		pawnWhoDoubleMovedPos = pawnWhoDoubleMovedPosBuffer;

		//Toggle turn
		currZobristHash ^= SideToMoveIsBlack;
		return currZobristHash;
	}
};

struct EvalAndBestMove {
	double eval;
	MoveDesc bestMove;
	int depth;
	bool abortedDueToTime = false;
	bool noMoves = false;
};

class LeafNodesAndCurrPos {
public:
	int leafNodes;
	int depth;
	//LeafNodesAndCurrPos(int leafNodesIn, char** allPositionMatrixIn) { // Constructor with parameters
	//	leafNodes = leafNodesIn;
	//	allPositionMatrix = allPositionMatrixIn;
	//}
};

class PosAndColor {
public:
	AllCurrPositions allCurrPositions;
	bool color;
};

struct MoveAndCapBitboards {
	Bitboard moveBitboard;
	Bitboard capBitboard;
};