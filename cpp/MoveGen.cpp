#include "Global.h"
#include "MoveGen.h"
#include "Chess.h"


using namespace std;


//vector<Bitboard> main2() {
//	
//}



//YOU DON'T STORE THE BITBOARDS GENEREATED BY THIS FUNCTION
//This just gives you the moves for the pieces which you will feed into the search algorithm
MoveCapAndPinnedBBs genBitboard(char piece, int x, int y, AllCurrPositions allCurrPositions, bool pseudo, bool currentColor) {
	Bitboard oppColorPosBB = allCurrPositions.colorBitboards[!currentColor].colorCombinedBB;
	Bitboard thisColorPosBB = allCurrPositions.colorBitboards[currentColor].colorCombinedBB;
	Bitboard capBitboard = 0;
	Bitboard moveBitboard = 0;
	//Stores position(Int) and the ORed bitboard of the Sliding piece's dir 
	// and the king's opposite which is where the pinned piece can move obviously after being ANDed with that piece's pseudo moves
	vector<PinnedPieceData> pinnedPieces;
	MoveCapAndPinnedBBs generatedBitboards;

	char pieceType = tolower(piece);
	//White:true, black:false
	bool color = !(piece == pieceType);
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

	}if (pieceType == 'r' || pieceType == 'q') {
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
		
		for (MoveMag dir : dirs) {
			array<Bitboard, 2> res = dirToBitboard(dir, oppColorPosBB, thisColorPosBB, x, y);
			Bitboard localMoveBitboard = res[0];
			Bitboard localCapBitboard = res[1];

			//GenKingOppDir gens only capture bitboard
			Bitboard kingPosBB = allCurrPositions.colorBitboards[currentColor].pieceTypes[pieceToNumber['k']].posBB[0];
			int kingPos = _tzcnt_u64(kingPosBB);
			array<Bitboard, 2> kingOppBBs = dirToBitboard(kingOppDir(dir, kingPos), oppColorPosBB, thisColorPosBB, kingPos%8,kingPos/8);

			Bitboard kingMoveBitboard = kingOppBBs[0];
			Bitboard kingCapBitboard = kingOppBBs[1];
			//The pinned piece can go in the kings opp dir, the piece's dir and the piece itself.
			Bitboard pinnedMoveMask = kingMoveBitboard| localMoveBitboard | (1ULL << (x+(y*8)));
			//A piece is pinned if it is in the king's capture bitboard, the piece's capture bitboard
			Bitboard pinnedPieceBB = ((kingCapBitboard & localCapBitboard) & oppColorPosBB);

			if ( pinnedPieceBB != 0 ) {
				PinnedPieceData pinnedPiece;
				pinnedPiece.pos = _tzcnt_u64(pinnedPieceBB);
				pinnedPiece.posMoveMask = pinnedMoveMask;
				pinnedPieces.push_back(pinnedPiece);
			}
			
			moveBitboard |= localMoveBitboard;
			capBitboard |= localCapBitboard;
		}
	}
	//Else if because if the piece is a knight, the dirs would be empty...
	// thus using the check before to cut down on the pieces you have to check are knights
	else if (pieceType == 'n') {
		cout << "Knight" << endl;
		// All possible moves of a knight
		int possX[8] = { 2, 1, -1, -2, -2, -1,  1,  2 };
		int possY[8] = { 1, 2,  2,  1, -1, -2, -2, -1 };
		for (int i = 0; i < 8; i++) {
			cout << "Knight" << i << endl;
			int nY = y + possY[i];
			int nX = x + possX[i];
			if (checkBounds(nX, nY)) {
				cout << "In bounds" << i << endl;
				if (getBit(oppColorPosBB, nX, nY)) {
					setBitTo(&capBitboard, nX, nY, 1);
				} else {
					setBitTo(&moveBitboard, nX, nY, 1);
				}
			}
		}
	} else if (pieceType == 'p') {
		//0 indexed btw
		int startRank;
		int lastRank;
		//Checking if white
		if (color) {
			//Inverted because of how stupid matricies are made in programming languages
			startRank = 6;
			lastRank = 0;
		} else {
			startRank = 1;
			lastRank = 7;
		}

		//Checking if they can go forward once
		//Ignore the grey dotted line under the y, false positive:
		//https://stackoverflow.com/questions/71013618/understanding-the-sub-expression-overflow-reasoning
		//y>lastfile is useless, optimize. Can't be on the last file
		int forwards = (color ? -1 : 1);
		bool notOverY = ((color && y < 7) || (!color && y > 0));
		if (notOverY && !getBit(oppColorPosBB, x, y + forwards)) {
			setBitTo(&moveBitboard, x, y + forwards, 1);
			//Checking if it can go forwards twice
			if (y == startRank && !getBit(oppColorPosBB, x, y + (forwards * 2))) {
				setBitTo(&moveBitboard, x, y + (forwards * 2), 1);
			}
		}
		//Take to the left
		if (x > 0 && notOverY && getBit(oppColorPosBB, x - 1, y + forwards)) {
			setBitTo(&capBitboard, x - 1, y + forwards, 1);
		}
		if (x < 7 && notOverY && getBit(oppColorPosBB, x + 1, y + forwards)) {
			setBitTo(&capBitboard, x + 1, y + forwards, true);
		}
		//Do En passant
		//EN PASSANT c'est le pire, je deteste (Squiggly line C)a. 
	}
	else if (pieceType == 'k') {
		
		if (pseudo) {
			//Generating pseudo-legal moves for the king,
			//Necessary because to generate the actual king moves you need to have every square that your opponent is attacking
			//And you can't have that without calculating the moves for a king, thus a loop
			//So you generate the pseudo-legal king moves such that you can use them to calculate the moves of the king of the opposite color.
			
			//No need to calculate castles here as where the king moves in a castle it isn't currently attacking, 
			//thus useless for generating pseudo legal moves to then use to generate legal moves for the king
			//Not the same as the knight, these are not pairs but combinations
			for (int xInc = -1; xInc < 2; xInc++) {
				for (int yInc = -1; yInc < 2; (xInc == 0) ? (yInc += 2) : (yInc++)) {
					if (getBit(oppColorPosBB, x + xInc, y + yInc)) {
						setBitTo(&capBitboard, x + xInc, y + yInc, 1);
					} else {
						setBitTo(&moveBitboard, x + xInc, y + yInc, 1);
					}
				}
			}
		} else {

		}
	}
	

	//RETURN BOTH BITBOARDS
	generatedBitboards.moveBitboard = moveBitboard;
	generatedBitboards.capBitboard = capBitboard;
	generatedBitboards.pinnedPieces = pinnedPieces;
	return generatedBitboards;
}

array<Bitboard, 2> dirToBitboard(MoveMag dir, Bitboard oppColorPosBB, Bitboard thisColorPosBB, int x, int y) {
	Bitboard localCapBitboard = 0;
	Bitboard localMoveBitboard = 0;
	int i = 1;
	//Plus one because we start at one and therefore since the x is zero indexed we need to go one more
	//Refer to the beautiful paint document for MATHEMATICAL proof.
	int nX = x;
	int nY = y;
	while (i < (dir[2] + 1)) {
		nX += dir[0];
		nY += dir[1];
		//Checks if true, if it is true it means there is a piece of the opposite color and you should ...
		if (getBit(oppColorPosBB, nX, nY)) {
			//Append it to the capture bb
			setBitTo(&localCapBitboard, nX, nY, 1);
			//Break out of the loop
			break;
		} else if (getBit(thisColorPosBB, nX, nY)) {
			break;
		}
		//No need for an else statment as it breaks if it is true
		setBitTo(&localMoveBitboard, nX, nY, 1);
		i++;
	}
	array<Bitboard, 2> res;
	res[0] = localMoveBitboard;
	res[1] = localCapBitboard;
}

Bitboard pieceToPieceBitboard(MoveMag dir, int x, int y) {
	Bitboard localMoveBitboard = 0;
	int i = 1;
	//Plus one because we start at one and therefore since the x is zero indexed we need to go one more
	//Refer to the beautiful paint document for MATHEMATICAL proof.
	int nX = x;
	int nY = y;
	while (i < (dir[2] + 1)) {
		nX += dir[0];
		nY += dir[1];

		setBitTo(&localMoveBitboard, nX, nY, 1);
		i++;
	}
	return localMoveBitboard;
}

array<Bitboard, 2> genKingLegalMoves(Bitboard kingPseudoCapBitboard, Bitboard kingPseudoMoveBitboard, Bitboard oppColorPseudoAttackBB) {
	//The oppColorPseudoAttackBB is a bitboard of the move and capture pseudo bitboards(ORed together) of the opposite color
	//Pseudo because it calculates the king moves without thinking about checks or anything else...
	//such that I can actually generate the actual king moves of the opposite-colored king
	Bitboard kingCapBitboard = kingPseudoCapBitboard & ~oppColorPseudoAttackBB;
	Bitboard kingMoveBitboard = kingPseudoMoveBitboard & ~oppColorPseudoAttackBB;
	return { kingCapBitboard, kingMoveBitboard };
}

AllPosMoves fullMoveGenLoop(bool currentColor, AllCurrPositions allPositionBitboards) {
	calcCombinedPos(allPositionBitboards);
	//AllPosMoves posMoves;
	//for (int i = 0; i < 6; i++) {
	//	PieceTypePosMoves newPiece;
	//	newPiece.pieceType = pieces[i];
	//	posMoves.pieceTypes[i] = newPiece;
	//}

	OneColorCurrPositions oppColorCurrPosWithoutKing = allPositionBitboards.colorBitboards[!currentColor];

	//Optimize this:
	oppColorCurrPosWithoutKing.pieceTypes[pieceToNumber['k']].posBB = {};
	AttackingAndPinnedBBs attackingAndPinned = firstPseudoMoves(oppColorCurrPosWithoutKing);
	Bitboard oppAttacking = attackingAndPinned.attacking;
	vector<PinnedPieceData> pinnedPieces = attackingAndPinned.pinnedPieces;

	CheckData checkChecksRes = checkChecks(allPositionBitboards, currentColor);
	int numOfCheck = checkChecksRes.numOfChecks;

	//The bitboard will have two 1s in the case of 2 checkers, or more(If possible). 
	// Won't matter as it doesn't check the checkerLocations if it has 2 checkers
	vector<Bitboard> checkerLocations = checkChecksRes.checkerLocations;
	//Set the pieceToNum to something static/use #define
	//Only one king thus why I accessed the [0] (0th)/(first) element of the vector
	Bitboard kingPosBB = allPositionBitboards.colorBitboards[currentColor].pieceTypes[pieceToNumber['k']].posBB[0];
	int kingPos = _tzcnt_u64(kingPosBB);
	//No if check for 2 checks because you have to gen the king moves anyways



	//All but king moves:
	AllPosMoves posMoves = secondPseudoMoves(numOfCheck, pinnedPieces, allPositionBitboards, currentColor, checkChecksRes, kingPos);

	//King Moves:
	//Not sure if the line below works.
	MoveCapAndPinnedBBs pseudoLegalKingMoves = genBitboard(currentColor?'K' : 'k', kingPos % 8, kingPos / 8, allPositionBitboards, true, currentColor);
	array<Bitboard, 2> legalKingMoves = genKingLegalMoves(pseudoLegalKingMoves.capBitboard, pseudoLegalKingMoves.moveBitboard, oppAttacking);
	SinglePiecePosMoves kingMoves;
	kingMoves.moveBitboard = legalKingMoves[0];
	kingMoves.capBitboard = legalKingMoves[1];
	kingMoves.posBitboard = kingPosBB;
	posMoves.pieceTypes[pieceToNumber['k']].posBB = { kingMoves };
	
 }

AttackingAndPinnedBBs firstPseudoMoves(AllCurrPositions allCurrPositions, bool currColor) {
	OneColorCurrPositions everyPieceColor = allCurrPositions.colorBitboards[currColor];
	vector<PinnedPieceData> pinnedPieces;
	vector<SinglePiecePosMoves> allBitboards;

	//Everypiece is of the class PieceTypeCurrPositions
	for (PieceTypeCurrPositions everyPiece : everyPieceColor.pieceTypes) {
		for (Bitboard piece : everyPiece.posBB) {
			Bitboard localCapBitboard = 0;
			Bitboard localMoveBitboard = 0;

			int piecePos = _tzcnt_u64(piece);
			MoveCapAndPinnedBBs generatedBitboards = genBitboard(!currColor ? toupper(everyPiece.pieceType) : tolower(everyPiece.pieceType), piecePos % 8, piecePos / 8, allCurrPositions, true, currColor);

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

	vector<Bitboard> checkerLocations = checkData.checkerLocations;
	int numOfChecks = checkData.numOfChecks;
	
	//It uses the first checker as if there are more than one, it wont use these variables
	int firstCheckerPos = _tzcnt_u64(checkerLocations[0]);
	//Distances must be ints so I can do the max,min limiting.
	int xDist = (firstCheckerPos % 8) - (kingPos % 8);
	int xInc = min(max(xDist, -1), 1);

	int yDist = (firstCheckerPos / 8) - (kingPos / 8);
	int yInc = min(max(yDist, -1), 1);
	
	int squareDist = max(xDist, yDist);
	Bitboard checkerToKingBB = pieceToPieceBitboard({yInc, xInc, squareDist}, kingPos%8, kingPos/8);

	if (numOfCheck == 1) {
		//Generate the checker to king bitboard
		//CheckData checkCheckRes = checkChecks
		
	}
	//Everypiece is of the class PieceTypeCurrPositions
	
	for (int i = 0; i < 6; i++) {
		PieceTypeCurrPositions everyPiece = everyPieceColor.pieceTypes[i];
		PieceTypePosMoves pieceTypeMoveBBStorer;
		pieceTypeMoveBBStorer.pieceType = everyPiece.pieceType;
		allMovesBitboard.pieceTypes[i]=pieceTypeMoveBBStorer;
		if (i == pieceToNumber['k']) {
			//If it is a king, let it initialise the pieceType, because then the code in the other function just adds this king to its type.
			continue;
		}
		for (Bitboard piece : everyPiece.posBB) {
			Bitboard localCapBitboard = 0;
			Bitboard localMoveBitboard = 0;
			int piecePos = _tzcnt_u64(piece);
			
			MoveCapAndPinnedBBs generatedBitboards = genBitboard(currColor ? toupper(everyPiece.pieceType) : tolower(everyPiece.pieceType), piecePos % 8, piecePos / 8, allCurrPositions, true,currColor);
			Bitboard moveGeneratedBitboard = generatedBitboards.moveBitboard;
			Bitboard capGeneratedBitboard = generatedBitboards.capBitboard;

			if (numOfCheck == 1) {
				localMoveBitboard |= (moveGeneratedBitboard & checkerToKingBB);
				//Since it is check, it cannot possibly capture something to block the check except the attacker
				//capBitboard |= (generatedBitboards[1] & checkerToKingBB);
				localCapBitboard |= (checkerLocations[0] & capGeneratedBitboard);
			} else {
				localMoveBitboard |= moveGeneratedBitboard;
				localCapBitboard |= capGeneratedBitboard;
			}

			//Might be dangerous deleting elements while looping even when going backwards
			for (int i = pinnedPieces.size() - 1; i >= 0; i--) {
				PinnedPieceData pinnedPieceInstance = pinnedPieces[i];
				if (getBit(piece, pinnedPieceInstance.pos%8, pinnedPieceInstance.pos/8)) {
					localMoveBitboard &= pinnedPieceInstance.posMoveMask;
					localCapBitboard &= pinnedPieceInstance.posMoveMask;
					pinnedPieces.erase(pinnedPieces.begin() + i);
				}
			}
			SinglePiecePosMoves singlePieceMoveBBs;
			singlePieceMoveBBs.moveBitboard = localMoveBitboard; 
			singlePieceMoveBBs.capBitboard = localCapBitboard;
			singlePieceMoveBBs.posBitboard = piece;
			allMovesBitboard.pieceTypes[ pieceToNumber[ everyPiece.pieceType ] ].posBB.push_back(singlePieceMoveBBs);
		}
	}
	return allMovesBitboard;
}

MoveMag kingOppDir(MoveMag dir, int kingPos) {
	int kingX = kingPos % 8;
	int kingY = kingPos / 8; 
	//Not sure if this is correct, check
	return { dir[0] * -1, dir[1] * -1, dir[2] - max(kingX,kingY)};
}

CheckData checkChecks(AllCurrPositions allCurrPositions, bool currColor) {
	CheckData res;
	int& numOfChecks = res.numOfChecks;
	vector<Bitboard>& checkerLocations = res.checkerLocations;
	//Can remove this line of code and just modify the allcurrpositions param. Optimize. Not really anymore as I use the param to get the original vector of bbs, can change this though.
	AllCurrPositions kingMorphedPositions = allCurrPositions;
	//Loops over all types of pieces, (not all pieces on the board as the name of the variable might suggest)
	for (char pieceType : pieces) {
		if (pieceType == 'k') {
			continue;
		}
		//Optimize:
		Bitboard kingPosBB = kingMorphedPositions.colorBitboards[currColor].pieceTypes[pieceToNumber['k']].posBB[0];
		int kingPos = _tzcnt_u64(kingPosBB);

		kingMorphedPositions.colorBitboards[currColor].pieceTypes[pieceToNumber['k']].posBB = {};
		kingMorphedPositions.colorBitboards[currColor].pieceTypes[pieceToNumber[pieceType]].posBB.push_back(kingPosBB);
		
		MoveCapAndPinnedBBs gennedBitboards = genBitboard('k', kingPos % 8, kingPos / 8, kingMorphedPositions, true, currColor);
		Bitboard gennedCapBitboard = gennedBitboards.capBitboard;
		Bitboard checkersBB = gennedBitboards.capBitboard & allCurrPositions.colorBitboards[currColor].pieceTypes[pieceToNumber[pieceType]].pieceTypeCombinedBB;

		//Pretty sure you don't need to add the checkers if you have at least 2 as if you do you can only move your king thus making their position useless.
		//Might be wrong.
		while (checkersBB != 0 || numOfChecks!=2) {
			numOfChecks++;
			int checkerCurrPosition = _tzcnt_u64(checkersBB);
			checkerLocations.push_back(1ULL << checkerCurrPosition);
			setBitTo(&checkersBB, checkerCurrPosition % 8, checkerCurrPosition / 8, 0);
		}
	}
	//numOfChecks and checkerLocations are references thus I can just return res.
	return res;
}

void calcCombinedPos(AllCurrPositions& allCurrPositions) {
	for (int color=0; color < 2; color++) {
		Bitboard colorCombinedBitboard = 0;
		for (int i=0; i < 6; i++) {
			Bitboard pieceTypeCombinedBitboard = 0;
			for (Bitboard positionBitboard : allCurrPositions.colorBitboards[color].pieceTypes[i].posBB) {
				pieceTypeCombinedBitboard |= positionBitboard;
			}
			allCurrPositions.colorBitboards[color].pieceTypes[i].pieceTypeCombinedBB = pieceTypeCombinedBitboard;
			colorCombinedBitboard |= pieceTypeCombinedBitboard;
		}
		allCurrPositions.colorBitboards[color].colorCombinedBB = colorCombinedBitboard;
	}
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

//vector<int> pieceCoordinates(char piece, char** board) {
//	vector<int> arr;
//	for (int i = 0; i < 8; i++) {
//		for (int j = 0; j < 8; j++) {
//			if (board[i][j] == piece) {
//				arr.push_back(i);
//				arr.push_back(j);
//			}
//		}
//		if (arr[0] != -1) {
//			break;
//		}
//	}
//	return arr;
//}

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
