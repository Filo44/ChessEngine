#include "SearchAlgorithm.h"
int amountOfLeafNodes = 0;
int captures = 0;
int enPassant = 0;
int totPos = 0;
//int amOfEnPassantXORAdds = 0;
//int amOfEnPassantXORRemovals = 0;
int hypos = 0;
unordered_map<ZobristHash, LeafNodesAndCurrPos> transpositionTablePerft = {};
extern unordered_map<ZobristHash, EvalAndBestMove> transpositionTable = {};
vector<string> movesAndLeafNodes = {};
unordered_map<char, double> pieceTypeToVal = {
	{rook, 5.00},
	{knight, 3.00},
	{bishop, 3.00},
	{queen, 9.00},
	{pawn, 1.00}
};


EvalAndBestMove minMax(AllCurrPositions& allCurrPositions, bool color, int depthCD, ZobristHash currZobristHash, double alpha, double beta, double cutOffTime) {
	if (transpositionTable.find(currZobristHash) != transpositionTable.end() && depthCD <= transpositionTable[currZobristHash].depth) {
		return transpositionTable[currZobristHash];
	}

	MovesVectAndPawnAtt resultOfSearch = fullMoveGenLoop(color, allCurrPositions, currZobristHash);
	vector<MoveDesc>& posMoves = resultOfSearch.moves;

	if (depthCD > -1) {
		double bestEval = color ? (-INFINITY) : (INFINITY);
		MoveDesc bestMove;
		bestMove.nullMove = true;
		orderMoves(posMoves, allCurrPositions, resultOfSearch.pawnAttacking);
		for (MoveDesc move : posMoves) {
			if (time(nullptr) > cutOffTime) {
				EvalAndBestMove abortedRes;
				abortedRes.eval = bestEval;
				abortedRes.depth = depthCD;
				abortedRes.bestMove = bestMove;
				abortedRes.noMoves = bestMove.nullMove;
				abortedRes.abortedDueToTime = true;
				cout << "Aborted due to going over the time available" << endl;
				return abortedRes;
			}
			AllCurrPositions newPositionsAfterMove = allCurrPositions;
			ZobristHash localZobristHash = newPositionsAfterMove.applyMove(move, currZobristHash);
			calcCombinedPos(newPositionsAfterMove);

			//If it's white, you want to take the best move, thus the higher val. Opposite for black
			EvalAndBestMove result = minMax(newPositionsAfterMove, !color, depthCD - 1, localZobristHash, alpha, beta, cutOffTime);
			if (result.abortedDueToTime) {
				EvalAndBestMove abortedRes;
				abortedRes.eval = bestEval;
				abortedRes.depth = depthCD;
				abortedRes.noMoves = bestMove.nullMove;
				abortedRes.bestMove = bestMove;
				abortedRes.abortedDueToTime = true;
				return abortedRes;
			}

			//Searching for color's moves,
			if (color ? (result.eval > bestEval) : (result.eval < bestEval)) {
				bestEval = result.eval;
				bestMove = move;
				if (color) {
					alpha = max(alpha, bestEval);
				}
				else {
					beta = min(beta, bestEval);
				}
				if (beta <= alpha) {
					break;
				}
			}
		}

		EvalAndBestMove& posSearchRes = transpositionTable[currZobristHash];
		posSearchRes.eval = bestEval; //If there have been no moves(Checkmate) will be negative infinity if white and pos infinity if black
		posSearchRes.depth = depthCD;
		posSearchRes.noMoves = bestMove.nullMove;
		posSearchRes.bestMove = bestMove;
		return posSearchRes;
	}
	else {
		EvalAndBestMove res;
		res.eval = simpleEval(allCurrPositions, color, currZobristHash);
		return res;
	}
}

int perft(AllCurrPositions allCurrPositions, bool color, int depthCD, ZobristHash currZobristHash, int initDepth) {
	if (transpositionTablePerft.find(currZobristHash) != transpositionTablePerft.end() && depthCD == transpositionTablePerft[currZobristHash].depth) {
		return transpositionTablePerft[currZobristHash].leafNodes;
	}
	if (depthCD != 0) {
		uint64_t totalOfLeafsCaused = 0;
		vector<MoveDesc> posMoves = fullMoveGenLoop(color, allCurrPositions, currZobristHash).moves;

		for (const MoveDesc& move : posMoves) {

			AllCurrPositions newPositionsAfterMove = allCurrPositions;
			ZobristHash localZobristHash = newPositionsAfterMove.applyMove(move, currZobristHash);
			calcCombinedPos(newPositionsAfterMove);

			//REMOVE AFTER(Remove the buffer)
			int bufferLeafsCaused = perft(newPositionsAfterMove, !color, depthCD - 1, localZobristHash, initDepth);
			totalOfLeafsCaused += bufferLeafsCaused;
			//REMOVE AFTER
			if (initDepth == depthCD) {
				//cout << "String rep of board: " << convertToString(allPositionBitboardsToMatrix(allCurrPositions)) << endl;
				movesAndLeafNodes.push_back(moveToUCI(move) + ": " + to_string(bufferLeafsCaused));
			}
		}

		LeafNodesAndCurrPos ttData;
		ttData.leafNodes = totalOfLeafsCaused;
		ttData.depth = depthCD;
		transpositionTablePerft[currZobristHash] = ttData;
		return totalOfLeafsCaused;
	}
	else {
		return 1;
	}
}

ZobristHash applyMovesTo(AllCurrPositions& allCurrPositions, vector<MoveDesc> movesTo, ZobristHash currZobristHash) {
	//May have to reverse for loop
	for (int i = movesTo.size() - 1; i >= 0; i--) {
		MoveDesc move = movesTo[i];
		allCurrPositions.applyMove(move, currZobristHash);
	}
	return currZobristHash;
}

double simpleEval(AllCurrPositions allCurrPositions, bool colorToMove, ZobristHash currZobristHash) {
	double total = 0;
	for (int color = 0; color < 2; color++) {
		int mult = (color) ? 1 : -1;
		for (int pieceType = 0; pieceType < 6; pieceType++) {
			// didn't want to change it because it just makes more sense in this function
			if (pieceType == king) {
				continue;
			}
			Bitboard pieceTypePosBitboard = allCurrPositions.pieceTypePositions[pieceType];
			total += (amOfSetBits(pieceTypePosBitboard) * pieceTypeToVal[pieceType]) * mult;
		}
	}
	vector<MoveDesc> posMoves = fullMoveGenLoop(colorToMove, allCurrPositions, currZobristHash).moves;
	if (posMoves.size() == 0) {
		if (checkChecks(allCurrPositions, colorToMove).numOfChecks != 0) {
			//If it is white to move and it has no moves, and it is in check(Thus checkmate) returns -INFINITY, opp for black
			return colorToMove ? -INFINITY : INFINITY;
		}
		else {
			//If it is anyone to move and it has no moves but it isn't in check(Thus stalemate) returns 0;
			return 0;
		}
	}
	return total;
}

void orderMoves(vector<MoveDesc>& moves, AllCurrPositions& allCurrPositions, Bitboard pawnAttacking) {
	sort(begin(moves), end(moves), [&allCurrPositions, &pawnAttacking](auto const& move1, auto const& move2) {
		//This is the correct operator(i.e. ">")
		return guessEval(move1, allCurrPositions, pawnAttacking) > guessEval(move2, allCurrPositions, pawnAttacking);
		});
}

int guessEval(MoveDesc move, AllCurrPositions& allCurrPositions, Bitboard pawnAttacking) {
	int boostFactor = 0;
	int normalizedPieceType = move.pieceType > 5 ? move.pieceType - 6 : move.pieceType;
	if (move.moveOrCapture == CAPTURE) {
		int pieceTypeCapturing = allCurrPositions.searchPieceType(move.posOfMove, !move.pieceMovingColor);
		if (!move.pieceMovingColor) {
			pieceTypeCapturing -= 6;
		}
		boostFactor += 10 * pieceTypeToVal[pieceTypeCapturing] - pieceTypeToVal[normalizedPieceType];
	}
	if (move.promotingToPiece != -1) {
		boostFactor = pieceTypeToVal[move.promotingToPiece > 5 ? move.promotingToPiece - 6 : move.promotingToPiece];
	}
	if (getBit(pawnAttacking, move.posOfMove)) {
		boostFactor -= pieceTypeToVal[normalizedPieceType];
	}

	return boostFactor;
}