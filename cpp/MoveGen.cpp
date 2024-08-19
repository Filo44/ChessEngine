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
		{'p', 5}
};

MoveCapAndMoveDescs genPawnBitboard(AllCurrPositions allCurrPositions, bool colorToMove, bool pseudo) {
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
		//One square push
		pawnMoveBitboard = (colorToMove ? (pawnMoveBitboard & ~cRank[0]) >> 8 : (pawnMoveBitboard & ~cRank[7]) << 8) & ~posCombinedBitboard;
		moves = bitboardToMoves(pawnMoveBitboard, colorToMove, pieceType, MOVE, 0, 1 * yMult);
		//Then it "checks" (AND it with a bitboard of those ranks) whether they are in the 3rd or 6th rank after being moved once,
		//  thus meaning they were originally in the first rank
		//Then takes away the ones which have just moved into a piece
		bufferBitboard = (colorToMove ? (pawnMoveBitboard & whiteDoubleRank) << 8 : (pawnMoveBitboard & blackDoubleRank) >> 8) & ~posCombinedBitboard;
		moves = addVectors(bitboardToMoves(bufferBitboard, colorToMove, pieceType, MOVE, 0, 2 * yMult), moves);
		pawnMoveBitboard |= bufferBitboard;

		//En passant
		Bitboard enPassantCapBitboard = pawnPosBitboard;
		//En passant towards lower files
		enPassantCapBitboard = ((enPassantCapBitboard & ~file[0]) >> 1) & (1ULL << pawnWhoDoubleMovedPos);
		//En passant towards higher files(Diff variable because I need to check whether it was to the right or left after to get the pos of the piece that is en passant-ing)
		Bitboard higherFileEnPassantCapBitboard = ((enPassantCapBitboard & ~file[7]) << 1) & (1ULL << pawnWhoDoubleMovedPos);
		enPassantCapBitboard |= higherFileEnPassantCapBitboard;


		if (enPassantCapBitboard != 0) {
			Bitboard enPassantMoveBitboard = colorToMove ? (enPassantCapBitboard >> 8) : (enPassantCapBitboard << 8);
			int posOfEnPassantMove = _tzcnt_u64(enPassantMoveBitboard);

			MoveDesc enPassantMove;
			enPassantMove.pieceMovingColor = colorToMove;
			enPassantMove.moveOrCapture = MOVE;
			enPassantMove.pieceType = colorToMove ? whitePawn : blackPawn;
			enPassantMove.posFrom = _tzcnt_u64(higherFileEnPassantCapBitboard != 0 ? enPassantCapBitboard >> 1 : enPassantCapBitboard << 1);
			enPassantMove.posOfMove = posOfEnPassantMove;

			if (!goesIntoCheck(allCurrPositions, enPassantMove, colorToMove)) {
				//Adding the enPassant moves to the bitboards
				pawnMoveBitboard |= enPassantMoveBitboard;
				pawnCapBitboard |= enPassantCapBitboard;

				moves.push_back(enPassantMove);
			}
		}

	}
	//To the lower file capture moves:
	pawnCapBitboard = ((colorToMove ? ((pawnPosBitboard & ~file[0] & ~cRank[0]) >> 8) : ((pawnPosBitboard & ~file[0] & ~cRank[7]) << 8)) >> 1) & oppColorCombinedBitboard;

	//To the higher file capture moves(Same thing just exclude the 8th/n=7 file and shift one left instead of one right):
	bufferBitboard = ((colorToMove ? ((pawnPosBitboard & ~file[7] & ~cRank[0]) >> 8) : ((pawnPosBitboard & ~file[7] & ~cRank[7]) << 8)) << 1) & oppColorCombinedBitboard;
	if (!pseudo) {
		moves = addVectors(bitboardToMoves(pawnCapBitboard, colorToMove, pieceType, CAPTURE, -1, 1 * yMult), moves);
		moves = addVectors(bitboardToMoves(bufferBitboard, colorToMove, pieceType, CAPTURE, 1, 1 * yMult), moves);
	}
	pawnCapBitboard |= bufferBitboard;

	return { pawnMoveBitboard, pawnCapBitboard, moves };
}
MoveCapAndMoveDescs genKingBitboard(AllCurrPositions allCurrPositions, bool colorToMove, bool pseudo) {
	const int pieceType = colorToMove ? whiteKing : blackKing;
	const Bitboard& kingPosBitboard = allCurrPositions.pieceTypePositions[pieceType];

	Bitboard posCombinedBitboard = allCurrPositions.allPiecesCombBitboard;
	Bitboard oppColorCombinedBitboard = allCurrPositions.colorCombinedPosBitboard[!colorToMove];
	Bitboard bufferBitboard = 0;

	vector<MoveDesc> moves = {};

	Bitboard kingMoveBitboard = 0;
	//dX = -1 dY = 0 thus >> 1
	kingMoveBitboard = (kingPosBitboard & ~file[0]) >> 1;

	//dX = -1 dY = -1 thus >> 1 + 8 = 9
	kingMoveBitboard |= (kingPosBitboard & ~file[0] & ~cRank[0]) >> 9;
	//dX = -1 dY = 1 thus << -1 + 8 = 7
	kingMoveBitboard |= (kingPosBitboard & ~file[0] & ~cRank[0]) << 7;

	//dX = 0 dY = -1 thus >> 8
	kingMoveBitboard |= (kingPosBitboard & ~file[0] & ~cRank[0]) >> 8;
	//dX = 0 dY = 1 thus << 8 
	kingMoveBitboard |= (kingPosBitboard & ~file[0] & ~cRank[0]) << 8;

	//dX = 1 dY = 0 thus << 1
	kingMoveBitboard = (kingPosBitboard & ~file[0]) << 1;
	//dX = 1 dY = -1 thus >> -1 + 8 = 7
	kingMoveBitboard |= (kingPosBitboard & ~file[0] & ~cRank[0]) >> 7;
	//dX = 1 dY = 1 thus << 1 + 8 = 9
	kingMoveBitboard |= (kingPosBitboard & ~file[0] & ~cRank[0]) << 9;

	Bitboard kingCapBitboard = kingMoveBitboard & oppColorCombinedBitboard;
	kingMoveBitboard &= ~posCombinedBitboard;
	if (!pseudo) {
		moves = bitboardToMoves(kingMoveBitboard, colorToMove, pieceType, MOVE, _tzcnt_u64(kingPosBitboard));
	}

	return { kingMoveBitboard, kingCapBitboard, moves };
}
MoveCapAndMoveDescs genKnightBitboard(AllCurrPositions allCurrPositions, bool colorToMove, bool pseudo) {
	const int pieceType = colorToMove ? whiteKnight : blackKnight;
	const Bitboard& knightPosBitboard = allCurrPositions.pieceTypePositions[pieceType];
	Bitboard posCombinedBitboard = allCurrPositions.allPiecesCombBitboard;
	Bitboard oppColorCombinedBitboard = allCurrPositions.colorCombinedPosBitboard[!colorToMove];

	Bitboard knightMoveBitboard = 0;
	Bitboard bufferBitboard;
	vector<MoveDesc> moves = {};

	//dX=-2, dY=-1. Thus >> 2 + 8 = 10
	knightMoveBitboard = (knightPosBitboard & ~file[0] & ~file[1] & ~cRank[0]) >> 10;
	moves = bitboardToMoves(knightMoveBitboard & ~posCombinedBitboard, colorToMove, pieceType, MOVE, -2, -1);
	moves = addVectors(bitboardToMoves(knightMoveBitboard & oppColorCombinedBitboard, colorToMove, pieceType, CAPTURE, -2, -1), moves);
	//dX=-2, dY=1. Thus << -2 + 8 = 6
	bufferBitboard = (knightPosBitboard & ~file[0] & ~file[1] & ~cRank[7]) << 6;
	knightMoveBitboard |= bufferBitboard;
	if (!pseudo) {
		moves = addVectors(bitboardToMoves(bufferBitboard & ~posCombinedBitboard, colorToMove, pieceType, MOVE, -2, 1), moves);
		moves = addVectors(bitboardToMoves(bufferBitboard & oppColorCombinedBitboard, colorToMove, pieceType, CAPTURE, -2, 1), moves);
	}
	//dX=2, dY=-1. Thus >> -2 + 8 = 6
	bufferBitboard = (knightPosBitboard & ~file[6] & ~file[7] & ~cRank[0]) >> 6;
	knightMoveBitboard |= bufferBitboard;
	if (!pseudo) {
		moves = addVectors(bitboardToMoves(bufferBitboard & ~posCombinedBitboard, colorToMove, pieceType, MOVE, 2, -1), moves);
		moves = addVectors(bitboardToMoves(bufferBitboard & oppColorCombinedBitboard, colorToMove, pieceType, CAPTURE, 2, -1), moves);
	}
	//dX=2, dY=1. Thus << 2 + 8 = 10
	bufferBitboard = (knightPosBitboard & ~file[6] & ~file[7] & ~cRank[7]) << 10;
	knightMoveBitboard |= bufferBitboard;
	if (!pseudo) {
		moves = addVectors(bitboardToMoves(bufferBitboard & ~posCombinedBitboard, colorToMove, pieceType, MOVE, 2, 1), moves);
		moves = addVectors(bitboardToMoves(bufferBitboard & oppColorCombinedBitboard, colorToMove, pieceType, CAPTURE, 2, 1), moves);
	}

	//dX=-1, dY=-2. Thus >> 1 + 16 = 17
	bufferBitboard = (knightPosBitboard & ~cRank[0] & ~cRank[1] & ~file[0]) >> 17;
	knightMoveBitboard |= bufferBitboard;
	if (!pseudo) {
		moves = addVectors(bitboardToMoves(bufferBitboard & ~posCombinedBitboard, colorToMove, pieceType, MOVE, -1, -2), moves);
		moves = addVectors(bitboardToMoves(bufferBitboard & oppColorCombinedBitboard, colorToMove, pieceType, CAPTURE, -1, -2), moves);
	}
	//dX=1, dY=-2. Thus << -1 + 16 = 15
	bufferBitboard = (knightPosBitboard & ~cRank[0] & ~cRank[1] & ~file[7]) << 15;
	knightMoveBitboard |= bufferBitboard;
	if (!pseudo) {
		moves = addVectors(bitboardToMoves(bufferBitboard & ~posCombinedBitboard, colorToMove, pieceType, MOVE, 1, -2), moves);
		moves = addVectors(bitboardToMoves(bufferBitboard & oppColorCombinedBitboard, colorToMove, pieceType, CAPTURE, 1, -2), moves);
	}
	//dX=-1, dY=2. Thus >> -1 + 16 = 15
	bufferBitboard = (knightPosBitboard & ~cRank[6] & ~cRank[7] & ~file[0]) >> 15;
	knightMoveBitboard |= bufferBitboard;
	if (!pseudo) {
		moves = addVectors(bitboardToMoves(bufferBitboard & ~posCombinedBitboard, colorToMove, pieceType, MOVE, -1, -2), moves);
		moves = addVectors(bitboardToMoves(bufferBitboard & oppColorCombinedBitboard, colorToMove, pieceType, CAPTURE, -1, -2), moves);
	}
	//dX=1, dY=2. Thus << 1 + 16 = 17
	bufferBitboard = (knightPosBitboard & ~cRank[6] & ~cRank[7] & ~file[7]) << 17;
	knightMoveBitboard |= bufferBitboard;
	if (!pseudo) {
		moves = addVectors(bitboardToMoves(bufferBitboard & ~posCombinedBitboard, colorToMove, pieceType, MOVE, 1, 2), moves);
		moves = addVectors(bitboardToMoves(bufferBitboard & oppColorCombinedBitboard, colorToMove, pieceType, CAPTURE, 1, 2), moves);
	}

	Bitboard knightCapBitboard = knightMoveBitboard & oppColorCombinedBitboard;
	knightMoveBitboard &= ~posCombinedBitboard;

	return { knightMoveBitboard, knightCapBitboard, moves };
}
MoveCapPinnedAndMoves genSlidingBitboard(AllCurrPositions allCurrPositions, bool colorToMove, bool pseudo, const DirectionBitboards(&PreCalculatedRays)[8][8], int pieceType) {
	Bitboard slidingPiecePosBitboard = allCurrPositions.pieceTypePositions[pieceType];
	Bitboard posCombinedBitboard = allCurrPositions.allPiecesCombBitboard;
	Bitboard oppColorCombinedBitboard = allCurrPositions.colorCombinedPosBitboard[!colorToMove];
	int oppKingPos = _tzcnt_u64(allCurrPositions.pieceTypePositions[colorToMove ? blackKing : whiteKing]);

	Bitboard moveBitboard = 0;
	Bitboard capBitboard = 0;
	Bitboard bufferBitboard;
	vector<MoveDesc> moves = {};
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
				posMoves ^= PreCalculatedRays[posOfBlocker % 8][posOfBlocker / 8][dir];
				setBitTo(&posBlockers, posOfBlocker, 0);
			}
			bufferBitboard |= posMoves;

			Bitboard kingOppRay = PreCalculatedRays[oppKingPos % 8][oppKingPos / 8][(dir + 2) % 4];
			//If the opp king ray hits one of their pieces
			Bitboard blockingPiece = kingOppRay & oppColorCombinedBitboard;
			//And this is the same piece which blocks this pieces ray
			blockingPiece &= (bufferBitboard & oppColorCombinedBitboard);

			if (blockingPiece != 0) {
				PinnedPieceData newPinnedPiece;
				newPinnedPiece.pos = _tzcnt_u64(blockingPiece);
				//Makes the king opp ray the ray from the king to the opposite piece by:
				//ANDing it to the ray in the opposite direction of the king's ray(i.e. the original dir) 
				// which doesn't include the king(Because it isn't included in the king's ray) and doesn't include the blocked piece(bc not included in its ray)
				//But since this piece's ray includes the pinned piece, it's ok.
				kingOppRay &= PreCalculatedRays[newPinnedPiece.pos % 8][newPinnedPiece.pos / 8][dir];
				//ANDing it with the inverse of the poscomb... instead of the piece because requires less operations, would need to bit shift too.
				newPinnedPiece.pushMask = (kingOppRay | bufferBitboard) & ~posCombinedBitboard;
				newPinnedPiece.captureMask = (kingOppRay | bufferBitboard) & posCombinedBitboard;
				pinnedPieces.push_back(newPinnedPiece);
			}
		}
		Bitboard bufferCapBitboard = bufferBitboard & oppColorCombinedBitboard;
		moves = addVectors(bitboardToMoves(bufferCapBitboard, colorToMove, pieceType, CAPTURE, pos), moves);

		bufferBitboard &= ~posCombinedBitboard;
		moves = addVectors(bitboardToMoves(bufferBitboard, colorToMove, pieceType, MOVE, pos), moves);
		moveBitboard |= bufferBitboard;
		capBitboard |= bufferCapBitboard;


	}
	return { moveBitboard, capBitboard, pinnedPieces, moves };
}

vector<MoveDesc> bitboardToMoves(Bitboard bitboard, bool pieceMovingColor, int pieceType, bool moveOrCapture, int dXApplied, int dYApplied) {
	vector<MoveDesc> moves = {};
	MoveDesc templateMove;
	templateMove.pieceMovingColor = pieceMovingColor;
	templateMove.pieceType = pieceType;
	templateMove.moveOrCapture = moveOrCapture;
	while (bitboard != 0) {
		MoveDesc move = templateMove;
		move.posOfMove = _tzcnt_u64(bitboard);
		move.posFrom = move.posOfMove - (dXApplied + (dYApplied * 8));
		moves.push_back(move);
		setBitTo(&bitboard, move.posFrom, 0);
	}
	return moves;
}
vector<MoveDesc> bitboardToMoves(Bitboard bitboard, bool pieceMovingColor, int pieceType, bool moveOrCapture, int pos) {
	vector<MoveDesc> moves = {};
	MoveDesc templateMove;
	templateMove.pieceMovingColor = pieceMovingColor;
	templateMove.pieceType = pieceType;
	templateMove.moveOrCapture = moveOrCapture;
	templateMove.posFrom = pos;
	while (bitboard != 0) {
		MoveDesc move = templateMove;
		move.posOfMove = _tzcnt_u64(bitboard);
		moves.push_back(move);
		setBitTo(&bitboard, pos, 0);
	}
	return moves;
}
array<Bitboard, 2> genKingLegalMoves(Bitboard kingPseudoCapBitboard, Bitboard kingPseudoMoveBitboard, Bitboard oppColorPseudoAttackBB) {
	//The oppColorPseudoAttackBB is a bitboard of the move and capture pseudo bitboards(ORed together) of the opposite pieceMovingColor
	//Pseudo because it calculates the king moves without thinking about checks or anything else...
	//such that I can actually generate the actual king moves of the opposite-colored king
	Bitboard kingCapBitboard = kingPseudoCapBitboard & ~oppColorPseudoAttackBB;
	Bitboard kingMoveBitboard = kingPseudoMoveBitboard & ~oppColorPseudoAttackBB;
	return { kingMoveBitboard , kingCapBitboard };
}

bool goesIntoCheck(AllCurrPositions allCurrPositions, MoveDesc move, bool colorToMoveBeforeThisMove) {
	allCurrPositions.applyMove(move, 0);
	return checkChecks(allCurrPositions, colorToMoveBeforeThisMove).numOfChecks > 0;
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

AllPosMoves fullMoveGenLoop(bool currentColor, AllCurrPositions& allPositionBitboards, ZobristHash& currZobristHash) {
	calcCombinedPos(allPositionBitboards);


	//Optimize this:
	AllCurrPositions allPositionBitboardsMinusKing = allPositionBitboards;
	allPositionBitboardsMinusKing.pieceTypePositions[currentColor ? whiteKing : blackKing] = 0;

	//Pass down colorToMove, not its inverse as the function inverts it;
	AttackingAndPinnedBBs attackingAndPinned = firstPseudoMoves(allPositionBitboards, currentColor);

	Bitboard oppAttacking = attackingAndPinned.attacking;
	vector<PinnedPieceData> pinnedPieces = attackingAndPinned.pinnedPieces;

	CheckData checkChecksRes = checkChecks(allPositionBitboards, currentColor);
	int numOfCheck = checkChecksRes.numOfChecks;

	//The bitboard will have two 1s in the case of 2 checkers, or more(If possible). 
	// Won't matter as it doesn't check the checkerLocations if it has 2 checkers
	vector<BitboardAndPieceInfo> checkerLocations = checkChecksRes.checkerLocations;

	Bitboard kingPosBB = allPositionBitboardsMinusKing.pieceTypePositions[currentColor ? whiteKing : blackKing];
	int kingPos = _tzcnt_u64(kingPosBB);
	//No if check for 2 checks because you have to gen the king moves anyways



	//All but king moves:
	AllPosMoves posMoves = secondPseudoMoves(numOfCheck, pinnedPieces, allPositionBitboards, currentColor, checkChecksRes, kingPos);

	//King Moves:
	//Not sure if the line below works.
	MoveCapAndPinnedBBs pseudoLegalKingMoves = genBitboard(currentColor ? 'K' : 'k', kingPos % 8, kingPos / 8, allPositionBitboards, false, currentColor, oppAttacking);

	array<Bitboard, 2> legalKingMoves = genKingLegalMoves(pseudoLegalKingMoves.capBitboard, pseudoLegalKingMoves.moveBitboard, oppAttacking);

	SinglePiecePosMoves kingMoves{};
	kingMoves.moveBitboard = legalKingMoves[0];
	kingMoves.capBitboard = legalKingMoves[1];
	kingMoves.posBitboard = kingPosBB;
	posMoves.pieceTypes[pieceToNumber['k']].positionBitboard = { kingMoves };
	calcCombinedMoves(posMoves);
	return posMoves;
}
AttackingAndPinnedBBs firstPseudoMoves(AllCurrPositions allCurrPositions, bool currColor) {
	vector<PinnedPieceData> pinnedPieces;
	vector<SinglePiecePosMoves> allBitboards;

	//Everypiece is of the class PieceTypeCurrPositions 
	for (PieceTypeCurrPositions pieceType : allCurrPositions.pieceTypePositions) {
		for (Bitboard piece : pieceType.positionBitboard) {
			Bitboard localCapBitboard = 0;
			Bitboard localMoveBitboard = 0;

			int piecePos = _tzcnt_u64(piece);
			//I have to change the genBitboard function to also take in the allcurrpositionsminusking such that it can create pseudo moves for the king while still checking for pins.
			//I think I can just remove the king(opposite colored) if pseudo==true.
			MoveCapAndPinnedBBs generatedBitboards = genBitboard(pieceType.pieceType, piecePos % 8, piecePos / 8, allCurrPositions, true, !currColor);

			vector<PinnedPieceData> pinnedPiecesGened = generatedBitboards.pinnedPieces;
			Bitboard moveGeneratedBitboard = generatedBitboards.moveBitboard;
			Bitboard capGeneratedBitboard = generatedBitboards.capBitboard;

			//pinnedPieces are of the opposite colour
			for (PinnedPieceData pinnedPiece : pinnedPiecesGened) {
				pinnedPieces.push_back(pinnedPiece);
			}

			localMoveBitboard |= moveGeneratedBitboard;
			localCapBitboard |= capGeneratedBitboard;

			SinglePiecePosMoves singleBitboards;
			singleBitboards.moveBitboard = localMoveBitboard;
			singleBitboards.capBitboard = localCapBitboard;
			singleBitboards.posBitboard = piece;
			allBitboards.push_back(singleBitboards);
			//cout << "PieceType: " << pieceType.pieceType << endl;
			//cout << "Pos of Piece: " << piecePos << endl;
			//cout << "Move bitboard: " << endl << convertBBJS(localMoveBitboard).str() << endl;
			//cout << "Capture bitboard: " << endl << convertBBJS(localCapBitboard).str() << endl;
			//cout << "-------------------------" << endl;
		}

	}

	//We add the cap & move bitboards with each of their own kind and then themselves because we only care where they are "Attacking"
	// Because we are removing where they are attacking from the king's pseudo moves to get the legal moves
	Bitboard runningAttackingBitboard = 0;
	for (SinglePiecePosMoves singleBitboards : allBitboards) {
		runningAttackingBitboard |= singleBitboards.moveBitboard;
		runningAttackingBitboard |= singleBitboards.capBitboard;
	}
	AttackingAndPinnedBBs res;
	res.attacking = runningAttackingBitboard;
	res.pinnedPieces = pinnedPieces;
	return res;
}

AllPosMoves secondPseudoMoves(int numOfCheck, vector<PinnedPieceData> pinnedPieces, AllCurrPositions allCurrPositions, bool colorToMove, CheckData checkData, int kingPos) {
	AllPosMoves allMovesBitboard;

	vector<BitboardAndPieceInfo> checkerLocations = checkData.checkerLocations;
	Bitboard checkerToKingBBMove = ~((Bitboard)0);
	Bitboard checkerToKingBBCapture = ~((Bitboard)0);
	vector<MoveDesc> moves;

	if (numOfCheck == 1) {
		if (checkerLocations[0].pieceType == 'q' || checkerLocations[0].pieceType == 'b' || checkerLocations[0].pieceType == 'r') {
			//Generate the checker to king bitboard

			//It uses the first checker as if there are more than one, it wont use these variables
			int& firstCheckerPos = checkerLocations[0].pos;

			//Distances must be ints so I can do the max,min limiting.
			int firstCheckerX = firstCheckerPos % 8;
			int firstCheckerY = firstCheckerPos / 8;
			//cout << "First checker x: " << firstCheckerX << ", y:" << firstCheckerY << endl;
			int xDist = firstCheckerX - (kingPos % 8);
			int xInc = min(max(xDist, -1), 1);
			//cout << "Xinc: " << xInc << endl;

			int yDist = firstCheckerY - (kingPos / 8);
			int yInc = min(max(yDist, -1), 1);
			//cout << "Yinc: " << yInc << endl;

			int squareDist = max(xDist, yDist);
			array<Bitboard, 2> checkerToKingBBs = pieceToPieceBitboard({ xInc, yInc, squareDist }, kingPos % 8, kingPos / 8);
			checkerToKingBBMove = checkerToKingBBs[0];
			checkerToKingBBCapture = checkerToKingBBs[1];
		} else {
			//If it isn't a sliding Piece the Piece that is checking them, you can't block it
			checkerToKingBBMove = (Bitboard)0;
			//Thus the empty checkerToKingBBMove. And you can only capture the Piece
			checkerToKingBBCapture = checkerLocations[0].checkerBitboard;
			//Thus the checkerToKingBBCapture is the position of the checker
		}
	}

	moves = genPawnBitboard(allCurrPositions, colorToMove, false).moves;
	moves = addVectors(genKnightBitboard(allCurrPositions, colorToMove, false).moves, moves);
	moves = addVectors(genSlidingBitboard(allCurrPositions, colorToMove, false, PreCalculatedHorizontalRays, colorToMove ? whiteRook : blackRook).moves, moves);
	moves = addVectors(genSlidingBitboard(allCurrPositions, colorToMove, false, PreCalculatedDiagonalRays, colorToMove ? whiteBishop : blackBishop).moves, moves);
	moves = addVectors(genSlidingBitboard(allCurrPositions, colorToMove, false, PreCalculatedHorizontalRays, colorToMove ? whiteQueen : blackQueen).moves, moves);
	moves = addVectors(genSlidingBitboard(allCurrPositions, colorToMove, false, PreCalculatedDiagonalRays, colorToMove ? whiteQueen : blackQueen).moves, moves);

	if (moves.size() != 0 && pinnedPieces.size() != 0) {
		for (MoveDesc move : moves) {
			for (PinnedPieceData pinnedPiece : pinnedPieces) {
				if (move.posFrom == pinnedPiece.pos) {

				}
			}
		}
	}

	return allMovesBitboard;
}

MoveMag kingOppDir(MoveMag dir, int kingPos) {
	int kingX = kingPos % 8;
	int kingY = kingPos / 8;
	int maxDist = dir[2];
	if ((dir[0] * -1) == -1) {
		maxDist = min(maxDist, kingX);
	} else if ((dir[0] * -1) == 1) {
		maxDist = min(maxDist, 7 - kingX);
	}
	if ((dir[1] * -1) == -1) {
		maxDist = min(maxDist, kingY);
	} else if ((dir[1] * -1) == 1) {
		maxDist = min(maxDist, 7 - kingY);
	}
	return { dir[0] * -1, dir[1] * -1, maxDist };
}

CheckData checkChecks(AllCurrPositions allCurrPositions, bool currColor) {
	CheckData res;
	int& numOfChecks = res.numOfChecks;
	vector<BitboardAndPieceInfo>& checkerLocations = res.checkerLocations;
	//Can remove this line of code and just modify the allcurrpositions param. Optimize. Not really anymore as I use the param to get the original vector of bbs, can change this though.
	Bitboard kingPosBB = allCurrPositions.colorBitboards[currColor].pieceTypes[pieceToNumber['k']].positionBitboard[0];

	////REMOVE AFTER
	//cout << "What is happening?!" << endl;
	//cout << "King bbs: " << convertVofBBJS(allCurrPositions.colorBitboards[currColor].pieceTypes[pieceToNumber['k']].positionBitboard) << endl;

	int kingPos = _tzcnt_u64(kingPosBB);
	//Loops over all types of pieces, (not all pieces on the board as the name of the variable might suggest)
	for (int pieceType = 0 + (currColor * 6); pieceType < 6 + (currColor * 6); pieceType++) {
		if (pieceType == currColor ? whiteKing : blackKing) {
			continue;
		}
		AllCurrPositions kingMorphedPositions = allCurrPositions;

		kingMorphedPositions.pieceTypePositions[currColor ? whiteKing : blackKing].positionBitboard = {};
		setBitTo(&kingMorphedPositions.pieceTypePositions[pieceType].positionBitboard, kingPos, 1);

		//CurrColor and not its opposite as we won't to check whether we attack something of the opposite colorToMove(So as if we were the same colorToMove as the king)
		MoveCapAndPinnedBBs gennedBitboards = genBitboard(pieceType, kingPos % 8, kingPos / 8, kingMorphedPositions, true, currColor, true);

		Bitboard gennedCapBitboard = gennedBitboards.capBitboard;
		Bitboard checkersBB = gennedBitboards.capBitboard & allCurrPositions.colorBitboards[!currColor].pieceTypes[pieceToNumber[pieceType]].positionBitboard;
		/*if (gennedCapBitboard != 0) {
			cout << "pieceType: " << pieceType << endl;
			cout << "Generated capture bitboard: " << (bitset<64>)gennedCapBitboard << endl;
			cout << "Piece type combined position bitboard: " << (bitset<64>)allCurrPositions.colorBitboards[!currColor].pieceTypes[pieceToNumber[pieceType]].positionBitboard << endl;
			cout << "Checkers bitboard: " << (bitset<64>)checkersBB << endl;
			cout << "-------------------" << endl;
		}*/

		//Pretty sure you don't need to add the checkers if you have at least 2 as if you do you can only move your king thus making their position useless.
		//Might be wrong.
		while (checkersBB != 0 && numOfChecks != 2) {
			numOfChecks++;
			int checkerCurrPosition = _tzcnt_u64(checkersBB);
			BitboardAndPieceInfo checkerInfo = {};
			checkerInfo.checkerBitboard = 1ULL << checkerCurrPosition;
			checkerInfo.pieceType = pieceType;
			checkerInfo.pos = checkerCurrPosition;
			checkerLocations.push_back(checkerInfo);
			setBitTo(&checkersBB, checkerCurrPosition, 0);
		}
	}
	//numOfChecks and checkerLocations are references thus I can just return res.
	return res;
}

void calcCombinedPos(AllCurrPositions& allCurrPositions) {
	Bitboard allCombined = 0;
	for (int color = 0; color < 2; color++) {
		Bitboard colorCombinedBitboard = 0;
		for (int i = 0; i < 6; i++) {
			Bitboard pieceTypeCombinedBitboard = 0;
			for (Bitboard positionBitboard : allCurrPositions.colorBitboards[color].pieceTypes[i].positionBitboard) {
				pieceTypeCombinedBitboard |= positionBitboard;
			}
			allCurrPositions.colorBitboards[color].pieceTypes[i].positionBitboard = pieceTypeCombinedBitboard;
			colorCombinedBitboard |= pieceTypeCombinedBitboard;
		}
		allCurrPositions.colorBitboards[color].colorCombinedBB = colorCombinedBitboard;
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
