#include "Chess.h"
#include "MoveGen.h"
#include "Classes.h"
#include "SearchAlgorithm.h"
#include "ZobristSeed.h"

using namespace std;

int main(int argc, char* argv[]) {
	int depth = 4;
	int port = 8080;
	bool color;
	string lFen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
	//string lFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	if (argc > 1) {
		depth = stoi(argv[1]);
		if (argc > 2) {
			lFen = argv[2];
			cout << "FEN: " << lFen << endl;
			if (argc > 3) {
				port = stoi(argv[3]);
				if (argc > 4) {
					color = stoi(argv[4]) == 1;
					cout << "Color: " << color << endl;
				}
			}
		}
	}

	cout << "Started" << endl;
	httplib::Server svr;

	PosAndColor gameState = fenToPosBitboards(lFen);
	AllCurrPositions allPositionBitboards = gameState.allCurrPositions;
	color = gameState.color; //REMOVE THIS FOR THE ONE TO ONE FOR THE CHESS ENGINES!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	cout << "Calculated gamestate" << endl;
	//cout << "Board: " << convertToString(allPositionBitboardsToMatrix(allPositionBitboards), 8, 8) << endl;
	ZobristHash currZobristHash = genInitZobristHash(allPositionBitboards, color);
	cout << "Calculated the zobrist hash" << endl;
	//vector<MoveDesc> posMoves = fullMoveGenLoop(color, allPositionBitboards, currZobristHash);
	//cout << "Finished searching. Amount of moves found: " << posMoves.size() << endl;
	//cout << "Moves: " << endl << convertVectorOfMovesToJs(posMoves) << endl;
	//cout << "CurrPos: " << convertToString(allPositionBitboardsToMatrix(allPositionBitboards), 8, 8) << endl;
	//cout << "Zobrist hash: " << currZobristHash << endl;

	//MoveDesc move;
	//move.pieceMovingColor = BLACK;
	//move.moveOrCapture = MOVE;
	//move.pieceType = blackPawn;
	//move.posOfMove = 27;
	//move.posFrom = 11;

	//color = !color;
	//depth--;
	//currZobristHash = allPositionBitboards.applyMove(posMoves[14], currZobristHash);
	//calcCombinedPos(allPositionBitboards);
	//posMoves = fullMoveGenLoop(color, allPositionBitboards, currZobristHash);
	//cout << "Finished searching. Amount of moves found: " << posMoves.size() << endl;
	//cout << "allPositionBitboards.pawnWhoDoubleMovedPos: " << allPositionBitboards.pawnWhoDoubleMovedPos << endl;
	//cout << "Moves: " << endl << convertMovesVectorToUCIMoves(posMoves) << endl;
	//cout << "CurrPos: " << convertToString(allPositionBitboardsToMatrix(allPositionBitboards), 8, 8) << endl;
	//cout << "Zobrist hash: " << currZobristHash << endl;

	//amountOfLeafNodes = 0;
	//captures = 0;
	//enPassant = 0;
	//totPos = 0;
	//transpositionTablePerft = {};
	//movesAndLeafNodes = {};
	////transpositionTable = {};
	//cout << "Starting the perft search " << endl;
	//uint64_t actualAmountOfLeafNodes = perft(allPositionBitboards, color, depth, currZobristHash, depth);
	//cout << "actualAmountOfLeafNodes: " << actualAmountOfLeafNodes << endl;
	//cout << "enPassant: " << enPassant << endl;
	//cout << "captures: " << captures << endl;
	//cout << "totPos: " << totPos << endl;
	//for (string moveAndLeafNode : movesAndLeafNodes) {
	//	cout << moveAndLeafNode << endl;
	//}
	//cout << "FINISHED" << endl;
	//return actualAmountOfLeafNodes;

	unordered_map<ZobristHash, EvalAndBestMove> transpositionTable = {};
	EvalAndBestMove res = iterativeSearch(allPositionBitboards, color, currZobristHash, timeManagementFunction(30));
	cout << "Eval " << res.eval << endl;
	cout << "Hi" << endl;


	//Fixes CORS errors
	svr.Options("/MoveResponse", [](const httplib::Request& /*req*/, httplib::Response& res) {
		res.set_header("Access-Control-Allow-Origin", "*");
		res.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
		res.set_header("Access-Control-Allow-Headers", "Content-Type");
		res.set_header("Access-Control-Max-Age", "3600"); // Optional: Cache preflight response for 1 hour
		res.set_content("", "text/plain");
		});
	svr.Options("/GetFirstMove", [](const httplib::Request& /*req*/, httplib::Response& res) {
		res.set_header("Access-Control-Allow-Origin", "*");
		res.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
		res.set_header("Access-Control-Allow-Headers", "Content-Type");
		res.set_header("Access-Control-Max-Age", "3600"); // Optional: Cache preflight response for 1 hour
		res.set_content("", "text/plain");
		});

	//// Handle GET requests
	//svr.Get("/data", [&allPositionBitboards, &posMoves](const httplib::Request& /*req*/, httplib::Response& res) {
	//	char** arr = allPositionBitboardsToMatrix(allPositionBitboards);
	//	res.set_content(convertToJSArr(arr, 8, 8), "text/plain");
	//	res.set_header("Access-Control-Allow-Origin", "*");
	//	delete2DArray(arr, 8);
	//	});
	//svr.Get("/eval", [searchRes](const httplib::Request& /*req*/, httplib::Response& res) {
	//    string eval = to_string(searchRes.eval);
	//    res.set_content(eval, "text/plain");
	//    res.set_header("Access-Control-Allow-Origin", "*");
	//});
	//Dummy function
	svr.Get("/eval", [](const httplib::Request& /*req*/, httplib::Response& res) {
		res.set_content("0", "text/plain");
		res.set_header("Access-Control-Allow-Origin", "*");
		});

	//svr.Get("/getBitboards", [posMoves](const httplib::Request& /*req*/, httplib::Response& res) {
	//	res.set_content(allPosMovesToMatrix(posMoves), "text/plain");
	//	res.set_header("Access-Control-Allow-Origin", "*");
	//	});
	svr.Post("/GetFirstMove", [&allPositionBitboards, &color, &currZobristHash](const httplib::Request& req, httplib::Response& res) {
		cout << "Requested first move" << endl;
		json json_data = json::parse(req.body);
		double timeLeft = json_data["timeLeft"];

		EvalAndBestMove resultOfMinMaxSearch = getMoveAndApplyFromPos(allPositionBitboards, currZobristHash, timeLeft, color);

		res.set_content(posAndGameStateToJS(allPositionBitboards, resultOfMinMaxSearch), "text/plain");
		res.set_header("Access-Control-Allow-Origin", "*");
		});
	svr.Post("/MoveResponse", [&allPositionBitboards, &color, &currZobristHash](const httplib::Request& req, httplib::Response& res) {
		cout << "Requesting move response" << endl;
		// Parse the JSON data from the request body
		cout << "Raw req.body: " << req.body << endl;
		json json_data = json::parse(req.body);
		cout << "JSON data" << json_data << endl;
		double timeLeft = json_data["timeLeft"];
		cout << "timeLeft: " << timeLeft << endl;

		cout << "Starting to parse" << endl;
		MoveDesc move = parseMove(json_data["prevMove"], allPositionBitboards);
		cout << "Finished parsing" << endl;

		// Apply the move and get the result
		currZobristHash = allPositionBitboards.applyMove(move, currZobristHash);
		calcCombinedPos(allPositionBitboards);

		cout << "Starting minMaxSearch" << endl;
		EvalAndBestMove resultOfMinMaxSearch = getMoveAndApplyFromPos(allPositionBitboards, currZobristHash, timeLeft, color);
		cout << "Finished minMaxSearch" << endl;

		// Convert the result to JSON and send it back
		res.set_content(posAndGameStateToJS(allPositionBitboards, resultOfMinMaxSearch), "text/plain");
		res.set_header("Access-Control-Allow-Origin", "*");
		});
	svr.Post("/exit", [](const httplib::Request& /*req*/, httplib::Response& res) {
		return;
		});

	// Run the server
	svr.listen("localhost", port);


	return 0;
}

PosAndColor fenToPosBitboards(std::string fen) {
	PosAndColor res = PosAndColor();
	AllCurrPositions allPositionBitboards;

	for (int i = 0; i < 12; i++) {
		allPositionBitboards.pieceTypePositions[i] = 0;
	}

	int maxI = -1;
	int actualPos = -1;
	for (int i = 0; i < fen.length(); i++) {
		if (fen[i] == '/') {
			continue;
		}
		else if (fen[i] == ' ') {
			maxI = i;
			break;
		}
		if (i == fen.length() - 1) {
			cout << "What? No space detected" << endl;
		}
		actualPos++;
		//std::cout << "actualPos:" << actualPos << std::endl;
		if (isdigit(fen[i])) {
			int currNum = fen[i] - '0';
			actualPos += currNum - 1;
			continue;
		}
		else {
			//Else is useless, looks better though. Shush.
			int y = actualPos / 8;
			//Keeps as int therefore rounds down. 
			int x = actualPos % 8;
			//Goes to the Piece type by checking hte Piece to number map
			//then sets the bit in that pos to one
			setBitTo(&allPositionBitboards.pieceTypePositions[pieceToNumber[fen[i]]], actualPos, 1);
		}
	}

	allPositionBitboards.castlingRights[0].canCastleKSide = false;
	allPositionBitboards.castlingRights[0].canCastleQSide = false;
	allPositionBitboards.castlingRights[1].canCastleKSide = false;
	allPositionBitboards.castlingRights[1].canCastleQSide = false;

	//If no space detected, i.e. maxI=-1, it goes to the default values
	if (maxI != -1) {
		if (fen[maxI + 1] == 'b') {
			//cout << "Black" << endl;
			res.color = 0;
		}
		else {
			//cout << "White" << endl;
			res.color = 1;
		}
		int maxJ = -1;
		if (fen[maxI + 3] == '-') {
			//cout << "No castling rights" << endl;
			maxJ = 4;
		}
		else {
			for (int j = 3; j < 7; j++) {
				char el = fen[maxI + j];
				if (el == ' ') {
					maxJ = j;
					break;
				}
				bool castleRightsColor = isupper(el);
				if (tolower(el) == 'k') {
					//cout << (castleRightsColor ? "White" : "Black") << " can castle king side. " << endl;
					allPositionBitboards.castlingRights[castleRightsColor].canCastleKSide = true;
				}
				else {
					//cout << (castleRightsColor ? "White" : "Black") << " can castle queen side. " << endl;
					allPositionBitboards.castlingRights[castleRightsColor].canCastleQSide = true;
				}
				if (j == 6) {
					maxJ = 7;
				}
			}
		}
		if (fen[maxI + maxJ + 1] != '-') {
			char file = fen[maxI + maxJ + 1];
			//cout << "file: " << file << endl;

			int rank = fen[maxI + maxJ + 2] - '0';
			//cout << "rankChar: " << fen[maxI + maxJ + 2]  << endl;

			int vulnY = 8 - rank;

			//Backwards for white is + 1
			int y = res.color ? vulnY + 1 : vulnY - 1;
			int x = file - 97;
			//cout << "En passant. X: "<<x<<", Y:" << y<<". " << endl;
			//cout << "!res.color: " << !res.color << endl;
			cout << "Pawn who double moved pos from FEN: " << x + (y * 8) << endl;
			allPositionBitboards.pawnWhoDoubleMovedPos = x + (y * 8);
		}
	}
	else {
		res.color = 1;
	}

	res.allCurrPositions = allPositionBitboards;
	return res;
}
char** allPositionBitboardsToMatrix(AllCurrPositions allPositionBitboardsL) {
	char** arr = new char* [8];
	for (int i = 0; i < 8; ++i) {
		arr[i] = new char[8];
	}
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			arr[i][j] = ' ';
		}
	}
	for (int pieceType = 0; pieceType < 12; pieceType++) {
		Bitboard currCheckingBitboard = allPositionBitboardsL.pieceTypePositions[pieceType];
		while (currCheckingBitboard != 0) {
			//Finds the last, and in this case only, 1
			int pos = _tzcnt_u64(currCheckingBitboard);

			//y, x for arrays
			arr[pos / 8][pos % 8] = pieces[pieceType];
			setBitTo(&currCheckingBitboard, pos, 0);
		}
	}
	return arr;
}
string allPosMovesToMatrix(AllPosMoves posMoves) {
	string s = "{ \"pieceTypes\": [";
	for (int i = 0; i < 6; i++) {
		PieceTypePosMoves pieceType = posMoves.pieceTypes[i];
		s += "{\"pieceChar\": ";
		string pieceTypeString{ pieceType.pieceType };
		s += "\"" + pieceTypeString + "\"";
		s += ", \"combinedCapBB\":";
		s += convertBBJS(pieceType.pieceTypeCombinedCapBB).str();
		s += ", \"combinedMoveBB\":";
		s += convertBBJS(pieceType.pieceTypeCombinedMoveBB).str();
		s += ", \"pieces\":[  ";
		for (int j = 0; j < pieceType.positionBitboard.size(); j++) {
			SinglePiecePosMoves piece = pieceType.positionBitboard[j];
			s += "{\"capBitboard\": ";
			s += convertBBJS(piece.capBitboard).str();
			s += ", \"moveBitboard\": ";
			s += convertBBJS(piece.moveBitboard).str();
			s += "}";
			if (j != (pieceType.positionBitboard.size() - 1)) {
				s += ", ";
			}
		}
		s += "]}";
		if (i != 5) {
			s += ", ";
		}
	}
	s += "]}";
	return s;
}

ZobristHash genInitZobristHash(AllCurrPositions currPositions, bool colorToMove) {
	//When the first XOR is done, since it is instantiated to zero, the currZobristHash will become that number.
	ZobristHash currZobristHash = 0;
	uint8_t castlingKey = 0b00000000;
	int enPassantFile = -1;

	for (int color = 0; color < 2; color++) {
		for (int pieceType = 0; pieceType < 6; pieceType++) {
			Bitboard pieceTypeBitboard = currPositions.pieceTypePositions[pieceType + (color * 6)];
			while (pieceTypeBitboard != 0) {
				int pos = _tzcnt_u64(pieceTypeBitboard);
				currZobristHash ^= ZobristSeed[color ? pieceType + 6 : pieceType][pos];
				setBitTo(&pieceTypeBitboard, pos, 0);
			}
		}
		//If it is white it multiplies everything by 4, or shifts it up twice
		int colorMult = color ? 4 : 1;
		if (currPositions.castlingRights[color].canCastleKSide) {
			castlingKey |= (0b00000001 * colorMult);
		}
		if (currPositions.castlingRights[color].canCastleQSide) {
			castlingKey |= (0b00000010 * colorMult);
		}

		// Maybe don't store the Piece but rather its position in terms of its file (Optimize)?
		if (currPositions.pawnWhoDoubleMovedPos != -1) {
			int enPassantVictimFile = currPositions.pawnWhoDoubleMovedPos % 8;
			//Since it(.pawnWhoDoubleMovedPos) is the position of the Piece who can get taken, that works just as well as a square which can get en passanted into.
			currZobristHash ^= EnPassantFileSeed[enPassantVictimFile];
		}
	}

	currZobristHash ^= CastlingSeed[castlingKey];
	if (colorToMove == false) {
		currZobristHash ^= SideToMoveIsBlack;


		return currZobristHash;
	}
}
void delete2DArray(char** arr, int rows) {
	for (int i = 0; i < rows; ++i) {
		delete[] arr[i];
	}
	delete[] arr;
}
string convertToString(char** a, int cols, int rows)
{
	string s = "\n - - - - - - - - - - - - \n - - - - - - - - - - - -";
	for (int i = 0; i < cols; i++) {
		s += "\n - - ";
		for (int j = 0; j < rows; j++) {
			if (a[i][j] != ' ') {
				s += a[i][j];
				s += ' ';
			}
			else {
				s += ". ";
			}
		}
		s += "- -";
	}
	s += "\n - - - - - - - - - - - - \n - - - - - - - - - - - -";
	delete2DArray(a, 8);
	return s;
}
string convertToJSArr(char** a, int cols, int rows)
{
	string s = "[";
	for (int i = 0; i < cols; i++) {
		s += "[";
		for (int j = 0; j < rows; j++) {
			s += "\"";
			s += a[i][j];
			s += "\"";
			if (j != rows - 1) {
				s += ",";
			}
		}
		if (i != cols - 1) {
			s += "],";
		}
		else {
			s += "]";
		}

	}
	s += "]";
	return s;
}

string convertVofBBJS(vector<Bitboard> matrixVector) {
	//cout << matrixVector[0] << endl;
	std::stringstream ss;
	ss << "[";
	int matrixVectorSize = matrixVector.size();
	for (size_t i = 0; i < matrixVector.size(); ++i) {
		Bitboard curBB = matrixVector[i];
		ss << convertBBJS(curBB).str();
		if (i < matrixVectorSize - 1) {
			ss << ",";
		}
	}
	ss << "]";
	string jsonString = ss.str();
	return jsonString;
}
stringstream convertBBJS(Bitboard curBB) {
	stringstream ss;
	ss << "[";
	for (int bit = 0; bit < 64; ++bit) {
		if (bit % 8 == 0) {
			ss << "[";
		}
		ss << (getBit(curBB, bit) ? "true" : "false");
		if (bit % 8 != 7) {
			ss << ",";
		}
		else {
			ss << "]";
			if (bit / 8 != 7) {
				ss << ",";
			}
		}
	}
	ss << "]";
	return ss;
}
string convertVectorOfMovesToJs(vector<MoveDesc> moves) {
	stringstream ss;
	ss << "[";
	int amOfMoves = moves.size();
	for (int i = 0; i < amOfMoves; ++i) {
		MoveDesc move = moves[i];
		ss << convertMoveToJS(move);
		if (i < amOfMoves - 1) {
			ss << ", \n";
		}
	}
	ss << "]";
	string jsonString = ss.str();
	return jsonString;
}

MoveDesc parseMove(const json moveStr, AllCurrPositions allCurrPositions) {
	// Your logic to convert moveStr to a MoveDesc object
	MoveDesc move;
	move.pieceMovingColor = (bool)moveStr["pieceMovingColor"];
	cout << "move.pieceMovingColor: " << move.pieceMovingColor << endl;
	move.pieceType = moveStr["pieceType"];
	cout << "move.pieceType: " << move.pieceType << endl;
	move.posOfMove = moveStr["posOfMove"];
	cout << "move.posOfMove: " << move.posOfMove << endl;
	move.moveOrCapture = (int)moveStr["moveOrCapture"];
	cout << "move.moveOrCapture :" << move.moveOrCapture << endl;
	move.enPassant = (int)moveStr["enPassant"];
	cout << "move.enPassant :" << move.enPassant << endl;
	move.promotingToPiece = moveStr["promotingToPiece"];
	cout << "move.promotingToPiece :" << move.promotingToPiece << endl;
	if (moveStr.contains("xFrom")) {
		move.posFrom = (int)moveStr["xFrom"] + ((int)moveStr["yFrom"] * 8);
	}
	else {
		move.posFrom = moveStr["posFrom"];
	}
	cout << "move.posFrom: " << move.posFrom << endl;
	return move;
}
string convertMoveToJS(MoveDesc move) {
	string res = "{\"pieceMovingColor\":";
	res += move.pieceMovingColor ? "true" : "false";
	res += ", \"pieceType\":";
	res += to_string(move.pieceType);
	res += ", \"posOfMove\":";
	res += to_string(move.posOfMove);
	res += ", \"moveOrCapture\":";
	res += to_string(move.moveOrCapture);
	res += ", \"posFrom\":";
	res += to_string(move.posFrom);
	res += ", \"enPassant\":";
	res += to_string(move.enPassant);
	res += ", \"promotingToPiece\":";
	res += to_string(move.promotingToPiece);
	res += "}";
	return res;
}

double timeManagementFunction(double timeRemaining) {
	return 30.00;
}

EvalAndBestMove iterativeSearch(AllCurrPositions allCurrPositions, bool color, ZobristHash currZobristHash, double timeAvailable) {
	double cutOffTime = (double)time(nullptr) + timeAvailable;
	extern unordered_map<ZobristHash, EvalAndBestMove> transpositionTable;
	EvalAndBestMove res;
	int depth = 2;
	while (time(nullptr) < cutOffTime) {
		cout << "Depth: " << depth << endl;

		//transpositionTable = {};
		EvalAndBestMove searchResults = minMax(allCurrPositions, color, depth, currZobristHash, -INFINITY, INFINITY, cutOffTime);

		//Currently ignores aborted searches
		if (!searchResults.abortedDueToTime && !searchResults.bestMove.nullMove) {
			res.eval = searchResults.eval;
			res.bestMove = searchResults.bestMove;

		}
		//if (depth == 5) {
		//	break;
		//}

		depth++;
	}
	return res;
}
EvalAndBestMove getMoveAndApplyFromPos(AllCurrPositions& allPositionBitboards, ZobristHash& currZobristHash, double timeLeft, bool color) {
	double timeAssigned = timeManagementFunction(timeLeft);
	EvalAndBestMove resultOfMinMaxSearch = iterativeSearch(allPositionBitboards, color, currZobristHash, timeAssigned);
	cout << "Best move: " << convertMoveToJS(resultOfMinMaxSearch.bestMove) << endl;

	currZobristHash = allPositionBitboards.applyMove(resultOfMinMaxSearch.bestMove, currZobristHash);
	calcCombinedPos(allPositionBitboards);
	return resultOfMinMaxSearch;
}

string posAndGameStateToJS(AllCurrPositions allPositionBitboards, EvalAndBestMove resultOfMinMaxSearch) {
	string res = "{\"newPos\": " + convertToJSArr(allPositionBitboardsToMatrix(allPositionBitboards), 8, 8) + ", \"move\":" + convertMoveToJS(resultOfMinMaxSearch.bestMove)
		+ ", \"canWhiteCastleKSide\":" + (allPositionBitboards.castlingRights[1].canCastleKSide ? "true" : "false")
		+ ", \"canWhiteCastleQSide\":" + (allPositionBitboards.castlingRights[1].canCastleQSide ? "true" : "false")
		+ ", \"canBlackCastleKSide\":" + (allPositionBitboards.castlingRights[0].canCastleKSide ? "true" : "false")
		+ ", \"canBlackCastleQSide\":" + (allPositionBitboards.castlingRights[0].canCastleQSide ? "true" : "false")
		+ "}";
	cout << "Responding: " << res;
	return res;
}
// Helper function to convert 0-63 index to UCI square notation
string squareToUCI(int pos) {
	char file = 'a' + (pos % 8);  // Calculate the file ('a' to 'h')
	char rank = '1' + (7 - (pos / 8));  // Calculate the rank ('1' to '8')
	return string(1, file) + string(1, rank);
}
// Function to convert MoveDesc to UCI move format
string moveToUCI(const MoveDesc& move) {
	string uciMove = squareToUCI(move.posFrom) + squareToUCI(move.posOfMove);

	// Check if the move is a promotion
	if (move.promotingToPiece != -1) {
		char promotionChar = pieces[move.promotingToPiece];
		uciMove += promotionChar;
	}
	return uciMove;
}

string convertMovesByPosToUCIMoves(MovesByPos moves) {
	vector<MoveDesc> movesVector;
	stringstream ss;
	for (int i = 0; i < 64; i++) {
		movesVector = addVectors(moves[i], movesVector);
	}
	for (MoveDesc move : movesVector) {
		ss << moveToUCI(move) << "\n";
	}
	return ss.str();
}
string convertMovesVectorToUCIMoves(vector<MoveDesc> movesVector) {
	stringstream ss;
	for (MoveDesc move : movesVector) {
		ss << moveToUCI(move) << "\n";
	}
	return ss.str();
}