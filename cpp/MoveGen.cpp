#include "Global.h"
#include "MoveGen.h"
#include "Chess.h"


using namespace std;

vector<int> pieceCoordinates(char piece, char** board);
//REMEMBER TO INITIALIZE THIS IN THE CHESS.CPP FILE IF YOU CAN'T FIX THE BUG

vector<Bitboard> main2() {
	Bitboard existanceBB = 0;
	setBitTo(&existanceBB, 2, 2, 1);
	int x = 3;
	int y = 3;
	vector<Bitboard> tester = arrayToVector(genBitboard('P', x, y, existanceBB));
	Bitboard position = 0;
	setBitTo(&position, x, y, 1);
	tester.push_back(position);
	cout << convertVofBBJS(tester) << endl;
	return tester;
}



//Change type, and return. 
int* isChecked(char** board, char color) {
	int kX = -1;
	int kY = -1;
	//Can optimize king color checking, going to keep it like this for readability
	vector<int> coordArr = pieceCoordinates((color == 'w' ? 'K' : 'k'), board);
	kX = coordArr[0];
	kY = coordArr[1];
	int bitboard[64] = {};
	return 0;
}

//YOU DON'T STORE THE BITBOARDS GENEREATED BY THIS FUNCTION
//This just gives you the moves for the pieces which you will feed into the search algorithm
array<Bitboard, 3> genBitboard(char piece, int x, int y, Bitboard oppColorPosBB, bool pseudo, int kingPos) {
	Bitboard capBitboard;
	Bitboard moveBitboard;
	//Stores position(Int) and the ORed bitboard of the Sliding piece's dir 
	// and the king's opposite which is where the pinned piece can move obviously after being ANDed with that piece's pseudo moves
	vector<ClassWhichHasAnIntposAndBitboardSeeAbove> pinnedPiecesPositions;
	array<Bitboard, 3> bitboards;

	capBitboard = 0;
	moveBitboard = 0;
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
			Bitboard localCapBitboard=0;
			Bitboard localMoveBitboard=0;
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
				}
				//No need for an else statment as it breaks if it is true
				setBitTo(&localMoveBitboard, nX, nY, 1);
				i++;
			}
			//GenKingOppDir gens only capture bitboard
			if (_tzcnt_u64( (genKingOppDir(dir) & localCapBitboard) & oppColorPosBB) == somethingthatindicatesnosigbit) {

			}
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
	bitboards[0] = moveBitboard;
	bitboards[1] = capBitboard;
	bitboards[2] = pinnedPiecesPositions;
	return bitboards;
}

array<Bitboard, 2> genKingLegalMoves(int x, int y, Bitboard oppColorPosBB, Bitboard oppColorPseudoAttackBB) {
	//The oppColorPseudoAttackBB is a bitboard of the move and capture pseudo bitboards(ORed together) of the opposite color
	//Pseudo because it calculates the king moves without thinking about checks or anything else...
	//such that I can actually generate the actual king moves of the opposite-colored king

}

something fullMoveGenLoop(bool currentColor, AllPositionBitboards allPositionBitboards) {
	Bitboard capBitboard=0;
	Bitboard moveBitboard=0;
	newClassWhichStoresCapandMoveBBAndposBB EveryPieceBitboards;
	vector<int> pinnedPieces;
	array<Bitboard, 2> bitboards;
	someTypeThatAllowsAnArrayWithDifferentTypes checkChecksRes = checkChecks();
	int numOfCheck = checkChecksRes[0];
	//The bitboard will have two 1s in the case of 2 checkers, or more(If possible). 
	// Won't matter as it doesn't check the checkerLocBB if it has 2 checkers
	Bitboard checkerLocBB = checkChecksRes[1];
	//Set the pieceToNum to something static/use #define
	//Only one king thus why I accessed the [0] (0th)/(first) element of the vector
	int kingPos = _tzcnt_u64(allPositionBitboards.colorBitboards[currentColor].pieceTypes[pieceToNumber['k']].posBB[0]);
	if (numOfCheck == 2) {
		//passing down kingPos is useless but I think it will return an error if I just don't feed anything of the right type in.
		array<Bitboard,2> legalKingMoves = genBitboard('k', kingPos%8, kingPos/8, allPositionBitboards.colorBitboards[currentColor].colorCombinedBB, kingPos);
		moveBitboard = legalKingMoves[0];
		capBitboard = legalKingMoves[1];
	} else {
		if (numOfCheck == 1) {
			//Generate the checker to king bitboard
			Bitboard checkerToKingBB;
		}
		//Everypiece is of the class PieceTypeBBStorer
		for (Bitboard piece : everypiece) {
			Bitboard localCapBitboard = 0;
			Bitboard localMoveBitboard = 0;
			array<Bitboard, 3> generatedBitboards=genBitboard(piece);
			//pinnedPieces are of the opposite colour
			pinnedPieces.push_back(generatedBitboards[2]);
			if (numOfCheck == 1) {
				localMoveBitboard |= (generatedBitboards[0] & checkerToKingBB);
				//Since it is check, it cannot possibly capture something to block the check except the attacker
				//capBitboard |= (generatedBitboards[1] & checkerToKingBB);
				localCapBitboard |= (checkerLocBB & generatedBitboards[1]);
			} else {
				localMoveBitboard |= generatedBitboards[0];
				localCapBitboard |= generatedBitboards[1];
			}
			ChildClassOfnewClassWhichStoresCapandMoveBBAndposBB singleBitboards;
			singleBitboards.moveBitboard = localMoveBitboard;
			singleBitboards.capBitboard = localCapBitboard;
			singleBitboards.posBitboard = piece;
			EveryPieceBitboards.push_back(singleBitboards);
		}
	}

	
 }




vector<int> pieceCoordinates(char piece, char** board) {
	vector<int> arr;
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if (board[i][j] == piece) {
				arr.push_back(i);
				arr.push_back(j);
			}
		}
		if (arr[0] != -1) {
			break;
		}
	}
	return arr;
}

bool checkBounds(int x, int y) {
	return (x >= 0 && y >= 0 && x < 8 && y < 8);
}

vector<Bitboard> arrayToVector(array<Bitboard, 2> arr) {
	vector<Bitboard> v;

	for (int i = 0; i < 2; i++) {
		v.push_back(arr[i]);
	}
	return v;
}
