#include "Global.h"
#include "MoveGen.h"
#include "Chess.h"


using namespace std;

vector<int> pieceCoordinates(char piece, char** board);
//REMEMBER TO INITIALIZE THIS IN THE CHESS.CPP FILE IF YOU CAN'T FIX THE BUG

vector<Bitboard> main2() {
	Bitboard existanceBB;
	existanceBB << genEmptyBitboard();
	existanceBB(2, 2) = true;
	int x = 3;
	int y = 3;
	vector<Bitboard> tester = arrayToVector(genBitboard('P', x, y, existanceBB));
	Bitboard position;
	position << genEmptyBitboard();
	position(y, x) = true;
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

//
array<Bitboard,2> genBitboard(char piece, int x, int y, Bitboard oppColorBB) {
	Bitboard capBitboard;
	Bitboard moveBitboard;
	array<Bitboard, 2> bitboards;

	capBitboard << genEmptyBitboard();
	moveBitboard << genEmptyBitboard();
	char loweredPiece = tolower(piece);
	//White:true, black:false
	bool color = !(piece == loweredPiece);
	//Two ifs in case it is a queen and then just else ifs
	
	
	//Sliding Pieces

	//Ineficient, use magic bitboards later

	//Creating a direction vector of coordinates and magnitudes(Max distance so you don't have to check if you are in bounds)
	vector<MoveMag> dirs;
	//Will add the directions of the sliding pieces such that I won't have to have it twice, and no function required
	if ( loweredPiece=='b' || loweredPiece=='q') {
		if (x !=0 && y!=0) {
			MoveMag topLeft = { -1,-1,min(x,y) };
			dirs.push_back(topLeft);
		}
		if (x != 0 && y!=7) {
			MoveMag bottomLeft = { -1,1,min(x,7-y) };
			dirs.push_back(bottomLeft);
		}
		if (x != 7 && y != 7) {
			MoveMag bottomRight= { 1,1,min(7-x,7 - y) };
			dirs.push_back(bottomRight);
		}
		if (x != 7 && y != 0) {
			MoveMag topRight = { 1,-1,min(7 - x,y) };
			dirs.push_back(topRight);
		}
	}if (loweredPiece == 'r' || loweredPiece == 'q') {
		if (x != 0) {
			MoveMag left = { -1,0,x};
			dirs.push_back(left);
		}
		if (x != 7) {
			MoveMag right = { 1,0,7-x};
			dirs.push_back(right);
		}
		if (y != 0) {
			MoveMag top = { 0,-1,y};
			dirs.push_back(top);
		}
		if (y != 7) {
			MoveMag bottom = { 0,1,7-y };
			dirs.push_back(bottom);
		}
	}
	//Finishing the sliding pieces
	if (!dirs.empty()) {
		for (MoveMag dir : dirs) {
			int i = 1;
			//Plus one because we start at one and therefore since the x is zero indexed we need to go one more
			//Refer to the beatiful paint document for MATHEMATICAL proof.
			int nX = x;
			int nY = y;
			while (i < (dir[2]+1)) {
				nX += dir[0];
				nY += dir[1];
				//Checks if true, if it is true it means there is a piece of the opposite color and you should ...
				if (oppColorBB(nY,nX)) {
					//Append it to the capture bb
					capBitboard(nY, nX) = true;
					//Break out of the loop
					break;
				}
				//No need for an else statment as it breaks if it is true
				moveBitboard(nY, nX) = true;
				i++;
			}
		}
	}else if (loweredPiece == 'n') {
		cout << "Knight" << endl;
		// All possible moves of a knight
		int possX[8] = { 2, 1, -1, -2, -2, -1, 1, 2 };
		int possY[8] = { 1, 2, 2, 1, -1, -2, -2, -1 };
		for (int i = 0; i < 8; i++) {
			cout << "Knight"<<i << endl;
			int nY = y + possY[i];
			int nX = x + possX[i];
			if (checkBounds(nX, nY)) {
				cout << "In bounds" << i << endl;
				if (oppColorBB(nY, nX)) {
					capBitboard(nY, nX) = true;
				} else {
					moveBitboard(nY, nX) = true;
				}
			}
		}
	}
	else if (loweredPiece == 'p') {
		//0 indexed btw
		int startRank;
		int lastRank;
		//Checking if white
		if (color) {
			//Inverted because of how stupid matricies are made in programming languages
			startRank = 6;
			lastRank = 0;
		}
		else {
			startRank = 1;
			lastRank = 7;
		}

		//Checking if they can go forward once
		//Ignore the grey dotted line under the y, false positive:
		//https://stackoverflow.com/questions/71013618/understanding-the-sub-expression-overflow-reasoning
		//y>lastfile is useless, optimize. Can't be on the last file
		int forwards = (color ? -1 : 1);
		bool notOverY = ((color && y < 7) || (!color && y > 0));
		if (notOverY && !oppColorBB(y + forwards, x)) {
			moveBitboard(y + forwards, x) = true;
		}
		//Checking if it can go forwards twice
		if (y == startRank && !oppColorBB(y + (forwards * 2), x)) {
			moveBitboard(y + (forwards * 2), x) = true;
		}
		//Take to the left
		if (x > 0 && notOverY && oppColorBB(y + forwards, x - 1)) {
			capBitboard(y + forwards, x - 1) = true;
		}
		if (x < 7 && notOverY && oppColorBB(y + forwards, x + 1)) {
			capBitboard(y + forwards, x + 1) = true;
		}
		//Do En passant
		//EN PASSANT c'est le pire, je deteste (Squiggly line C)a. 
	}

	//RETURN BOTH BITBOARDS
	bitboards[0] = moveBitboard;
	bitboards[1] = capBitboard;
	return bitboards;
}

//Bitboard genAllBitBoards(char** board, char color, bool attacking) {
//	Bitboard fullBitboard;
//	fullBitboard << genEmptyBitboard();
//
//	for (int i = 0; i < 8; i++) {
//		for (int j = 0; j < 8; j++) {
//			char piece = board[i][j];
//			//Checks if piece is a color
//			if (isupper(piece) == color) {
//				fullBitboard = genBitboard(piece, j, i, board, ).array() || fullBitboard.array();
//			}
//		}
//	}
//	return fullBitboard;
//}

vector<int> pieceCoordinates(char piece, char** board) {
	vector<int> arr;
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if (board[i][j] == piece) {
				arr.push_back(i);
				arr.push_back(j);
			}
		}
		if (arr[0]!=-1) {
			break;
		}
	}
	return arr;
}

bool checkBounds(int x, int y) {
	return (x >= 0 && y >= 0 && x < 8 && y < 8);
}

Bitboard genEmptyBitboard() {
	Bitboard emptyBB;
	emptyBB << false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false;
	return emptyBB;
}

vector<Bitboard> arrayToVector(array<Bitboard, 2> arr) {
	vector<Bitboard> v;

	for (int i = 0; i < 2; i++) {
		v.push_back(arr[i]);
	}
	return v;
}