#include "Chess.h"
#include "MoveGen.h"
#include "Classes.h"
#include "SearchAlgorithm.h"
#include <bitset>

using namespace std;

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
MoveCapAndPinnedBBs genBitboard(char piece, int x, int y, AllCurrPositions allCurrPositions, bool pseudo, bool currentColor, bool pawnMorphing ,Bitboard oppColorAttackingSquares) {
	Bitboard oppColorPosBBMinusKing = allCurrPositions.colorBitboards[!currentColor].colorCombinedBB;
	if (pseudo) {
		//Remove king from oppColorPosBBMinusKing
		if (allCurrPositions.colorBitboards[!currentColor].pieceTypes[pieceToNumber['k']].posBB.size() == 0) {
			cout << "HUH?? No opp colored king, fix later" << endl;
			//cout << "All pos: " << endl << convertToString(allPositionBitboardsToMatrix(allCurrPositions), 8, 8) << endl;
		} else {
			int oppKingPos = _tzcnt_u64(allCurrPositions.colorBitboards[!currentColor].pieceTypes[pieceToNumber['k']].posBB[0]);
			setBitTo(&oppColorPosBBMinusKing, oppKingPos, 0);
		}
	}

	Bitboard thisColorPosBB = allCurrPositions.colorBitboards[currentColor].colorCombinedBB;
	Bitboard capBitboard = 0;
	Bitboard moveBitboard = 0;
	//Stores position(Int) and the ORed bitboard of the Sliding piece's dir 
	// and the king's opposite which is where the pinned piece can move obviously after being ANDed with that piece's pseudo moves
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
		bool kingExists = (allCurrPositions.colorBitboards[!currentColor].pieceTypes[pieceToNumber['k']].posBB.size()) != 0;
		for (MoveMag dir : dirs) {
			array<Bitboard, 2> res = dirToBitboard(dir, oppColorPosBBMinusKing, thisColorPosBB, x, y, pseudo);
			Bitboard localMoveBitboard = res[0];
			Bitboard localCapBitboard = res[1];
			//cout << "CurrColor: " << currentColor<< endl;
			//cout << convertVofBBJS( allCurrPositions.colorBitboards[!currentColor].pieceTypes[pieceToNumber['k']].posBB )<< endl;
			if (kingExists) {

				Bitboard kingPosBB = allCurrPositions.colorBitboards[!currentColor].pieceTypes[pieceToNumber['k']].posBB[0];
				int kingPos = _tzcnt_u64(kingPosBB);
				//cout << "---------------" << endl;
				//cout << "Generating king opp BBs, " << endl;
				//Same opp and this currentColor bitboards as you want to check if they capture a piece of the same currentColor.
				array<Bitboard, 2> kingOppBBs = dirToBitboard(kingOppDir(dir, kingPos), oppColorPosBBMinusKing, thisColorPosBB, kingPos % 8, kingPos / 8, pseudo);
				//cout << "Finished generating king opp BBs, " << endl;

				Bitboard kingMoveBitboard = kingOppBBs[0];
				Bitboard kingCapBitboard = kingOppBBs[1];
				//cout << "Curr currentColor: " << (int)currentColor << endl;
				//cout << "pieceType: " << piece << endl;
				//cout << "oppKingPos: " << kingPos << endl;
				//cout << "King opposite dir XInc:" << kingOppDir(dir,kingPos)[0] << ", YInc:" << kingOppDir(dir, kingPos)[1] << endl;
				//cout << (bitset<64>)kingMoveBitboard << endl;
				//cout << (bitset<64>)kingCapBitboard << endl;
				//The pinned piece can go in the kings opp dir, the piece's dir and the piece itself.
				Bitboard pinnedPushMask = kingMoveBitboard | localMoveBitboard;
				Bitboard pinnedCaptureMask = (1ULL << (x + (y * 8)));
				//A piece is pinned if it is in the king's capture bitboard, the piece's capture bitboard
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
	//Else if because if the piece is a knight, the dirs would be empty...
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
	} 
	else if (pieceType == 'p') {
		//If you are checking if the king is in check by morphing, you want to invert the pawns dir
		//Yet not whether it thinks a piece is its enemy or ally
		/*cout << "Current color: " << currentColor << endl;
		bool pawnMovingColor = currentColor ^ pawnMorphing;
		cout << "pawnMovingColor: " << pawnMovingColor << endl;*/
		
		//0 indexed btw
		int startRank;
		int lastRank;
		int enPassantRow;
		//Checking if white
		if (currentColor) {
			//Inverted because of how stupid matricies are made in programming languages
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
		int forwards = (currentColor ? -1 : 1);
		bool notOverY = ((currentColor && y < 7) || (!currentColor && y > 0)) || pawnMorphing;
		//cout << "forwards: " << forwards << endl;

		//!pseudo because when you are checking which squares the opp currentColor is attacking a pawn forwards move doesn't count as attacking
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
			int pawnWhoDoubleMoved = allCurrPositions.pawnWhoDoubleMoved;;

			if (pawnWhoDoubleMoved != -1 && y == enPassantRow) {
				//cout << "Checking stage 2" << endl;
				int pawnPos = _tzcnt_u64(allCurrPositions.colorBitboards[!currentColor].pieceTypes[pieceToNumber['p']].posBB[pawnWhoDoubleMoved]);
				//There can only be one... pawn who can get en passant
				int pieceIndex;
				for (int i = 0; i < allCurrPositions.colorBitboards[currentColor].pieceTypes[pieceToNumber['p']].posBB.size(); i++) {
					Bitboard pawnChecking = allCurrPositions.colorBitboards[currentColor].pieceTypes[pieceToNumber['p']].posBB[i];
					if (_tzcnt_u64(pawnChecking) == (x + (y * 8))) {
						pieceIndex = i;
					}
				}
				//Creates an en passant move with values which are the same when capturing to the left as to the right
				MoveDesc enPassantMove;
				enPassantMove.pieceMovingColor = currentColor;
				enPassantMove.moveOrCapture = 0;
				enPassantMove.pieceType = pieceToNumber['p'];
				enPassantMove.piece = pieceIndex;
				int forwards = (currentColor) ? -1 : 1;

				if ((pawnPos % 8) == (x - 1)) {
					//cout << "Left" << endl;
					AllCurrPositions afterEnPassantPos = allCurrPositions;
					MoveDesc enPassantMoveLeft = enPassantMove;

					enPassantMoveLeft.posOfMove = (x - 1 + (8 * (y+forwards)));

					//Optimize, don't need to check a zobrist hash if it is a theoretical move. It is en passant though...
					afterEnPassantPos.applyMove(enPassantMoveLeft, 0);
					//Remove after
					hypos++;

					calcCombinedPos(afterEnPassantPos);
					afterEnPassantPos.pawnWhoDoubleMoved = -1;
					
					//cout << convertToJSArr(allPositionBitboardsToMatrix(afterEnPassantPos), 8, 8) << endl;
					//cout << "numOfChecks: " << checkChecks(afterEnPassantPos, currentColor).numOfChecks << endl;

					if (checkChecks(afterEnPassantPos, currentColor).numOfChecks == 0) {
						enPassantIsPosLeft = true;
					}
				} else if ((pawnPos % 8) == (x + 1)) {
					//cout << "Right" << endl;
					AllCurrPositions afterEnPassantPos = allCurrPositions;
					MoveDesc enPassantMoveRight = enPassantMove;

					enPassantMoveRight.posOfMove = (x + 1 + (8 * (y+ forwards)));

					//Optimize, don't need to check a zobrist hash if it is a theoretical move. It is en passant though...
					afterEnPassantPos.applyMove(enPassantMoveRight, 0);
					//Remove after
					hypos++;

					calcCombinedPos(afterEnPassantPos);
					afterEnPassantPos.pawnWhoDoubleMoved = -1;
					
					//cout << convertToJSArr(allPositionBitboardsToMatrix(afterEnPassantPos), 8, 8) << endl;
					//cout << "numOfChecks: " << checkChecks(afterEnPassantPos, currentColor).numOfChecks << endl;

					if (checkChecks(afterEnPassantPos, currentColor).numOfChecks == 0) {
						enPassantIsPosRight = true;
					}
				}
			}

			//En Passant captures into a square which doesn't have a piece 
			// thus that is how I am going to detect whether it is en passant
		}
		//Take to the left
		if (x > 0 && notOverY) {
			//|| pseudo because when you are checking which squares the opp currentColor is attacking a pawn takes even though there isn't a piece there should be counted
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
			//|| pseudo because when you are checking which squares the opp currentColor is attacking a pawn takes even though there isn't a piece there should be counted
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
		

	} 
	else if (pieceType == 'k') {
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
			if (allCurrPositions.colorBitboards[currentColor].canCastleKSide || allCurrPositions.colorBitboards[currentColor].canCastleQSide) {
				//First rank is dependent on currentColor
				int firstRank = currentColor ? 7 : 0;
				//Checks whether we are in check
				bool canActuallyCastleKSide = false;
				bool canActuallyCastleQSide = false;
				//If not in check(X=4 because if the king hasn't castled that's his x position)
				if (!getBit(oppColorAttackingSquares, 4, firstRank)) {
					if (allCurrPositions.colorBitboards[currentColor].canCastleQSide) {
						canActuallyCastleQSide = true;
						for (int i = 1; i <= 3; i++) {
							//If there is a piece(Ours or not) on one of the squares we have to move through, or that square is being attacked:
							//We can't castle in that direction
							if (getBit(allCurrPositions.allPiecesCombBitboard, i, firstRank) || getBit(oppColorAttackingSquares, i, firstRank)) {
								canActuallyCastleQSide = false;
								break;
							}
						}
					}
					if (allCurrPositions.colorBitboards[currentColor].canCastleKSide) {
						canActuallyCastleKSide = true;
						for (int i = 6; i >= 5; i--) {
							//If there is a piece(Ours or not) on one of the squares we have to move through, or that square is being attacked:
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
		//Checks if true, if it is true it means there is a piece of the opposite pieceMovingColor and you should ...
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
			//cout << "Just broke(Same currentColor piece)" << endl;
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
	allPositionBitboardsMinusKing.colorBitboards[currentColor].pieceTypes[pieceToNumber['k']].posBB = {};
	//Pass down currentColor, not its inverse as the function inverts it;
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
	//Only one king thus why I accessed the [0] (0th)/(first) element of the vector
	Bitboard kingPosBB = allPositionBitboards.colorBitboards[currentColor].pieceTypes[pieceToNumber['k']].posBB[0];
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
	posMoves.pieceTypes[pieceToNumber['k']].posBB = { kingMoves };
	calcCombinedMoves(posMoves);
	return posMoves;
}

AttackingAndPinnedBBs firstPseudoMoves(AllCurrPositions allCurrPositions, bool currColor) {
	OneColorCurrPositions everyPieceColor = allCurrPositions.colorBitboards[!currColor];
	vector<PinnedPieceData> pinnedPieces;
	vector<SinglePiecePosMoves> allBitboards;

	//Everypiece is of the class PieceTypeCurrPositions
	for (PieceTypeCurrPositions everyPiece : everyPieceColor.pieceTypes) {
		for (Bitboard piece : everyPiece.posBB) {
			Bitboard localCapBitboard = 0;
			Bitboard localMoveBitboard = 0;

			int piecePos = _tzcnt_u64(piece);
			//I have to change the genBitboard function to also take in the allcurrpositionsminusking such that it can create pseudo moves for the king while still checking for pins.
			//I think I can just remove the king(opposite colored) if pseudo==true.
			MoveCapAndPinnedBBs generatedBitboards = genBitboard(everyPiece.pieceType, piecePos % 8, piecePos / 8, allCurrPositions, true, !currColor);

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
			//cout << "PieceType: " << everyPiece.pieceType << endl;
			//cout << "Pos of piece: " << piecePos << endl;
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

AllPosMoves secondPseudoMoves(int numOfCheck, vector<PinnedPieceData> pinnedPieces, AllCurrPositions allCurrPositions, bool currColor, CheckData checkData, int kingPos) {
	OneColorCurrPositions everyPieceColor = allCurrPositions.colorBitboards[currColor];
	AllPosMoves allMovesBitboard;

	vector<BitboardAndPieceInfo> checkerLocations = checkData.checkerLocations;
	Bitboard checkerToKingBBMove = ~((Bitboard)0);
	Bitboard checkerToKingBBCapture = ~((Bitboard)0);


	if (numOfCheck == 1) {
		if (checkerLocations[0].pieceType == 'q' || checkerLocations[0].pieceType == 'b' || checkerLocations[0].pieceType == 'r') {
			//Generate the checker to king bitboard

			//It uses the first checker as if there are more than one, it wont use these variables
			int &firstCheckerPos = checkerLocations[0].pos;
		
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
			//If it isn't a sliding piece the piece that is checking them, you can't block it
			checkerToKingBBMove = (Bitboard)0;
			//Thus the empty checkerToKingBBMove. And you can only capture the piece
			checkerToKingBBCapture = checkerLocations[0].checkerBitboard;
			//Thus the checkerToKingBBCapture is the position of the checker
		}
	}
	//Everypiece is of the class PieceTypeCurrPositions

	for (int i = 0; i < 6; i++) {
		PieceTypeCurrPositions everyPiece = everyPieceColor.pieceTypes[i];
		PieceTypePosMoves pieceTypeMoveBBStorer;
		pieceTypeMoveBBStorer.pieceType = everyPiece.pieceType;
		allMovesBitboard.pieceTypes[i] = pieceTypeMoveBBStorer;
		if (i == pieceToNumber['k']) {
			//If it is a king, let it initialise the pieceType, because then the code in the other function just adds this king to its type.
			continue;
		}
		if (numOfCheck != 2) {
			for (Bitboard piece : everyPiece.posBB) {
				Bitboard localCapBitboard = 0;
				Bitboard localMoveBitboard = 0;
				int piecePos = _tzcnt_u64(piece);

				MoveCapAndPinnedBBs generatedBitboards = genBitboard(currColor ? toupper(everyPiece.pieceType) : tolower(everyPiece.pieceType), piecePos % 8, piecePos / 8, allCurrPositions, false, currColor);
				Bitboard moveGeneratedBitboard = generatedBitboards.moveBitboard;
				Bitboard capGeneratedBitboard = generatedBitboards.capBitboard;

				//Don't check if checks==1 because the bitboards are full(111111...) if there aren't any checks
				localMoveBitboard |= (moveGeneratedBitboard & checkerToKingBBMove);

				//Since it is check, it cannot possibly capture something to block the check except the attacker 
				// (Changed the function to return it, because I have a feeling its different for en passant)
				localCapBitboard |= (capGeneratedBitboard & checkerToKingBBCapture); 
				//Only one number(enPassantCapPos) because you can only enPassant max one piece in any position 
				if (generatedBitboards.enPassantCapPos != -1) {
					//cout << "phase 1 passed" << endl;
					//If there is a en passant move
					int enPassantCapPos = generatedBitboards.enPassantCapPos;
					int forwards = currColor ? -1 : 1;
					if (getBit(localCapBitboard, enPassantCapPos) || getBit(localMoveBitboard, enPassantCapPos % 8, (enPassantCapPos / 8)+forwards)) {
						//cout << "phase 2 passed" << endl;
						//And if you can either capture that position, or move to the position in front

						//Disable the capture en passant move
						setBitTo(&localCapBitboard, enPassantCapPos, 0);
						//And set the move en passant move
						setBitTo(&localMoveBitboard, enPassantCapPos % 8, (enPassantCapPos / 8) + forwards, 1);
					}
				}

				//Might be dangerous deleting elements while looping even when going backwards
				for (int i = pinnedPieces.size() - 1; i >= 0; i--) {
					PinnedPieceData pinnedPieceInstance = pinnedPieces[i];
					//cout << "pinnedPiecePos" << pinnedPieceInstance.pos;
					if (getBit(piece, pinnedPieceInstance.pos)) {
						//cout << "Pinned piece, x:" << pinnedPieceInstance.pos % 8 << ", y: " << pinnedPieceInstance.pos / 8 << endl;
						localMoveBitboard &= pinnedPieceInstance.pushMask;
						localCapBitboard &= pinnedPieceInstance.captureMask;
						pinnedPieces.erase(pinnedPieces.begin() + i);
					}
				}
				SinglePiecePosMoves singlePieceMoveBBs;
				singlePieceMoveBBs.moveBitboard = localMoveBitboard;
				singlePieceMoveBBs.capBitboard = localCapBitboard;
				singlePieceMoveBBs.posBitboard = piece;
				allMovesBitboard.pieceTypes[pieceToNumber[everyPiece.pieceType]].posBB.push_back(singlePieceMoveBBs);
			}
		} else {
			for (Bitboard piece : everyPiece.posBB) {
				SinglePiecePosMoves singlePieceMoveBBs;
				singlePieceMoveBBs.moveBitboard = 0;
				singlePieceMoveBBs.capBitboard = 0;
				singlePieceMoveBBs.posBitboard = piece;
				allMovesBitboard.pieceTypes[pieceToNumber[everyPiece.pieceType]].posBB.push_back(singlePieceMoveBBs);
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
	Bitboard kingPosBB = allCurrPositions.colorBitboards[currColor].pieceTypes[pieceToNumber['k']].posBB[0];

	////REMOVE AFTER
	//cout << "What is happening?!" << endl;
	//cout << "King bbs: " << convertVofBBJS(allCurrPositions.colorBitboards[currColor].pieceTypes[pieceToNumber['k']].posBB) << endl;

	int kingPos = _tzcnt_u64(kingPosBB);
	//Loops over all types of pieces, (not all pieces on the board as the name of the variable might suggest)
	for (char pieceType : pieces) {
		if (pieceType == 'k') {
			continue;
		}
		AllCurrPositions kingMorphedPositions = allCurrPositions;
		//Optimize:
		kingMorphedPositions.colorBitboards[currColor].pieceTypes[pieceToNumber['k']].posBB = {};
		kingMorphedPositions.colorBitboards[currColor].pieceTypes[pieceToNumber[pieceType]].posBB.push_back(kingPosBB);

		//CurrColor and not its opposite as we won't to check whether we attack something of the opposite currentColor(So as if we were the same currentColor as the king)
		MoveCapAndPinnedBBs gennedBitboards = genBitboard(pieceType, kingPos % 8, kingPos / 8, kingMorphedPositions, true, currColor, true);
		////REMOVE AFTER
		/*if (pieceType == 'p') {
			cout << "Pawn attacking bitboard: " << convertBBJS(gennedBitboards.capBitboard).str() << endl;
		}*/
		Bitboard gennedCapBitboard = gennedBitboards.capBitboard;
		Bitboard checkersBB = gennedBitboards.capBitboard & allCurrPositions.colorBitboards[!currColor].pieceTypes[pieceToNumber[pieceType]].pieceTypeCombinedBB;
		/*if (gennedCapBitboard != 0) {
			cout << "pieceType: " << pieceType << endl;
			cout << "Generated capture bitboard: " << (bitset<64>)gennedCapBitboard << endl;
			cout << "piece type combined position bitboard: " << (bitset<64>)allCurrPositions.colorBitboards[!currColor].pieceTypes[pieceToNumber[pieceType]].pieceTypeCombinedBB << endl;
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
			for (Bitboard positionBitboard : allCurrPositions.colorBitboards[color].pieceTypes[i].posBB) {
				pieceTypeCombinedBitboard |= positionBitboard;
			}
			allCurrPositions.colorBitboards[color].pieceTypes[i].pieceTypeCombinedBB = pieceTypeCombinedBitboard;
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
		for (SinglePiecePosMoves piecePosMovesData : posMoves.pieceTypes[i].posBB) {
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
