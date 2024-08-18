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

//vector<Bitboard> main2() {
//	
//}
unordered_map<char, int> pieceToNumber = {
		{'r', 0},
		{'n', 1},
		{'b', 2},
		{'q', 3},
		{'k', 4},
		{'p', 5}
};

//YOU DON'T STORE THE BITBOARDS GENEREATED BY THIS FUNCTION
//This just gives you the moves for the pieces which you will feed into the search algorithm
MoveCapAndPinnedBBs genBitboard(char piece, int x, int y, AllCurrPositions allCurrPositions, bool pseudo, bool colorToMove, bool pawnMorphing, Bitboard oppColorAttackingSquares) {
	Bitboard oppColorPosBBMinusKing = allCurrPositions.colorCombinedPosBitboard[!colorToMove];
	if (pseudo) {
		//Remove king from oppColorPosBBMinusKing
		if (allCurrPositions.colorBitboards[!colorToMove].pieceTypes[pieceToNumber['k']].positionBitboard.size() == 0) {
			cout << "HUH?? No opp colored king, fix later" << endl;
			//cout << "All pos: " << endl << convertToString(allPositionBitboardsToMatrix(allCurrPositions), 8, 8) << endl;
		} else {
			int oppKingPos = _tzcnt_u64(allCurrPositions.colorBitboards[!colorToMove].pieceTypes[pieceToNumber['k']].positionBitboard[0]);
			setBitTo(&oppColorPosBBMinusKing, oppKingPos, 0);
		}
	}

	Bitboard thisColorPosBB = allCurrPositions.colorBitboards[colorToMove].colorCombinedBB;
	Bitboard capBitboard = 0;
	Bitboard moveBitboard = 0;
	//Stores position(Int) and the ORed bitboard of the Sliding Piece's dir 
	// and the king's opposite which is where the pinned Piece can move obviously after being ANDed with that Piece's pseudo moves
	vector<PinnedPieceData> pinnedPieces;
	MoveCapAndPinnedBBs generatedBitboards;

	char pieceType = tolower(piece);
	//Two ifs in case it is a queen and then just else ifs

	//Sliding Pieces
	//Inefficient, use magic bitboards later
	//Creating a vector of directions with coordinates and magnitudes(Max distance so you don't have to check if you are in bounds)
	vector<MoveMag> dirs;
	//Will add the directions of the sliding pieces such that I won't have to have it twice, and no function required
	if (pieceType == 'b' || pieceType == 'q') {
		if (x != 0) {
			if (y != 0) {
				MoveMag topLeft = { -1,-1,min(x,y) };
				dirs.push_back(topLeft);
			}
			if (y != 7) {
				MoveMag bottomLeft = { -1,1,min(x,7 - y) };
				dirs.push_back(bottomLeft);
			}
		}
		if (x != 7) {
			if (y != 7) {
				MoveMag bottomRight = { 1,1,min(7 - x,7 - y) };
				dirs.push_back(bottomRight);
			}
			if (y != 0) {
				MoveMag topRight = { 1,-1,min(7 - x,y) };
				dirs.push_back(topRight);
			}
		}

	}
	if (pieceType == 'r' || pieceType == 'q') {
		if (x != 0) {
			MoveMag left = { -1,0,x };
			dirs.push_back(left);
		}
		if (x != 7) {
			MoveMag right = { 1,0,7 - x };
			dirs.push_back(right);
		}
		if (y != 0) {
			MoveMag top = { 0,-1,y };
			dirs.push_back(top);
		}
		if (y != 7) {
			MoveMag bottom = { 0,1,7 - y };
			dirs.push_back(bottom);
		}
	}
	//Finishing the sliding pieces
	if (!dirs.empty()) {
		bool kingExists = (allCurrPositions.colorBitboards[!colorToMove].pieceTypes[pieceToNumber['k']].positionBitboard.size()) != 0;
		for (MoveMag dir : dirs) {
			array<Bitboard, 2> res = dirToBitboard(dir, oppColorPosBBMinusKing, thisColorPosBB, x, y, pseudo);
			Bitboard localMoveBitboard = res[0];
			Bitboard localCapBitboard = res[1];
			//cout << "CurrColor: " << colorToMove<< endl;
			//cout << convertVofBBJS( allCurrPositions.colorBitboards[!colorToMove].pieceTypes[pieceToNumber['k']].positionBitboard )<< endl;
			if (kingExists) {

				Bitboard kingPosBB = allCurrPositions.colorBitboards[!colorToMove].pieceTypes[pieceToNumber['k']].positionBitboard[0];
				int kingPos = _tzcnt_u64(kingPosBB);
				//cout << "---------------" << endl;
				//cout << "Generating king opp BBs, " << endl;
				//Same opp and this colorToMove bitboards as you want to check if they capture a Piece of the same colorToMove.
				array<Bitboard, 2> kingOppBBs = dirToBitboard(kingOppDir(dir, kingPos), oppColorPosBBMinusKing, thisColorPosBB, kingPos % 8, kingPos / 8, pseudo);
				//cout << "Finished generating king opp BBs, " << endl;

				Bitboard kingMoveBitboard = kingOppBBs[0];
				Bitboard kingCapBitboard = kingOppBBs[1];
				//cout << "Curr colorToMove: " << (int)colorToMove << endl;
				//cout << "pieceType: " << Piece << endl;
				//cout << "oppKingPos: " << kingPos << endl;
				//cout << "King opposite dir XInc:" << kingOppDir(dir,kingPos)[0] << ", YInc:" << kingOppDir(dir, kingPos)[1] << endl;
				//cout << (bitset<64>)kingMoveBitboard << endl;
				//cout << (bitset<64>)kingCapBitboard << endl;
				//The pinned Piece can go in the kings opp dir, the Piece's dir and the Piece itself.
				Bitboard pinnedPushMask = kingMoveBitboard | localMoveBitboard;
				Bitboard pinnedCaptureMask = (1ULL << (x + (y * 8)));
				//A Piece is pinned if it is in the king's capture bitboard, the Piece's capture bitboard
				Bitboard pinnedPieceBB = ((kingCapBitboard & localCapBitboard) & oppColorPosBBMinusKing);

				if (pinnedPieceBB != 0) {
					PinnedPieceData pinnedPiece;
					pinnedPiece.pos = _tzcnt_u64(pinnedPieceBB);
					pinnedPiece.pushMask = pinnedPushMask;
					pinnedPiece.captureMask = pinnedCaptureMask;
					pinnedPieces.push_back(pinnedPiece);
				}
			}

			moveBitboard |= localMoveBitboard;
			capBitboard |= localCapBitboard;
		}
	}
	//Else if because if the Piece is a knight, the dirs would be empty...
	// thus using the check before to cut down on the pieces you have to check are knights
	else if (pieceType == 'n') {
		//cout << "Knight" << endl;
		// All possible moves of a knight
		int possX[8] = { 2, 1, -1, -2, -2, -1,  1,  2 };
		int possY[8] = { 1, 2,  2,  1, -1, -2, -2, -1 };
		for (int i = 0; i < 8; i++) {
			//cout << "Knight" << i << endl;
			int nY = y + possY[i];
			int nX = x + possX[i];
			if (checkBounds(nX, nY)) {
				//cout << "In bounds" << i << endl;
				if (getBit(oppColorPosBBMinusKing, nX, nY)) {
					setBitTo(&capBitboard, nX, nY, 1);
				} else if (!getBit(thisColorPosBB, nX, nY) || pseudo) {
					setBitTo(&moveBitboard, nX, nY, 1);
				}
			}
		}
	} else if (pieceType == 'p') {
		//If you are checking if the king is in check by morphing, you want to invert the pawns dir
		//Yet not whether it thinks a Piece is its enemy or ally
		/*cout << "Current color: " << colorToMove << endl;
		bool pawnMovingColor = colorToMove ^ pawnMorphing;
		cout << "pawnMovingColor: " << pawnMovingColor << endl;*/

		//0 indexed btw
		int startRank;
		int lastRank;
		int enPassantRow;
		//Checking if white
		if (colorToMove) {
			startRank = 6;
			lastRank = 0;
			//Its not where you can get en passant, it's where you can en passant
			enPassantRow = 3;
		} else {
			startRank = 1;
			lastRank = 7;

			//Its not where you can get en passant, it's where you can en passant
			enPassantRow = 4;
		}

		//Checking if they can go forward once
		//Ignore the grey dotted line under the y, false positive:
		//https://stackoverflow.com/questions/71013618/understanding-the-sub-expression-overflow-reasoning
		//y>lastfile is useless, optimize. Can't be on the last file
		int forwards = (colorToMove ? -1 : 1);
		bool notOverY = ((colorToMove && y < 7) || (!colorToMove && y > 0)) || pawnMorphing;
		//cout << "forwards: " << forwards << endl;

		//!pseudo because when you are checking which squares the opp colorToMove is attacking a pawn forwards move doesn't count as attacking
		if (notOverY && !getBit(oppColorPosBBMinusKing, x, y + forwards) && !getBit(thisColorPosBB, x, y + forwards) && !pseudo) {
			setBitTo(&moveBitboard, x, y + forwards, 1);
			//Checking if it can go forwards twice
			if (y == startRank && !getBit(oppColorPosBBMinusKing, x, y + (forwards * 2)) && !getBit(thisColorPosBB, x, y + (forwards * 2))) {
				setBitTo(&moveBitboard, x, y + (forwards * 2), 1);
			}
		}

		//Checking en passant
		bool enPassantIsPosLeft = false;
		bool enPassantIsPosRight = false;
		if (!pseudo) {
			const int& pawnWhoDoubleMovedPos = allCurrPositions.pawnWhoDoubleMovedPos;;

			if (pawnWhoDoubleMovedPos != -1 && y == enPassantRow) {

				int pawnPos = _tzcnt_u64(allCurrPositions.colorBitboards[!colorToMove].pieceTypes[pieceToNumber['p']].positionBitboard[pawnWhoDoubleMovedPos]);
				//There can only be one... pawn who can get en passant
				int pieceIndex;
				for (int i = 0; i < allCurrPositions.colorBitboards[colorToMove].pieceTypes[pieceToNumber['p']].positionBitboard.size(); i++) {
					Bitboard pawnChecking = allCurrPositions.colorBitboards[colorToMove].pieceTypes[pieceToNumber['p']].positionBitboard[i];
					if (_tzcnt_u64(pawnChecking) == (x + (y * 8))) {
						pieceIndex = i;
					}
				}
				//Creates an en passant move with values which are the same when capturing to the left as to the right
				MoveDesc enPassantMove;
				enPassantMove.pieceMovingColor = colorToMove;
				enPassantMove.moveOrCapture = 0;
				enPassantMove.pieceType = pieceToNumber['p'];
				enPassantMove.piece = pieceIndex;
				enPassantMove.posFrom = (x + (y * 8));

				int forwards = (colorToMove) ? -1 : 1;

				if ((pawnPos % 8) == (x - 1)) {
					//cout << "Left" << endl;
					AllCurrPositions afterEnPassantPos = allCurrPositions;
					MoveDesc enPassantMoveLeft = enPassantMove;

					enPassantMoveLeft.posOfMove = (x - 1 + (8 * (y + forwards)));

					//Optimize, don't need to check a zobrist hash if it is a theoretical move. It is en passant though...
					afterEnPassantPos.applyMove(enPassantMoveLeft, 0);
					//Remove after
					hypos++;

					calcCombinedPos(afterEnPassantPos);
					afterEnPassantPos.pawnWhoDoubleMovedPos = -1;

					//cout << convertToJSArr(allPositionBitboardsToMatrix(afterEnPassantPos), 8, 8) << endl;
					//cout << "numOfChecks: " << checkChecks(afterEnPassantPos, colorToMove).numOfChecks << endl;

					if (checkChecks(afterEnPassantPos, colorToMove).numOfChecks == 0) {
						enPassantIsPosLeft = true;
					}
				} else if ((pawnPos % 8) == (x + 1)) {
					//cout << "Right" << endl;
					AllCurrPositions afterEnPassantPos = allCurrPositions;
					MoveDesc enPassantMoveRight = enPassantMove;

					enPassantMoveRight.posOfMove = (x + 1 + (8 * (y + forwards)));

					//Optimize, don't need to check a zobrist hash if it is a theoretical move. It is en passant though...
					afterEnPassantPos.applyMove(enPassantMoveRight, 0);
					//Remove after
					hypos++;

					calcCombinedPos(afterEnPassantPos);
					afterEnPassantPos.pawnWhoDoubleMovedPos = -1;

					//cout << convertToJSArr(allPositionBitboardsToMatrix(afterEnPassantPos), 8, 8) << endl;
					//cout << "numOfChecks: " << checkChecks(afterEnPassantPos, colorToMove).numOfChecks << endl;

					if (checkChecks(afterEnPassantPos, colorToMove).numOfChecks == 0) {
						enPassantIsPosRight = true;
					}
				}
			}

			//En Passant captures into a square which doesn't have a Piece 
			// thus that is how I am going to detect whether it is en passant
		}
		//Take to the left
		if (x > 0 && notOverY) {
			//|| pseudo because when you are checking which squares the opp colorToMove is attacking a pawn takes even though there isn't a Piece there should be counted
			if (getBit(oppColorPosBBMinusKing, x - 1, y + forwards) || pseudo) {
				setBitTo(&capBitboard, x - 1, y + forwards, 1);
			}
			//For capture blocking and taking reasons, the takes has to be to the side, and the move has to be to where it moves.
			if (enPassantIsPosLeft) {
				generatedBitboards.enPassantCapPos = (x - 1 + (y * 8));
				setBitTo(&capBitboard, x - 1, y, 1);
				setBitTo(&moveBitboard, x - 1, y + forwards, 1);
			}
		}
		//Take to the right
		if (x < 7 && notOverY) {
			//|| pseudo because when you are checking which squares the opp colorToMove is attacking a pawn takes even though there isn't a Piece there should be counted
			if (getBit(oppColorPosBBMinusKing, x + 1, y + forwards) || pseudo) {
				setBitTo(&capBitboard, x + 1, y + forwards, 1);
			}
			//For capture blocking and taking reasons, the takes has to be to the side, and the move has to be to where it moves.
			if (enPassantIsPosRight) {
				generatedBitboards.enPassantCapPos = (x + 1 + (y * 8));
				setBitTo(&capBitboard, x + 1, y, 1);
				setBitTo(&moveBitboard, x + 1, y + forwards, 1);
			}
		}


	} else if (pieceType == 'k') {
		//Generating pseudo-legal moves for the king,
		//Necessary because to generate the actual king moves you need to have every square that your opponent is attacking
		//And you can't have that without calculating the moves for a king, thus a loop
		//So you generate the pseudo-legal king moves such that you can use them to calculate the moves of the king of the opposite pieceMovingColor.

		//No need to calculate castles here as where the king moves in a castle it isn't currently attacking, 
		//thus useless for generating pseudo legal moves to then use to generate legal moves for the king
		//Not the same as the knight, these are not pairs but combinations
		for (int xInc = -1; xInc < 2; xInc++) {
			int nX = x + xInc;
			for (int yInc = -1; yInc < 2; yInc++) {
				if (yInc == 0 && xInc == 0) {
					continue;
				}
				int nY = y + yInc;
				if (checkBounds(nX, nY)) {
					if (getBit(oppColorPosBBMinusKing, nX, nY)) {
						setBitTo(&capBitboard, nX, nY, 1);
					} else if (!getBit(thisColorPosBB, nX, nY) || pseudo) {
						setBitTo(&moveBitboard, nX, nY, 1);
					}
				}
			}
		}
		if (!pseudo) {
			if (allCurrPositions.castlingRights[colorToMove].canCastleKSide || allCurrPositions.castlingRights[colorToMove].canCastleQSide) {
				//First rank is dependent on colorToMove
				int firstRank = colorToMove ? 7 : 0;
				//Checks whether we are in check
				bool canActuallyCastleKSide = false;
				bool canActuallyCastleQSide = false;
				//If not in check(X=4 because if the king hasn't castled that's his x position)
				if (!getBit(oppColorAttackingSquares, 4, firstRank)) {
					if (allCurrPositions.castlingRights[colorToMove].canCastleQSide) {
						canActuallyCastleQSide = true;
						for (int i = 1; i <= 3; i++) {
							//If there is a Piece(Ours or not) on one of the squares we have to move through, or that square is being attacked:
							//We can't castle in that direction
							if (getBit(allCurrPositions.allPiecesCombBitboard, i, firstRank) || getBit(oppColorAttackingSquares, i, firstRank)) {
								canActuallyCastleQSide = false;
								break;
							}
						}
					}
					if (allCurrPositions.castlingRights[colorToMove].canCastleKSide) {
						canActuallyCastleKSide = true;
						for (int i = 6; i >= 5; i--) {
							//If there is a Piece(Ours or not) on one of the squares we have to move through, or that square is being attacked:
							//We can't castle in that direction
							if (getBit(allCurrPositions.allPiecesCombBitboard, i, firstRank) || getBit(oppColorAttackingSquares, i, firstRank)) {
								canActuallyCastleKSide = false;
								break;
							}
						}
					}
				}
				if (canActuallyCastleQSide) {
					setBitTo(&moveBitboard, 2, firstRank, 1);
				}
				if (canActuallyCastleKSide) {
					setBitTo(&moveBitboard, 6, firstRank, 1);
				}

			}
		}
	}


	//RETURN BOTH BITBOARDS
	generatedBitboards.moveBitboard = moveBitboard;
	generatedBitboards.capBitboard = capBitboard;
	generatedBitboards.pinnedPieces = pinnedPieces;
	return generatedBitboards;
}

MoveAndCapBitboards genPawnBitboard(AllCurrPositions allCurrPositions, bool colorToMove, bool pseudo) {
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

	return { pawnMoveBitboard, pawnCapBitboard };
}
MoveAndCapBitboards genKingBitboard(AllCurrPositions allCurrPositions, bool colorToMove, bool pseudo) {
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

	return { kingMoveBitboard, kingCapBitboard };
}
MoveAndCapBitboards genKnightBitboard(AllCurrPositions allCurrPositions, bool colorToMove, bool pseudo) {
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

	return { knightMoveBitboard, knightCapBitboard };
}
MoveCapAndPinnedBBs genSlidingBitboard(AllCurrPositions allCurrPositions, bool colorToMove, bool pseudo, DirectionBitboards(&PreCalculatedRays)[8][8], int pieceType) {
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
	return { moveBitboard, capBitboard, {} };
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

bool goesIntoCheck(AllCurrPositions allCurrPositions, MoveDesc move, bool colorToMoveBeforeThisMove) {
	allCurrPositions.applyMove(move, 0);
	return checkChecks(allCurrPositions, colorToMoveBeforeThisMove).numOfChecks > 0;
}

array<Bitboard, 2> dirToBitboard(MoveMag dir, Bitboard oppColorPosBB, Bitboard thisColorPosBB, int x, int y, bool pseudo) {
	Bitboard localCapBitboard = 0;
	Bitboard localMoveBitboard = 0;
	int i = 1;
	//Plus one because we start at one and therefore since the x is zero indexed we need to go one more
	//Refer to the beautiful paint document for MATHEMATICAL proof.
	int nX = x;
	int nY = y;
	//cout << "i=1 and dir[2]+1 or the magnitude +1 is:" << dir[2] + 1 << endl;
	while (i < (dir[2] + 1)) {
		nX += dir[0];
		nY += dir[1];
		//cout << "x: " << x << ", y: " << y << endl;
		//Checks if true, if it is true it means there is a Piece of the opposite pieceMovingColor and you should ...
		if (getBit(oppColorPosBB, nX, nY)) {
			//Append it to the capture bb
			setBitTo(&localCapBitboard, nX, nY, 1);
			//cout << "Added to capture bitboard and broke" << endl;
			//Break out of the loop
			break;
		} else if (getBit(thisColorPosBB, nX, nY)) {
			if (pseudo) {
				setBitTo(&localMoveBitboard, nX, nY, 1);
			}
			//cout << "Just broke(Same colorToMove Piece)" << endl;
			break;
		}
		//No need for an else statment as it breaks if it is true
		//cout << "Nothing, continued" << endl;
		setBitTo(&localMoveBitboard, nX, nY, 1);
		i++;
	}
	array<Bitboard, 2> res;
	res[0] = localMoveBitboard;
	res[1] = localCapBitboard;
	return res;
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

array<Bitboard, 2> genKingLegalMoves(Bitboard kingPseudoCapBitboard, Bitboard kingPseudoMoveBitboard, Bitboard oppColorPseudoAttackBB) {
	//The oppColorPseudoAttackBB is a bitboard of the move and capture pseudo bitboards(ORed together) of the opposite pieceMovingColor
	//Pseudo because it calculates the king moves without thinking about checks or anything else...
	//such that I can actually generate the actual king moves of the opposite-colored king
	Bitboard kingCapBitboard = kingPseudoCapBitboard & ~oppColorPseudoAttackBB;
	Bitboard kingMoveBitboard = kingPseudoMoveBitboard & ~oppColorPseudoAttackBB;
	return { kingMoveBitboard , kingCapBitboard };
}

AllPosMoves fullMoveGenLoop(bool currentColor, AllCurrPositions& allPositionBitboards, ZobristHash& currZobristHash) {
	calcCombinedPos(allPositionBitboards);


	//Optimize this:
	AllCurrPositions allPositionBitboardsMinusKing = allPositionBitboards;
	allPositionBitboardsMinusKing.pieceTypePositions[currentColor ? whiteKing : blackKing].positionBitboard = {};
	//Pass down colorToMove, not its inverse as the function inverts it;
	AttackingAndPinnedBBs attackingAndPinned = firstPseudoMoves(allPositionBitboards, currentColor);
	Bitboard oppAttacking = attackingAndPinned.attacking;
	//REMOVE AFTER
	//cout << convertBBJS(oppAttacking).str() << endl;
	vector<PinnedPieceData> pinnedPieces = attackingAndPinned.pinnedPieces;

	CheckData checkChecksRes = checkChecks(allPositionBitboards, currentColor);
	int numOfCheck = checkChecksRes.numOfChecks;
	//cout << "Num of checks: " << numOfCheck << endl;
	//cout << "First checker: " << _tzcnt_u64(checkChecksRes.checkerLocations[0].checkerBitboard) << endl;

	//The bitboard will have two 1s in the case of 2 checkers, or more(If possible). 
	// Won't matter as it doesn't check the checkerLocations if it has 2 checkers
	vector<BitboardAndPieceInfo> checkerLocations = checkChecksRes.checkerLocations;
	//Set the pieceToNum to something static/use #define
	//Only one king thus why I accessed the [0] (0th)/(first) element of t