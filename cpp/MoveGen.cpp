#include "Chess.h"
#include "MoveGen.h"
#include "Classes.h"
#include "SearchAlgorithm.h"
#include "ZobristSeed.h"
#include <bitset>

using namespace std;

//NOT THE SECOND RANK, one rank above (Look at pawn move gen for explanation why)
constexpr Bitboard whiteDoubleRank = 0xff0000000000;
//NOT THE SIXTH RANK, one rank above (Look at pawn move gen for explanation why)
constexpr Bitboard blackDoubleRank = 0xff0000;
constexpr Bitboard file[8] = { 0x101010101010101, 0x202020202020202, 0x404040404040404, 0x808080808080808, 0x1010101010101010, 0x2020202020202020, 0x4040404040404040, 0x8080808080808080 };
constexpr Bitboard cRank[8] = { 0xff, 0xff00, 0xff0000, 0xff000000, 0xff00000000, 0xff0000000000, 0xff000000000000, 0xff00000000000000 };

unordered_map<char, int> pieceToNumber = {
		{'r', 0},
		{'n', 1},
		{'b', 2},
		{'q', 3},
		{'k', 4},
		{'p', 5},
		{'R', 6},
		{'N', 7},
		{'B', 8},
		{'Q', 9},
		{'K', 10},
		{'P', 11}
};

MoveCapAndMoveDescs genPawnBitboard(AllCurrPositions allCurrPositions, bool colorToMove, const bool pseudo) {
	const int pieceType = colorToMove ? whitePawn : blackPawn;
	const Bitboard& pawnPosBitboard = allCurrPositions.pieceTypePositions[pieceType];
	const int& pawnWhoDoubleMovedPos = allCurrPositions.pawnWhoDoubleMovedPos;
	Bitboard posCombinedBitboard = allCurrPositions.allPiecesCombBitboard;
	Bitboard oppColorCombinedBitboard = allCurrPositions.colorCombinedPosBitboard[!colorToMove];

	Bitboard pawnCapBitboard = 0;
	Bitboard pawnMoveBitboard = 0;
	Bitboard bufferBitboard = 0;

	vector<MoveDesc> moves = {};
	const int yMult = colorToMove ? -1 : 1;

	if (!pseudo) {
		const Bitboard& lastRank = colorToMove ? cRank[0] : cRank[7];
		//One square push
		pawnMoveBitboard = (colorToMove ? ((pawnPosBitboard & ~lastRank) >> 8) : ((pawnPosBitboard & ~lastRank) << 8)) & ~posCombinedBitboard;
		bitboardToPromotionMoves(pawnMoveBitboard & lastRank, colorToMove, MOVE, 0, 1 * yMult, moves);
		pawnMoveBitboard &= ~lastRank;
		bitboardToMoves(pawnMoveBitboard, colorToMove, pieceType, MOVE, 0, 1 * yMult, moves);
		//Then it "checks" (AND it with a bitboard of those ranks) whether they are in the 3rd or 6th rank after being moved once,
		//  thus meaning they were originally in the first rank
		//Then takes away the ones which have just moved into a piece
		bufferBitboard = (colorToMove ? (pawnMoveBitboard & whiteDoubleRank) >> 8 : (pawnMoveBitboard & blackDoubleRank) << 8) & ~posCombinedBitboard;
		bitboardToMoves(bufferBitboard, colorToMove, pieceType, MOVE, 0, 2 * yMult, moves);
		pawnMoveBitboard |= bufferBitboard;

		//En passant
		//En passant towards lower files
		Bitboard enPassantCapBitboard = ((pawnPosBitboard & ~file[0]) >> 1) & (1ULL << pawnWhoDoubleMovedPos);
		//En passant towards higher files(Diff variable because I need to check whether it was to the right or left after to get the pos of the piece that is en passant-ing)
		Bitboard higherFileEnPassantCapBitboard = ((pawnPosBitboard & ~file[7]) << 1) & (1ULL << pawnWhoDoubleMovedPos);
		enPassantCapBitboard |= higherFileEnPassantCapBitboard;

		if (enPassantCapBitboard != 0) {
			//Bitboard enPassantMoveBitboard = colorToMove ? (enPassantCapBitboard >> 8) : (enPassantCapBitboard << 8);
			int posOfEnPassantMove = _tzcnt_u64(enPassantCapBitboard);

			MoveDesc enPassantMove;
			enPassantMove.pieceMovingColor = colorToMove;
			enPassantMove.moveOrCapture = CAPTURE;
			enPassantMove.enPassant = true;
			enPassantMove.pieceType = colorToMove ? whitePawn : blackPawn;
			enPassantMove.posFrom = _tzcnt_u64(higherFileEnPassantCapBitboard != 0 ? enPassantCapBitboard >> 1 : enPassantCapBitboard << 1);
			enPassantMove.posOfMove = posOfEnPassantMove;

			if (!goesIntoCheck(allCurrPositions, enPassantMove, colorToMove)) {
				//Adding the enPassant moves to the bitboards
				moves.push_back(enPassantMove);
			}
		}

		//To the lower file capture moves:
		pawnCapBitboard = (((colorToMove ? ((pawnPosBitboard & ~cRank[0]) >> 8) : ((pawnPosBitboard & ~cRank[7]) << 8)) & ~file[0]) >> 1) & oppColorCombinedBitboard;
		bitboardToPromotionMoves(pawnCapBitboard & lastRank, colorToMove, CAPTURE, -1, 1 * yMult, moves);
		pawnCapBitboard &= ~lastRank;

		//To the higher file capture moves(Same thing just exclude the 8th/n=7 file and shift one left instead of one right):
		bufferBitboard = (((colorToMove ? ((pawnPosBitboard & ~cRank[0]) >> 8) : ((pawnPosBitboard & ~cRank[7]) << 8)) & ~file[7]) << 1) & oppColorCombinedBitboard;
		bitboardToPromotionMoves(bufferBitboard & lastRank, colorToMove, CAPTURE, 1, 1 * yMult, moves);
		bufferBitboard &= ~lastRank;

		bitboardToMoves(pawnCapBitboard, colorToMove, pieceType, CAPTURE, -1, 1 * yMult, moves);
		bitboardToMoves(bufferBitboard, colorToMove, pieceType, CAPTURE, 1, 1 * yMult, moves);
		pawnCapBitboard |= bufferBitboard;
	}
	else {
		//To the lower file capture moves:
		pawnCapBitboard = ((colorToMove ? ((pawnPosBitboard & ~file[0] & ~cRank[0]) >> 8) : ((pawnPosBitboard & ~file[0] & ~cRank[7]) << 8)) >> 1);

		//To the higher file capture moves(Same thing just exclude the 8th/n=7 file and shift one left instead of one right):
		bufferBitboard = ((colorToMove ? ((pawnPosBitboard & ~file[7] & ~cRank[0]) >> 8) : ((pawnPosBitboard & ~file[7] & ~cRank[7]) << 8)) << 1);
		pawnCapBitboard |= bufferBitboard;
	}

	return { pawnMoveBitboard, pawnCapBitboard, moves };
}
MoveCapAndMoveDescs genKnightBitboard(AllCurrPositions allCurrPositions, bool colorToMove, const bool pseudo, vector<MoveDesc>& moves) {
	const int pieceType = colorToMove ? whiteKnight : blackKnight;
	const Bitboard& knightPosBitboard = allCurrPositions.pieceTypePositions[pieceType];
	Bitboard posCombinedBitboard = allCurrPositions.allPiecesCombBitboard;
	Bitboard oppColorCombinedBitboard = allCurrPositions.colorCombinedPosBitboard[!colorToMove];

	Bitboard knightMoveBitboard = 0;
	Bitboard knightCapBitboard = 0;
	Bitboard bufferBitboard;

	//dX=-2, dY=-1. Thus >> 2 + 8 = 10
	knightMoveBitboard = (knightPosBitboard & ~file[0] & ~file[1] & ~cRank[0]) >> 10;
	if (!pseudo) {
		bitboardToMoves(knightMoveBitboard & ~posCombinedBitboard, colorToMove, pieceType, MOVE, -2, -1, moves);
		bitboardToMoves(knightMoveBitboard & oppColorCombinedBitboard, colorToMove, pieceType, CAPTURE, -2, -1, moves);
	}
	//dX=-2, dY=1. Thus << -2 + 8 = 6
	bufferBitboard = (knightPosBitboard & ~file[0] & ~file[1] & ~cRank[7]) << 6;
	knightMoveBitboard |= bufferBitboard;
	if (!pseudo) {
		bitboardToMoves(bufferBitboard & ~posCombinedBitboard, colorToMove, pieceType, MOVE, -2, 1, moves);
		bitboardToMoves(bufferBitboard & oppColorCombinedBitboard, colorToMove, pieceType, CAPTURE, -2, 1, moves);
	}
	//dX=2, dY=-1. Thus >> -2 + 8 = 6
	bufferBitboard = (knightPosBitboard & ~file[6] & ~file[7] & ~cRank[0]) >> 6;
	knightMoveBitboard |= bufferBitboard;
	if (!pseudo) {
		bitboardToMoves(bufferBitboard & ~posCombinedBitboard, colorToMove, pieceType, MOVE, 2, -1, moves);
		bitboardToMoves(bufferBitboard & oppColorCombinedBitboard, colorToMove, pieceType, CAPTURE, 2, -1, moves);
	}
	//dX=2, dY=1. Thus << 2 + 8 = 10
	bufferBitboard = (knightPosBitboard & ~file[6] & ~file[7] & ~cRank[7]) << 10;
	knightMoveBitboard |= bufferBitboard;
	if (!pseudo) {
		bitboardToMoves(bufferBitboard & ~posCombinedBitboard, colorToMove, pieceType, MOVE, 2, 1, moves);
		bitboardToMoves(bufferBitboard & oppColorCombinedBitboard, colorToMove, pieceType, CAPTURE, 2, 1, moves);
	}

	//dX=-1, dY=-2. Thus >> 1 + 16 = 17
	bufferBitboard = (knightPosBitboard & ~cRank[0] & ~cRank[1] & ~file[0]) >> 17;
	knightMoveBitboard |= bufferBitboard;
	if (!pseudo) {
		bitboardToMoves(bufferBitboard & ~posCombinedBitboard, colorToMove, pieceType, MOVE, -1, -2, moves);
		bitboardToMoves(bufferBitboard & oppColorCombinedBitboard, colorToMove, pieceType, CAPTURE, -1, -2, moves);
	}
	//dX=-1, dY=2. Thus << -1 + 16 = 15
	bufferBitboard = (knightPosBitboard & ~cRank[6] & ~cRank[7] & ~file[0]) << 15;
	knightMoveBitboard |= bufferBitboard;
	if (!pseudo) {
		bitboardToMoves(bufferBitboard & ~posCombinedBitboard, colorToMove, pieceType, MOVE, -1, 2, moves);
		bitboardToMoves(bufferBitboard & oppColorCombinedBitboard, colorToMove, pieceType, CAPTURE, -1, 2, moves);
	}
	//dX=1, dY=-2. Thus >> -1 + 16 = 15
	bufferBitboard = (knightPosBitboard & ~cRank[0] & ~cRank[1] & ~file[7]) >> 15;
	knightMoveBitboard |= bufferBitboard;
	if (!pseudo) {
		bitboardToMoves(bufferBitboard & ~posCombinedBitboard, colorToMove, pieceType, MOVE, 1, -2, moves);
		bitboardToMoves(bufferBitboard & oppColorCombinedBitboard, colorToMove, pieceType, CAPTURE, 1, -2, moves);
	}
	//dX=1, dY=2. Thus << 1 + 16 = 17
	bufferBitboard = (knightPosBitboard & ~cRank[6] & ~cRank[7] & ~file[7]) << 17;
	knightMoveBitboard |= bufferBitboard;
	if (!pseudo) {
		bitboardToMoves(bufferBitboard & ~posCombinedBitboard, colorToMove, pieceType, MOVE, 1, 2, moves);
		bitboardToMoves(bufferBitboard & oppColorCombinedBitboard, colorToMove, pieceType, CAPTURE, 1, 2, moves);

		knightCapBitboard = knightMoveBitboard & oppColorCombinedBitboard;
		knightMoveBitboard &= ~posCombinedBitboard;
	}

	return { knightMoveBitboard, knightCapBitboard };
}
MoveAndCapBitboards genPseudoKingBitboard(AllCurrPositions allCurrPositions, bool colorToMove, const Bitboard& kingPosBitboard, const bool pseudo) {
	const int pieceType = colorToMove ? whiteKing : blackKing;

	Bitboard posCombinedBitboard = allCurrPositions.allPiecesCombBitboard;
	Bitboard oppColorCombinedBitboard = allCurrPositions.colorCombinedPosBitboard[!colorToMove];

	Bitboard kingMoveBitboard = 0;
	Bitboard kingCapBitboard = 0;

	//dX = -1 dY = 0 thus >> 1
	kingMoveBitboard = (kingPosBitboard & ~file[0]) >> 1;

	//dX = -1 dY = -1 thus >> 1 + 8 = 9
	kingMoveBitboard |= (kingPosBitboard & ~file[0] & ~cRank[0]) >> 9;
	//dX = -1 dY = 1 thus << -1 + 8 = 7
	kingMoveBitboard |= (kingPosBitboard & ~file[0] & ~cRank[7]) << 7;

	//dX = 0 dY = -1 thus >> 8
	kingMoveBitboard |= (kingPosBitboard & ~cRank[0]) >> 8;
	//dX = 0 dY = 1 thus << 8 
	kingMoveBitboard |= (kingPosBitboard & ~cRank[7]) << 8;

	//dX = 1 dY = 0 thus << 1
	kingMoveBitboard |= (kingPosBitboard & ~file[7]) << 1;
	//dX = 1 dY = -1 thus >> -1 + 8 = 7
	kingMoveBitboard |= (kingPosBitboard & ~file[7] & ~cRank[0]) >> 7;
	//dX = 1 dY = 1 thus << 1 + 8 = 9
	kingMoveBitboard |= (kingPosBitboard & ~file[7] & ~cRank[7]) << 9;

	if (!pseudo) {
		kingCapBitboard = kingMoveBitboard & oppColorCombinedBitboard;
		kingMoveBitboard &= ~posCombinedBitboard;
	}

	return { kingMoveBitboard, kingCapBitboard };
}
Bitboard genCastlingMoves(AllCurrPositions allCurrPositions, bool colorToMove, const Bitboard& kingPosBitboard, Bitboard oppAttacking) {
	Bitboard& allPosCombined = allCurrPositions.allPiecesCombBitboard;
	Bitboard res = 0;
	//Pseudo, although not passed as a parameter, is known to be false
	if (allCurrPositions.castlingRights[colorToMove].canCastleQSide) {
		//This means that the king and the queen-side rook haven't been moved.
		//CHECKS QUEENSIDECASTLINGKINGMOVEMENT and not castling ray because it can't move only if a piece is attacking a square through which the king moves, 
		// which for castling queenside isn't the same as the castling ray
		if ((castlingRays[colorToMove][QUEENSIDE] & allPosCombined) == 0 && (queenSideCastlingKingMovement[colorToMove] & oppAttacking) == 0) {
			//If either the castling ray and a piece overlap or the castling ray and the oppAttacking bitboard overlap it means you can't castle.
			//Shifts are inverted, i.e. shift right 2 means it moves the bits in the bitboard left 2.
			res = kingPosBitboard >> 2;
		}
	}
	if (allCurrPositions.castlingRights[colorToMove].canCastleKSide) {
		//This means that the king and the king-side rook haven't been moved.
		if ((castlingRays[colorToMove][KINGSIDE] & allPosCombined) == 0 && (castlingRays[colorToMove][KINGSIDE] & oppAttacking) == 0) {
			//If either the castling ray and a piece overlap or the castling ray and the oppAttacking bitboard overlap it means you can't castle.
			//Shifts are inverted, i.e. shift left 2 means it moves the bits in the bitboard right 2.
			res |= kingPosBitboard << 2;
		}
	}
	return { res };
}
MoveCapPinnedAndMoves genSlidingBitboard(AllCurrPositions allCurrPositions, bool colorToMove, const bool pseudo, const DirectionBitboards(&PreCalculatedRays)[8][8], int pieceType, vector<MoveDesc>& moves, int oppKingPos) {
	Bitboard slidingPiecePosBitboard = allCurrPositions.pieceTypePositions[pieceType];
	Bitboard posCombinedBitboard = allCurrPositions.allPiecesCombBitboard;
	Bitboard oppColorCombinedBitboard = allCurrPositions.colorCombinedPosBitboard[!colorToMove];

	Bitboard moveBitboard = 0;
	Bitboard capBitboard = 0;
	Bitboard bufferBitboard;
	vector<PinnedPieceData> pinnedPieces = {};

	while (slidingPiecePosBitboard != 0) {
		bufferBitboard = 0;
		int pos = _tzcnt_u64(slidingPiecePosBitboard);
		const DirectionBitboards& squareDirs = PreCalculatedRays[pos % 8][pos / 8];
		for (int dir = 0; dir < 4; dir++) {
			Bitboard posMoves = squareDirs[dir];
			Bitboard posBlockers = posMoves & posCombinedBitboard;

			//Change this to get the first blocker by manipulating(Rotating) the bitboard such that the least sig bit is the first blocker
			while (posBlockers != 0) {
				int posOfBlocker = _tzcnt_u64(posBlockers);
				posMoves &= ~PreCalculatedRays[posOfBlocker % 8][posOfBlocker / 8][dir];
				setBitTo(&posBlockers, posOfBlocker, 0);
			}
			bufferBitboard |= posMoves;

			if (pseudo) {
				Bitboard kingOppRay = PreCalculatedRays[oppKingPos % 8][oppKingPos / 8][(dir + 2) % 4];

				Bitboard posOppKingBlockers = kingOppRay & posCombinedBitboard;
				//Change this to get the first blocker by manipulating(Rotating) the bitboard such that the least sig bit is the first blocker
				Bitboard actualKingOppRay = kingOppRay;
				while (posOppKingBlockers != 0) {
					int posOfBlocker = _tzcnt_u64(posOppKingBlockers);
					kingOppRay &= ~PreCalculatedRays[posOfBlocker % 8][posOfBlocker / 8][(dir + 2) % 4];
					setBitTo(&posOppKingBlockers, posOfBlocker, 0);
				}

				//If the opp king ray hits one of their pieces
				Bitboard blockingPiece = kingOppRay & oppColorCombinedBitboard;
				//And this is the same piece which blocks this pieces ray
				blockingPiece &= (posMoves & oppColorCombinedBitboard);

				if (blockingPiece != 0) {
					PinnedPieceData newPinnedPiece;
					newPinnedPiece.pos = _tzcnt_u64(blockingPiece);
					//Makes the king opp ray the ray from the king to the opposite piece by:
					//ANDing it to the ray in the opposite direction of the king's ray(i.e. the original dir) 
					// which doesn't include the king(Because it isn't included in the king's ray) and doesn't include the blocked piece(bc not included in its ray)
					//But since this piece's ray includes the pinned piece, it's ok.

					//Capture mask is just this piece(I.e. the sliding piece) 
					// and the push mask is the kingOppRay AND the inverse of the ray from the sliding piece in the same dir as the king(I.e. the opposite dir of the one we checked for moves)
					newPinnedPiece.captureMask = 1ULL << pos;
					newPinnedPiece.pushMask = (actualKingOppRay & ~PreCalculatedRays[pos % 8][pos / 8][(dir + 2) % 4] & ~newPinnedPiece.captureMask);
					pinnedPieces.push_back(newPinnedPiece);
				}
			}
		}
		Bitboard bufferCapBitboard;

		if (!pseudo) {
			bufferCapBitboard = bufferBitboard & oppColorCombinedBitboard;
			bufferBitboard &= ~posCombinedBitboard;
			bitboardToMoves(bufferCapBitboard, colorToMove, pieceType, CAPTURE, pos, moves);
			bitboardToMoves(bufferBitboard, colorToMove, pieceType, MOVE, pos, moves);
		}
		else {
			bufferCapBitboard = bufferBitboard & posCombinedBitboard;
			bufferBitboard &= ~posCombinedBitboard;
		}
		moveBitboard |= bufferBitboard;
		capBitboard |= bufferCapBitboard;

		setBitTo(&slidingPiecePosBitboard, pos, 0);
	}
	return { moveBitboard, capBitboard, pinnedPieces };
}
void bitboardToMoves(Bitboard bitboard, bool pieceMovingColor, int pieceType, bool moveOrCapture, int dXApplied, int dYApplied, vector<MoveDesc>& moves) {
	MoveDesc templateMove;
	templateMove.pieceMovingColor = pieceMovingColor;
	templateMove.pieceType = pieceType;
	templateMove.moveOrCapture = moveOrCapture;
	while (bitboard != 0) {
		templateMove.posOfMove = _tzcnt_u64(bitboard);
		templateMove.posFrom = templateMove.posOfMove - (dXApplied + (dYApplied * 8));
		moves.push_back(templateMove);
		bitboard &= bitboard - 1;
	}
}
void bitboardToMoves(Bitboard bitboard, bool pieceMovingColor, int pieceType, bool moveOrCapture, int pos, vector<MoveDesc>& moves) {
	MoveDesc templateMove;
	templateMove.pieceMovingColor = pieceMovingColor;
	templateMove.pieceType = pieceType;
	templateMove.moveOrCapture = moveOrCapture;
	templateMove.posFrom = pos;
	while (bitboard != 0) {
		templateMove.posOfMove = _tzcnt_u64(bitboard);
		moves.push_back(templateMove);
		bitboard &= bitboard - 1;
	}
}
void bitboardToPromotionMoves(Bitboard bitboard, bool pieceMovingColor, bool moveOrCapture, int dXApplied, int dYApplied, vector<MoveDesc>& moves) {
	if (bitboard != 0) {
		MoveDesc templateMove;
		templateMove.pieceMovingColor = pieceMovingColor;
		templateMove.pieceType = pieceMovingColor ? whitePawn : blackPawn;
		templateMove.moveOrCapture = moveOrCapture;
		while (bitboard != 0) {
			templateMove.posOfMove = _tzcnt_u64(bitboard);
			templateMove.posFrom = templateMove.posOfMove - (dXApplied + (dYApplied * 8));
			//pieceTypePromotingTo < (4 + (pieceMovingColor * 6)) and not .... < 6+... because it can't promote to a pawn or a king. :/ 
			for (int pieceTypePromotingTo = pieceMovingColor * 6; pieceTypePromotingTo < (4 + (pieceMovingColor * 6)); pieceTypePromotingTo++) {
				templateMove.promotingToPiece = pieceTypePromotingTo;
				moves.push_back(templateMove);
			}
			bitboard &= bitboard - 1;
		}
	}
}
vector<MoveDesc> bitboardToMoveVector(Bitboard bitboard, bool pieceMovingColor, int pieceType, bool moveOrCapture, int pos) {
	vector<MoveDesc> moves = {};
	MoveDesc templateMove;
	templateMove.pieceMovingColor = pieceMovingColor;
	templateMove.pieceType = pieceType;
	templateMove.moveOrCapture = moveOrCapture;
	while (bitboard != 0) {
		MoveDesc move = templateMove;
		move.posOfMove = _tzcnt_u64(bitboard);
		move.posFrom = pos;
		moves.push_back(move);
		setBitTo(&bitboard, move.posOfMove, 0);
	}
	return moves;
}

MoveAndCapBitboards genKingLegalMoves(Bitboard kingPseudoCapBitboard, Bitboard kingPseudoMoveBitboard, Bitboard oppColorPseudoAttackBB) {
	//The oppColorPseudoAttackBB is a bitboard of the move and capture pseudo bitboards(ORed together) of the opposite pieceMovingColor
	//Pseudo because it calculates the king moves without thinking about checks or anything else...
	//such that I can actually generate the actual king moves of the opposite-colored king
	Bitboard kingCapBitboard = kingPseudoCapBitboard & ~oppColorPseudoAttackBB;
	Bitboard kingMoveBitboard = kingPseudoMoveBitboard & ~oppColorPseudoAttackBB;
	return { kingMoveBitboard , kingCapBitboard };
}

bool goesIntoCheck(AllCurrPositions allCurrPositions, MoveDesc move, bool colorToMove) {
	allCurrPositions.applyMove(move, (ZobristHash)0);
	calcCombinedPos(allCurrPositions);
	//Don't want to invert the color because you want to know if YOU are in check after YOU make the move.
	return checkChecks(allCurrPositions, colorToMove).numOfChecks > 0;
}

array<Bitboard, 2> pieceToPieceBitboard(MoveMag dir, int x, int y) {
	Bitboard localMoveBitboard = 0;
	Bitboard localCapBitboard = 0;
	int i = 1;
	//Plus one because we start at one and therefore since the x is zero indexed we need to go one more
	//Refer to the beautiful paint document for MATHEMATICAL proof.
	int nX = x;
	int nY = y;
	//Goes one less because the last one will be set to the capture bitboard
	while (i < (dir[2])) {
		nX += dir[0];
		nY += dir[1];
		setBitTo(&localMoveBitboard, nX, nY, 1);
		i++;
	}
	nX += dir[0];
	nY += dir[1];
	setBitTo(&localCapBitboard, nX, nY, 1);
	return { localMoveBitboard, localCapBitboard };
}

MovesVectAndPawnAtt fullMoveGenLoop(bool colorToMove, AllCurrPositions& allCurrPositions, ZobristHash& currZobristHash) {
	calcCombinedPos(allCurrPositions);

	Bitboard kingPosBB = allCurrPositions.pieceTypePositions[colorToMove ? whiteKing : blackKing];
	int kingPos = _tzcnt_u64(kingPosBB);

	AllCurrPositions allPositionBitboardsMinusKing = allCurrPositions;
	allPositionBitboardsMinusKing.pieceTypePositions[colorToMove ? whiteKing : blackKing] = 0;
	calcCombinedPos(allPositionBitboardsMinusKing);

	//Pass down colorToMove, not its inverse as the function inverts it;
	AttackingAndPinnedBBs attackingAndPinned = genAttackingAndPinned(allPositionBitboardsMinusKing, colorToMove, kingPos);

	Bitboard oppAttacking = attackingAndPinned.attacking;
	vector<PinnedPieceData> pinnedPieces = attackingAndPinned.pinnedPieces;

	CheckData checkChecksRes = checkChecks(allCurrPositions, colorToMove);
	int numOfCheck = checkChecksRes.numOfChecks;
	//cout << "numOfCheck: " << numOfCheck << endl;
	vector<BitboardAndPieceInfo> checkerLocations = checkChecksRes.checkerLocations;

	vector<MoveDesc> posMoves = {};
	posMoves.reserve(35);
	if (numOfCheck < 2) {
		genAllLegalMoves(numOfCheck, pinnedPieces, allCurrPositions, colorToMove, checkChecksRes, kingPos, posMoves);
	}

	//King Moves:
	MoveAndCapBitboards pseudoLegalKingMoves = genPseudoKingBitboard(allCurrPositions, colorToMove, kingPosBB, false);

	//legalKingMoves removes king moves into attacked squares
	MoveAndCapBitboards legalKingMoves = genKingLegalMoves(pseudoLegalKingMoves.capBitboard, pseudoLegalKingMoves.moveBitboard, oppAttacking);

	if (numOfCheck == 0) {
		legalKingMoves.moveBitboard |= genCastlingMoves(allCurrPositions, colorToMove, kingPosBB, oppAttacking);
	}
	addVectorsCopyFirst(bitboardToMoveVector(legalKingMoves.moveBitboard, colorToMove, colorToMove ? whiteKing : blackKing, MOVE, kingPos), posMoves);
	addVectorsCopyFirst(bitboardToMoveVector(legalKingMoves.capBitboard, colorToMove, colorToMove ? whiteKing : blackKing, CAPTURE, kingPos), posMoves);

	return { move(posMoves), attackingAndPinned.pawnAttacking };
}

AttackingAndPinnedBBs genAttackingAndPinned(AllCurrPositions allCurrPositions, bool colorToMove, int kingPos) {
	vector<PinnedPieceData> pinnedPieces = {};
	Bitboard currAttackingBitboard = 0;
	Bitboard pawnAttacking = 0;
	MoveCapAndMoveDescs results;
	MoveAndCapBitboards onlyBitboardsResults;
	MoveCapPinnedAndMoves slidingResults;
	vector<MoveDesc> emptyMoves = {};

	results = genPawnBitboard(allCurrPositions, !colorToMove, true);
	pawnAttacking = results.capBitboard | results.moveBitboard;
	currAttackingBitboard = pawnAttacking;

	results = genKnightBitboard(allCurrPositions, !colorToMove, true, emptyMoves);
	currAttackingBitboard |= results.moveBitboard;
	currAttackingBitboard |= results.capBitboard;

	slidingResults = genSlidingBitboard(allCurrPositions, !colorToMove, true, PreCalculatedHorizontalRays, !colorToMove ? whiteRook : blackRook, emptyMoves, kingPos);
	currAttackingBitboard |= slidingResults.moveBitboard;
	currAttackingBitboard |= slidingResults.capBitboard;
	pinnedPieces = slidingResults.pinnedPieces;

	slidingResults = genSlidingBitboard(allCurrPositions, !colorToMove, true, PreCalculatedDiagonalRays, !colorToMove ? whiteBishop : blackBishop, emptyMoves, kingPos);
	currAttackingBitboard |= slidingResults.moveBitboard;
	currAttackingBitboard |= slidingResults.capBitboard;
	addVectors(slidingResults.pinnedPieces, pinnedPieces);

	slidingResults = genSlidingBitboard(allCurrPositions, !colorToMove, true, PreCalculatedHorizontalRays, !colorToMove ? whiteQueen : blackQueen, emptyMoves, kingPos);
	currAttackingBitboard |= slidingResults.moveBitboard;
	currAttackingBitboard |= slidingResults.capBitboard;
	addVectors(slidingResults.pinnedPieces, pinnedPieces);

	slidingResults = genSlidingBitboard(allCurrPositions, !colorToMove, true, PreCalculatedDiagonalRays, !colorToMove ? whiteQueen : blackQueen, emptyMoves, kingPos);
	currAttackingBitboard |= slidingResults.moveBitboard;
	currAttackingBitboard |= slidingResults.capBitboard;
	addVectors(slidingResults.pinnedPieces, pinnedPieces);

	const Bitboard& kingPosBitboard = allCurrPositions.pieceTypePositions[!colorToMove ? whiteKing : blackKing];
	onlyBitboardsResults = genPseudoKingBitboard(allCurrPositions, !colorToMove, kingPosBitboard, true);
	currAttackingBitboard |= onlyBitboardsResults.moveBitboard;
	currAttackingBitboard |= onlyBitboardsResults.capBitboard;

	AttackingAndPinnedBBs res;
	res.pinnedPieces = pinnedPieces;
	res.attacking = currAttackingBitboard;
	res.pawnAttacking = pawnAttacking;
	return res;
}
vector<MoveDesc> genAllLegalMoves(int numOfCheck, vector<PinnedPieceData> pinnedPieces, AllCurrPositions allCurrPositions, bool colorToMove, CheckData checkData, int kingPos, vector<MoveDesc>& moves) {
	vector<BitboardAndPieceInfo> checkerLocations = checkData.checkerLocations;
	Bitboard checkerToKingBBMove = ~((Bitboard)0);
	Bitboard checkerToKingBBCapture = ~((Bitboard)0);
	if (numOfCheck == 1) {
		if (checkerLocations[0].normalizedPieceType == queen || checkerLocations[0].normalizedPieceType == bishop || checkerLocations[0].normalizedPieceType == rook) {
			//Generate the checker to king bitboard

			//It uses the first checker as if there are more than one, it wont use these variables
			int firstCheckerPos = (int)checkerLocations[0].pos;
			int firstCheckerX = firstCheckerPos % 8;
			int firstCheckerY = firstCheckerPos / 8;

			int xDist = firstCheckerX - (kingPos % 8);
			int xInc = min(max(xDist, -1), 1);

			int yDist = firstCheckerY - (kingPos / 8);
			int yInc = min(max(yDist, -1), 1);

			checkerToKingBBMove = 0;
			checkerToKingBBCapture = 1ULL << firstCheckerPos;

			//DON'T MAKE IT A REFERENCE!
			int normalizedPieceType = checkerLocations[0].normalizedPieceType;
			if (checkerLocations[0].normalizedPieceType == queen) {
				if (xInc == 0 || yInc == 0) {
					//Means it is horizontal, thus treat it like a rook
					normalizedPieceType = rook;
				}
				else {
					normalizedPieceType = bishop;
				}
			}
			int dir;
			if (normalizedPieceType == rook) {
				if (yInc == -1 && xInc == 0) {
					dir = 0;
				}
				else if (yInc == 0 && xInc == 1) {
					dir = 1;
				}
				else if (yInc == 1 && xInc == 0) {
					dir = 2;
				}
				else if (yInc == 0 && xInc == -1) {
					dir = 3;
				}
				const Bitboard& kingRay = PreCalculatedHorizontalRays[kingPos % 8][kingPos / 8][dir];
				const Bitboard& checkerRay = PreCalculatedHorizontalRays[firstCheckerX][firstCheckerY][dir];

				checkerToKingBBMove = (kingRay & ~checkerRay & ~checkerToKingBBCapture);
			}
			else if (normalizedPieceType == bishop) {
				if (yInc == -1 && xInc == 1) {
					dir = 0;
				}
				else if (yInc == 1 && xInc == 1) {
					dir = 1;
				}
				else if (yInc == 1 && xInc == -1) {
					dir = 2;
				}
				else if (yInc == -1 && xInc == -1) {
					dir = 3;
				}
				const Bitboard& kingRay = PreCalculatedDiagonalRays[kingPos % 8][kingPos / 8][dir];
				const Bitboard& checkerRay = PreCalculatedDiagonalRays[firstCheckerX][firstCheckerY][dir];

				checkerToKingBBMove |= (kingRay & ~checkerRay & ~checkerToKingBBCapture);
			}
		}
		else {
			//If it isn't a sliding Piece the Piece that is checking them, you can't block it
			checkerToKingBBMove = (Bitboard)0;
			//Thus the empty checkerToKingBBMove. And you can only capture the Piece
			checkerToKingBBCapture = checkerLocations[0].checkerBitboard;
			//Thus the checkerToKingBBCapture is the position of the checker
		}
	}

	moves = genPawnBitboard(allCurrPositions, colorToMove, false).moves;
	genKnightBitboard(allCurrPositions, colorToMove, false, moves);
	genSlidingBitboard(allCurrPositions, colorToMove, false, PreCalculatedHorizontalRays, colorToMove ? whiteRook : blackRook, moves);
	genSlidingBitboard(allCurrPositions, colorToMove, false, PreCalculatedDiagonalRays, colorToMove ? whiteBishop : blackBishop, moves);
	genSlidingBitboard(allCurrPositions, colorToMove, false, PreCalculatedHorizontalRays, colorToMove ? whiteQueen : blackQueen, moves);
	genSlidingBitboard(allCurrPositions, colorToMove, false, PreCalculatedDiagonalRays, colorToMove ? whiteQueen : blackQueen, moves);

	if (pinnedPieces.size() != 0) {
		for (const PinnedPieceData& pinnedPiece : pinnedPieces) {
			for (int i = moves.size() - 1; i >= 0; i--) {
				MoveDesc move = moves[i];
				if (!getBit(move.moveOrCapture == CAPTURE ? pinnedPiece.captureMask : pinnedPiece.pushMask, move.posOfMove)) {
					moves.erase(moves.begin() + i);
				}
			}
		}
	}
	if (numOfCheck != 0) {
		for (int i = moves.size() - 1; i >= 0; i--) {
			const MoveDesc& move = moves[i];
			//Only push the move if the bit in the position of the posTo in the correct mask is on.
			if (getBit(move.moveOrCapture == MOVE ? checkerToKingBBMove : checkerToKingBBCapture, move.posOfMove)) {
				moves.erase(moves.begin() + i);
			}
		}
	}

	return moves;
}

MoveMag kingOppDir(MoveMag dir, int kingPos) {
	int kingX = kingPos % 8;
	int kingY = kingPos / 8;
	int maxDist = dir[2];
	if ((dir[0] * -1) == -1) {
		maxDist = min(maxDist, kingX);
	}
	else if ((dir[0] * -1) == 1) {
		maxDist = min(maxDist, 7 - kingX);
	}
	if ((dir[1] * -1) == -1) {
		maxDist = min(maxDist, kingY);
	}
	else if ((dir[1] * -1) == 1) {
		maxDist = min(maxDist, 7 - kingY);
	}
	return { dir[0] * -1, dir[1] * -1, maxDist };
}

CheckData checkChecks(AllCurrPositions allCurrPositions, bool colorToMove) {
	int numOfChecks = 0;
	vector<BitboardAndPieceInfo> checkerLocations = {};
	//Can remove this line of code and just modify the allcurrpositions param. Optimize. Not really anymore as I use the param to get the original vector of bbs, can change this though.
	Bitboard kingPosBB = allCurrPositions.pieceTypePositions[colorToMove ? whiteKing : blackKing];

	int kingPos = _tzcnt_u64(kingPosBB);
	for (int pieceType = 0 + (colorToMove * 6); pieceType < 6 + (colorToMove * 6); pieceType++) {
		if (pieceType == (colorToMove ? whiteKing : blackKing)) {
			continue;
		}
		AllCurrPositions kingMorphedPositions = allCurrPositions;

		//Replace the normalizedPieceType's position bitboard with the king's as you only want to test one piece, not if any piece hits a candidate checker.
		kingMorphedPositions.pieceTypePositions[colorToMove ? whiteKing : blackKing] = 0;
		kingMorphedPositions.pieceTypePositions[pieceType] = kingPosBB;

		//CurrColor and not its opposite as we won't to check whether we attack something of the opposite colorToMove(So as if we were the same colorToMove as the king)
		int normalizedPieceType = pieceType > 5 ? pieceType - 6 : pieceType;
		Bitboard gennedCapBitboard;
		vector<MoveDesc> emptyMoves = {};
		switch (normalizedPieceType) {
		case rook:
			gennedCapBitboard = genSlidingBitboard(kingMorphedPositions, colorToMove, false, PreCalculatedHorizontalRays, colorToMove ? whiteRook : blackRook, emptyMoves).capBitboard;
			break;
		case knight:
			gennedCapBitboard = genKnightBitboard(kingMorphedPositions, colorToMove, false, emptyMoves).capBitboard;
			break;
		case bishop:
			gennedCapBitboard = genSlidingBitboard(kingMorphedPositions, colorToMove, false, PreCalculatedDiagonalRays, colorToMove ? whiteBishop : blackBishop, emptyMoves).capBitboard;
			break;
		case queen:
			gennedCapBitboard = genSlidingBitboard(kingMorphedPositions, colorToMove, false, PreCalculatedHorizontalRays, colorToMove ? whiteQueen : blackQueen, emptyMoves).capBitboard
				| genSlidingBitboard(kingMorphedPositions, colorToMove, false, PreCalculatedDiagonalRays, colorToMove ? whiteQueen : blackQueen, emptyMoves).capBitboard;
			break;
		case pawn:
			gennedCapBitboard = genPawnBitboard(kingMorphedPositions, colorToMove, true).capBitboard;
			break;
		default:
			[[assume(false)]];
			break;
		}

		Bitboard checkersBB = gennedCapBitboard & allCurrPositions.pieceTypePositions[pieceType > 5 ? pieceType - 6 : pieceType + 6];

		//Pretty sure you don't need to add the checkers if you have at least 2 as if you do you can only move your king thus making their position useless.
		while (checkersBB != 0 && numOfChecks != 2) {
			numOfChecks++;
			int checkerCurrPosition = _tzcnt_u64(checkersBB);
			BitboardAndPieceInfo checkerInfo = {};
			checkerInfo.checkerBitboard = 1ULL << checkerCurrPosition;
			checkerInfo.normalizedPieceType = normalizedPieceType;
			checkerInfo.pos = checkerCurrPosition;
			checkerLocations.push_back(checkerInfo);
			setBitTo(&checkersBB, checkerCurrPosition, 0);
		}
	}
	CheckData res;
	res.checkerLocations = checkerLocations;
	res.numOfChecks = numOfChecks;
	return res;
}

void calcCombinedPos(AllCurrPositions& allCurrPositions) {
	Bitboard allCombined = 0;
	for (int color = 0; color < 2; color++) {
		Bitboard colorCombinedBitboard = 0;
		for (int i = 0; i < 6; i++) {
			colorCombinedBitboard |= allCurrPositions.pieceTypePositions[i + (color * 6)];
		}
		allCurrPositions.colorCombinedPosBitboard[color] = colorCombinedBitboard;
		allCombined |= colorCombinedBitboard;
	}
	allCurrPositions.allPiecesCombBitboard = allCombined;
}
void calcCombinedMoves(AllPosMoves& posMoves) {
	Bitboard allCapBitboard = 0;
	Bitboard allMoveBitboard = 0;
	for (int i = 0; i < 6; i++) {
		Bitboard combinedCapBitboard = 0;
		Bitboard combinedMoveBitboard = 0;
		for (SinglePiecePosMoves piecePosMovesData : posMoves.pieceTypes[i].positionBitboard) {
			combinedCapBitboard |= piecePosMovesData.capBitboard;
			combinedMoveBitboard |= piecePosMovesData.moveBitboard;
		}
		posMoves.pieceTypes[i].pieceTypeCombinedCapBB = combinedCapBitboard;
		posMoves.pieceTypes[i].pieceTypeCombinedMoveBB = combinedMoveBitboard;
		allCapBitboard |= combinedCapBitboard;
		allMoveBitboard |= combinedMoveBitboard;
	}
	posMoves.combinedCapBB = allCapBitboard;
	posMoves.combinedMoveBB = allMoveBitboard;
}

bool checkBounds(int x, int y) {
	return (x >= 0 && y >= 0 && x < 8 && y < 8);
}

vector<Bitboard> arrayToVector(array<Bitboard, 2> arr) {
	vector<Bitboard> v = {};

	for (int i = 0; i < 2; i++) {
		v.push_back(arr[i]);
	}
	return v;
}
