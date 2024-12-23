#pragma once
#include "Chess.h"
#include "MoveGen.h"
#include "ZobristSeed.h"
#include <bitset>

constexpr auto CAPTURE = 1;
constexpr auto MOVE = 0;

constexpr auto PIECE_TYPE = 0;
constexpr auto PIECE = 1;

constexpr auto BLACK = 0;
constexpr auto WHITE = 1;


using namespace std;

const char pieces[12] = { 'r','n','b','q','k','p','R','N','B','Q','K','P' };
extern std::unordered_map<char, int> pieceToNumber;

struct PieceInfo {
	int8 pieceType;
	int8 piece;
};

class MoveDesc {
public:
	bool pieceMovingColor;
	//Has color
	int8 pieceType;
	int8 posFrom;
	int8 posOfMove;
	bool moveOrCapture;
	//Has color
	int promotingToPiece = -1;
	bool enPassant = false;
	bool nullMove = false;

	int depthLeftAtLastSearch = -1;
	Eval prevEval;
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
	Bitboard pawnAttacking;
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
	vector<MoveDesc> moves;
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
	vector<MoveDesc> moves;
	int enPassantCapPos = -1;
};
struct MovesVectAndPawnAtt {
	vector<MoveDesc> moves;
	Bitboard pawnAttacking;
};


class CheckData {
public:
	int8 numOfChecks;
	vector<BitboardAndPieceInfo> checkerLocations;
};
class BitboardAndPieceInfo {
public:
	Bitboard checkerBitboard;
	int8 pos;
	int8 normalizedPieceType;
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
	int searchPieceType(int pos, bool colorOfPiece) {
		for (int i = colorOfPiece * 6; i < 6 + (colorOfPiece * 6); i++) {
			if (getBit(pieceTypePositions[i], pos)) {
				return i;
			}
		}

		cout << "I hate breakpoints" << endl;
		return -1;
	}
	uint8_t getStartingCastlingKey() const {
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
		return initCastlingKey;
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
		int8 normalizedPieceType = move.pieceType > 5 ? move.pieceType - 6 : move.pieceType;

		int forwards = (color) ? -1 : 1;

		int posFromX = posFrom % 8;
		int posFromY = posFrom / 8;

		//Checks if a pawn has moved two squares, thus opening it up to en passant
		if (move.pieceType == (color ? whitePawn : blackPawn)) {
			if (abs((moveToY - posFromY)) == 2) {
				pawnWhoDoubleMovedPosBuffer = move.posOfMove;
				//cout << "Added(XORed) the number for file: " << moveToX << endl;
				//amOfEnPassantXORAdds++;
				currZobristHash ^= EnPassantFileSeed[moveToX];
			}
		}
		else if (move.pieceType == (color ? whiteKing : blackKing)) {
			//THERE IS A CODE DUPLICATE IN THE FOLLOWING ELSE IF STATEMENT, IF YOU MAKE CHANGES HERE, DUPLIATE THOSE CHANGES THERE
			uint8_t initCastlingKey = getStartingCastlingKey();

			currZobristHash ^= CastlingSeed[initCastlingKey];
			//Any king move, including castling, removes the right to castle.
			castlingRights[color].canCastleKSide = false;
			castlingRights[color].canCastleQSide = false;

			uint8_t castlingKey = 0b00000000;
			//Checks the opposite color, as this color is obviously going to not have any castling rights anymore
			//YES I am checking this twice, optimize later.
			int colorMult = !color ? 4 : 1;
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
				//Removes the rooks pos hash from the currZobristHash:
				setBitTo(&rooksBitboard, rookMovingFromX + (moveToY * 8), 0);
				currZobristHash ^= ZobristSeed[color ? whiteRook : blackRook][rookMovingFromX + (moveToY * 8)];

				//Adds the rook
				setBitTo(&rooksBitboard, rookMovingToX + (moveToY * 8), 1);
				currZobristHash ^= ZobristSeed[color ? whiteRook : blackRook][rookMovingToX + (moveToY * 8)];
			}

		}
		else if (move.pieceType == (color ? whiteRook : blackRook)) {
			//White's starting rank is at y=7. Check the pawn starting rank for proof
			int rookStartingRank = color ? 7 : 0;
			//If a rook moves from those squares it must mean that they disable that side's castling
			if (posFromY == rookStartingRank) {
				//Queen and king side have the same x for both sides
				uint8_t castlingKey = 0b11111111;
				if (castlingRights[color].canCastleKSide || castlingRights[color].canCastleQSide) {
					uint8_t initCastlingKey = getStartingCastlingKey();
					//Removes the old number
					currZobristHash ^= CastlingSeed[initCastlingKey];
					castlingKey = initCastlingKey;
				}

				int colorMult = color ? 4 : 1;
				if (posFromX == 0 && castlingRights[color].canCastleQSide) {
					//Turns off the bit in the position which indicates color's queen side castling right.
					castlingKey &= ~(0b00000010 * colorMult);
					castlingRights[color].canCastleQSide = false;
				}
				else if (posFromX == 7 && castlingRights[color].canCastleKSide) {
					//Turns off the bit in the position which indicates color's king side castling right.
					castlingKey &= ~(0b00000001 * colorMult);
					castlingRights[color].canCastleKSide = false;
				}
				if (castlingKey != 0b11111111) {
					//Castling key cannot be equal to that if it triggered the first if statement
					currZobristHash ^= CastlingSeed[castlingKey];
				}
			}
		}

		//Removes Piece in its initial location
		setBitTo(&thisPieceTypeBitboard, posFromX, posFromY, 0);
		currZobristHash ^= ZobristSeed[move.pieceType][posFrom];

		if (move.promotingToPiece == -1) {
			//Puts the Piece in its new location. If it is en passant(Now using cap and pos of the piece capturing):
			//Move the moveTo forwards in the y axis.
			setBitTo(&thisPieceTypeBitboard, moveToX, moveToY + (move.enPassant ? forwards : 0), 1);
			currZobristHash ^= ZobristSeed[move.pieceType][move.posOfMove + (move.enPassant ? forwards * 8 : 0)];
		}
		else {
			//Promotion, can't be en passant. move.promotingToPiece has color info.
			setBitTo(&pieceTypePositions[move.promotingToPiece], moveToX, moveToY, 1);
			currZobristHash ^= ZobristSeed[move.promotingToPiece][move.posOfMove];
		}

		//Removes Piece there
		if (moveOrCapture == CAPTURE) {

			//(moveToX + (moveToY*8))!=move.posOfMove OK?! BECAUSE IF IT EN PASSANT I ONLY CHANGE THE MOVETOY
			int toCapturePieceType = searchPieceType(moveToX + (moveToY * 8), !color);
			int oppRookStartingRank = !color ? 7 : 0;
			if (toCapturePieceType == blackRook || toCapturePieceType == whiteRook) {
				if (moveToY == oppRookStartingRank) {
					if (moveToX == 0) {
						castlingRights[!color].canCastleQSide = false;
					}
					if (moveToX == 7) {
						castlingRights[!color].canCastleKSide = false;
					}
				}
			}

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

struct EvalAndDepthLeft {
	Eval eval;
	int depthLeftAtLastSearch;
};

struct EvalAndBestMove {
	Eval eval;
	MoveDesc bestMove;
	int8 depth;
	bool abortedDueToTime = false;
	bool noMoves = false;
	vector<EvalAndDepthLeft> evalsPerMove = {};
};

class LeafNodesAndCurrPos {
public:
	int8 leafNodes;
	int8 depth;
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