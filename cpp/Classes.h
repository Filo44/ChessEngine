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

const char pieces[6] = { 'r','n','b','q', 'k','p' };
extern std::unordered_map<char, int> pieceToNumber;

struct PieceInfo {
	int pieceType;
	int piece;
};

class MoveDesc {
public:
	bool pieceMovingColor;
	int pieceType;
	int piece;
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
	vector<SinglePiecePosMoves> posBB;
	//Maybe don't calculate this every time? Thus don't include it here. Don't know, may be worse.
	Bitboard pieceTypeCombinedCapBB;
	Bitboard pieceTypeCombinedMoveBB;
	vector<Bitboard> fetchBitboards(bool isMove) const {
		vector<Bitboard> res;
		for (const SinglePiecePosMoves singlePiece : posBB) {
			if (isMove) {
				res.push_back(singlePiece.moveBitboard);
			} else {
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

//Stores position of the pinned piece(pos) and the mask of the possible moves(poMoveMask)
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
//MoveCapAndPinnedBBs
class MoveCapAndPinnedBBs {
public:
	Bitboard capBitboard;
	Bitboard moveBitboard;
	vector<PinnedPieceData> pinnedPieces;
	int enPassantCapPos = -1;
};

//Something else

class CheckData {
public:
	int numOfChecks;
	vector<BitboardAndPieceInfo> checkerLocations;
};
class BitboardAndPieceInfo {
public:
	Bitboard checkerBitboard;
	int pos;
	char pieceType;
};

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
	bool canCastleQSide = true;
	bool canCastleKSide = true;
};

class AllCurrPositions {
public:
	OneColorCurrPositions colorBitboards[2];
	Bitboard allPiecesCombBitboard;
	int pawnWhoDoubleMoved = -1;
	PieceInfo searchPieceByPos(int pos, bool colorOfPiece) {
		int pieceType = -1;
		for (int i = 0; i < 6; i++) {
			if (getBit(colorBitboards[colorOfPiece].pieceTypes[i].pieceTypeCombinedBB, pos)) {
				pieceType = i;
				break;
			}
		}
		if (pieceType == -1) {
			cout << "ERROR! piecetype = -1" << endl;
			cout << convertToString(allPositionBitboardsToMatrix(*this), 8, 8) << endl;
			cout << "Damn you breakpoints!" << endl;
		}
		int piece = -1;
		for (int j = colorBitboards[colorOfPiece].pieceTypes[pieceType].posBB.size() - 1; j >= 0; j--) {
			if (getBit(colorBitboards[colorOfPiece].pieceTypes[pieceType].posBB[j], pos)) {
				piece = j;
				break;
			}
		}
		if (piece == -1) {
			cout << "ERROR! piece = -1" << endl;
		}
		return { pieceType, piece };
	}
	int searchPieceByPosAndType(int pos, int pieceType, bool colorOfPiece) {
		int piece = -1;
		for (int j = colorBitboards[colorOfPiece].pieceTypes[pieceType].posBB.size() - 1; j >= 0; j--) {
			if (getBit(colorBitboards[colorOfPiece].pieceTypes[pieceType].posBB[j], pos)) {
				piece = j;
				break;
			}
		}
		return piece;
	}
	ZobristHash applyMove(MoveDesc move, ZobristHash currZobristHash) {
		////REMOVE AFTER
		string stringRepOfBoardBeforeChanges = convertToString(allPositionBitboardsToMatrix(*this), 8, 8);

		Bitboard& thisPieceBitboard = colorBitboards[move.pieceMovingColor].pieceTypes[move.pieceType].posBB[move.piece];
		int pieceLocation = _tzcnt_u64(thisPieceBitboard);
		//Buffers the application of the pawnWhoDoubleMoved such that we can delete and set the new pawnWhoDoubleMoved at the end while still being able to read it
		int pawnWhoDoubleMovedBuffer = -1;


		int moveToX = move.posOfMove % 8; 
		int moveToY = move.posOfMove / 8;
		bool color = move.pieceMovingColor;
		bool moveOrCapture = move.moveOrCapture;

		int forwards = (color) ? -1 : 1;

		int pieceLocationX = pieceLocation % 8;
		int pieceLocationY = pieceLocation / 8;
		
		bool enPassantThroughCapture = false;

		//Checks if a pawn has moved two squares, thus opening it up to en passant
		if (move.pieceType == pieceToNumber['p']) {
			if (abs((moveToY - pieceLocationY)) == 2) {
				pawnWhoDoubleMovedBuffer = move.piece;
				//cout << "Added(XORed) the number for file: " << moveToX << endl;
				//amOfEnPassantXORAdds++;
				currZobristHash ^= EnPassantFileSeed[moveToX];
			}
			//If it is a capture en passant you should move it one forwards
			else if (pieceLocationY == moveToY) {
				//Can't just add forwards to moveToY because it removes the piece using moveToY
				//Thus I have to make it when it is a en passant through capture move it adds forward to the Y component of where it adds the piece
				enPassantThroughCapture = true;
				//cout << "Capture en passant sdfsfsd" << endl;

			}
		}
		else if (move.pieceType == pieceToNumber['k']) {
			uint8_t initCastlingKey = 0b00000000;
			//Checks white
			if (colorBitboards[1].canCastleKSide) {
				initCastlingKey |= (0b00000100);
			}
			if (colorBitboards[1].canCastleQSide) {
				initCastlingKey |= (0b00001000);
			}
			//Checks black
			if (colorBitboards[0].canCastleKSide) {
				initCastlingKey |= (0b00000001);
			}
			if (colorBitboards[0].canCastleQSide) {
				initCastlingKey |= (0b00000010);
			}
			//Removes the old number
			currZobristHash ^= CastlingSeed[initCastlingKey];
			//Any king move, including castling, removes the right to castle.
			colorBitboards[color].canCastleKSide = false;
			colorBitboards[color].canCastleQSide = false;

			uint8_t castlingKey = 0b00000000;
			//Checks the opposite color, as this color is obviously going to not have any castling rights anymore
			//YES I am checking this twice, optimize later.
			int colorMult = !color ? 4 : 0;
			if (colorBitboards[!color].canCastleKSide) {
				castlingKey |= (0b00000001 * colorMult);
			}
			if (colorBitboards[!color].canCastleQSide) {
				castlingKey |= (0b00000010 * colorMult);
			}

			//Adds the new caslting number
			currZobristHash ^= CastlingSeed[castlingKey];

			bool castlingQSide = false;
			bool castlingKSide = false;
			if ((pieceLocationX - moveToX) == 2) {
				castlingQSide = true;
			}else if ((pieceLocationX - moveToX) == -2) {
				castlingKSide = true;
			}

			//cout << "castlingQSide: " << castlingQSide << endl;
			//cout << "castlingKSide: " << castlingKSide << endl;
			if (castlingQSide || castlingKSide) {
				//The rook moves more towards the middle, if castling queen side, king moves to the left(X dec) thus rook to the right(X inc)
				int rookMovingToX = moveToX + ((castlingQSide) ? 1 : -1);
				int rookMovingFromX = ((castlingQSide) ? 0 : 7);
				//cout << "rookMovingFromX: " << rookMovingFromX << endl;
				//int rookMovingToY = moveToY;
				vector<Bitboard>& rooks = colorBitboards[color].pieceTypes[pieceToNumber['r']].posBB;
				for (int i = 0; i < rooks.size(); i++) {
					Bitboard& singleRookBitboard = rooks[i];
					if (getBit(singleRookBitboard, rookMovingFromX, moveToY)) {
						singleRookBitboard = (1ULL << rookMovingToX + (moveToY * 8));
						break;
					}
				}
			}
			
		} 
		else if (move.pieceType == pieceToNumber['r']) {
			//White's starting rank is at y=7. Check the pawn starting rank for proof
			int rookStartingRank = color ? 7 : 0;
			//If a rook moves from those squares it must mean that they disable that side's castling
			if (pieceLocationY == rookStartingRank) {
				//Queen and king side have the same x for both sides
				if (pieceLocationX == 0) {
					colorBitboards[color].canCastleQSide = false;
				} else if (pieceLocationX == 7) {
					colorBitboards[color].canCastleKSide = false;
				}
			}
		}

		//Removes piece in its initial location
		setBitTo(&thisPieceBitboard, pieceLocationX, pieceLocationY, 0);
		//Obviously change this later, it will trip an error though
		currZobristHash ^= ZobristSeed[color ? move.pieceType + 6 : move.pieceType][pieceLocation];

		//Puts the piece in its new location
		setBitTo(&thisPieceBitboard, moveToX, moveToY + (enPassantThroughCapture ? forwards : 0), 1);
		currZobristHash ^= ZobristSeed[color ? move.pieceType + 6 : move.pieceType][move.posOfMove];

		//There are two ways of doing an en passant move, the move and the capture.
		//The following statements check whether it was a move en passant and change it to a takes en passent
		//Which the capture check will take
		if (move.pieceType == pieceToNumber['p'] && !moveOrCapture && moveToX != pieceLocationX) {
			//cout << "Move en passant" << endl;
			moveOrCapture = CAPTURE;
			//Because if it is a move en passant you want to remove the piece that is one square behind it.
			moveToY -= forwards;
		}
		 
		//Removes piece there
		if (moveOrCapture == CAPTURE) {

			//Not really a binary search, still close enough
			//Please do optimize if you(I speak to future-me in 2nd person) can

			//cout << "pieceType:" <<pieceType << endl;
			//cout << "piece:" <<piece << endl;
			//(moveToX + (moveToY*8))!=move.posOfMove OK?! BECAUSE IF IT EN PASSANT I ONLY CHANGE THE MOVETOY
			PieceInfo toCapturePiece = searchPieceByPos(moveToX + (moveToY*8), !color);

			PieceTypeCurrPositions& pieceTypePiece = colorBitboards[!color].pieceTypes[toCapturePiece.pieceType];
			//cout << "firstEl: " << pieceTypePiece.posBB[0] << endl;;
			pieceTypePiece.posBB.erase(pieceTypePiece.posBB.begin() + toCapturePiece.piece);
			currZobristHash ^= ZobristSeed[!color ? toCapturePiece.pieceType + 6 : toCapturePiece.pieceType][move.posOfMove];
		}

		int pawnWhoDoubleMovedI = pawnWhoDoubleMoved;
		if (pawnWhoDoubleMovedI != -1) {
			//White pawn currently means normal pawn
			int pawnWhoDoubleMovedPos = _tzcnt_u64(colorBitboards[move.pieceMovingColor].pieceTypes[whitePawn].posBB[pawnWhoDoubleMovedI]);
			//cout << "Removed(XORed): " << EnPassantFileSeed[pawnWhoDoubleMovedPos % 8] << endl;
			currZobristHash ^= EnPassantFileSeed[pawnWhoDoubleMovedPos % 8];
			//amOfEnPassantXORRemovals++;
		}
		//Assigns the buffer, if the buffer hasn't been changed since instantiation it is -1 as that is the instantiated value.
		pawnWhoDoubleMoved = pawnWhoDoubleMovedBuffer;


		////REMOVE AFTER
		//if (colorBitboards[0].pieceTypes[pieceToNumber['k']].posBB.size() == 0) {
		//	cout << "No more black king :/" << endl;
		//}else if (colorBitboards[1].pieceTypes[pieceToNumber['k']].posBB.size() == 0) {
		//	cout << "No more white king :/" << endl;
		//}

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