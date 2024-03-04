#include "MoveGen.h"


using namespace std;

vector<int> pieceCoordinates(char piece, char** board);
//REMEMBER TO INITIALIZE THIS IN THE CHESS.CPP FILE IF YOU CAN'T FIX THE BUG

/*int main() {

}*/



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
Eigen::Matrix<bool, 8,8> genBitBoard(char piece, int x, int y,char** board,  bool attacking) {
	Eigen::Matrix<bool, 8, 8> bitboard;
	bitboard << genEmptyBitboard();
	char loweredPiece = tolower(piece);
	bool color = !(piece == loweredPiece);
	if ( loweredPiece=='b' || loweredPiece=='q') {
		
	}
	else if (loweredPiece == 'n') {
		// All possible moves of a knight
		int possX[8] = { 2, 1, -1, -2, -2, -1, 1, 2 };
		int possY[8] = { 1, 2, 2, 1, -1, -2, -2, -1 };
		for (int i = 0; i < 8; i++) {
			int nY = y + possY[i];
			int nX = x + possX[i];
			if (checkBounds(nX, nY)) {
				bitboard(nY, nX) = true;
			}
		}
	}
	else if (loweredPiece == 'p') {

	}
	return bitboard;
}

Eigen::Matrix<bool, 8, 8> genAllBitBoards(char** board, char color, bool attacking) {
	Eigen::Matrix<bool, 8, 8> fullBitboard;
	fullBitboard << genEmptyBitboard();

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			char piece = board[i][j];
			//Checks if piece is a color
			if (isupper(piece) == color) {
				fullBitboard = genBitBoard(piece, j, i, board, attacking).array() || fullBitboard.array();
			}
		}
	}
	return fullBitboard;
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
		if (arr[0]!=-1) {
			break;
		}
	}
	return arr;
}

bool checkBounds(int x, int y) {
	return (x >= 0 && y >= 0 && x < 8 && y < 8);
}

Eigen::Matrix<bool, 8, 8> genEmptyBitboard() {
	Eigen::Matrix<bool, 8, 8> emptyBB;
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