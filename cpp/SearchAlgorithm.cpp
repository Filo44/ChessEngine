#include "SearchAlgorithm.h"
int amountOfLeafNodes = 0;
int captures = 0;
int enPassant = 0;
int totPos = 0;
//int amOfEnPassantXORAdds = 0;
//int amOfEnPassantXORRemovals = 0;
int hypos = 0;
unordered_map<ZobristHash, LeafNodesAndCurrPos> transpositionTablePerft = {};
unordered_map<ZobristHash, EvalAndBestMove> transpositionTable = {};


//ostream& operator<<(std::ostream& os, const LeafNodesAndCurrPos& obj) {
//	//os << "Leaf Nodes: " << obj.leafNodes << std::endl;
//	os << "All Position Matrix: " << obj.allPositionMatrix << std::endl;
//	os << "Can Castle WQ: " << obj.canCastleWQ << std::endl;
//	os << "Can Castle WK: " << obj.canCastleWK << std::endl;
//	os << "Can Castle BQ: " << obj.canCastleBQ << std::endl;
//	os << "Can Castle BK: " << obj.canCastleBK << std::endl;
//	os << "Double Moved Pawn: " << obj.doubleMovedPawn << std::endl;
//	os << "Color to Move: " << obj.colorToMove << std::endl;
//	return os;
//}


EvalAndBestMove minMax(AllCurrPositions allCurrPositions, bool color, int depthCD, ZobristHash currZobristHash, double cutOffTime) {
	if (transpositionTable.find(currZobristHash) != transpositionTable.end() && depthCD <= transpositionTable[currZobristHash].depth) {
		return transpositionTable[currZobristHash];
	}

	AllPosMoves posMoves = fullMoveGenLoop(color, allCurrPositions, currZobristHash);

	if (depthCD > -1) {
		OneColorCurrPositions colorCurrPositions = allCurrPositions.colorBitboards[color];
		double bestEval = color ? (-INFINITY) : (INFINITY);
		MoveDesc bestMove;
		bestMove.nullMove = true;

		for (int i = 0; i < 6; i++) {
			PieceTypePosMoves& pieceTypePosMoves = posMoves.pieceTypes[i];

			MoveDesc thisMove;
			thisMove.pieceType = i;
			thisMove.pieceMovingColor = color;

			for (int j = 0; j < pieceTypePosMoves.posBB.size(); j++) {
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
				thisMove.piece = j;
				//If this code is being run when the depth has gone under but there still are captures (!depthHasNotFinished && captureMovesExist)
				//Only check capture moves
				for (int moveOrCapture = 0; moveOrCapture < 2; moveOrCapture++) {

					Bitboard currBitboard = (moveOrCapture == 0) ? pieceTypePosMoves.posBB[j].moveBitboard : pieceTypePosMoves.posBB[j].capBitboard;
					thisMove.moveOrCapture = (bool)moveOrCapture;

					while (currBitboard != 0) {

						int posOfNextBit = _tzcnt_u64(currBitboard);

						// i = pieceType, j = piece
						//If its a pawn, and it is moving and its original x is not the same as the x to where it is moving it is en passant
						if (i == pieceToNumber['p'] && moveOrCapture == 0 && (_tzcnt_u64(colorCurrPositions.pieceTypes[i].posBB[j]) % 8) != (posOfNextBit % 8)) {
							enPassant++;
						}
						thisMove.posOfMove = posOfNextBit;
						setBitTo(&currBitboard, posOfNextBit, 0);

						AllCurrPositions newPositionsAfterMove = allCurrPositions;
						ZobristHash localZobristHash = newPositionsAfterMove.applyMove(thisMove, currZobristHash);
						calcCombinedPos(newPositionsAfterMove);

						//If it's white, you want to take the best move, thus the higher val. Opposite for black
						EvalAndBestMove result = minMax(newPositionsAfterMove, !color, depthCD - 1, localZobristHash, cutOffTime);
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
							bestMove = thisMove;
						}
					}
				}
			}

		}

		EvalAndBestMove posSearchRes;
		posSearchRes.eval = bestEval; //If there have been no moves(Checkmate) will be negative infinity if white and pos infinity if black
		posSearchRes.depth = depthCD;
		posSearchRes.noMoves = bestMove.nullMove;
		posSearchRes.bestMove = bestMove;
		transpositionTable[currZobristHash] = posSearchRes;
		return posSearchRes;
	} else{
		EvalAndBestMove res;
		res.eval = simpleEval(allCurrPositions, color, currZobristHash);
		return res;
	}
}

int perft(AllCurrPositions allCurrPositions, bool color, int depthCD, ZobristHash currZobristHash) {
	//Strict TT
	//if (transpositionTablePerft.find(currZobristHash) != transpositionTablePerft.end() && depthCD==transpositionTablePerft[currZobristHash].depth)
	//Loose TT, leaf nodes and other stats won't be the same

	////REMOVE AFTER
	//string stringRepOfBoardBeforeChanges = convertToString(allPositionBitboardsToMatrix(allCurrPositions), 8, 8);

	if (transpositionTablePerft.find(currZobristHash) != transpositionTablePerft.end() && depthCD <= transpositionTablePerft[currZobristHash].depth) {
		return transpositionTablePerft[currZobristHash].leafNodes;
	}
	depthCD--;
	totPos++;
	if (depthCD != -1) {
		uint64_t totalOfLeafsCaused = 0;
		AllPosMoves posMoves = fullMoveGenLoop(color, allCurrPositions, currZobristHash);

		OneColorCurrPositions colorCurrPositions = allCurrPositions.colorBitboards[color];
		for (int i = 0; i < 6; i++) {
			PieceTypePosMoves& pieceTypePosMoves = posMoves.pieceTypes[i];

			MoveDesc thisMove;
			thisMove.pieceType = i;
			thisMove.pieceMovingColor = color;

			for (int j = 0; j < pieceTypePosMoves.posBB.size(); j++) {
				thisMove.piece = j;
				for (int moveOrCapture = 0; moveOrCapture < 2; moveOrCapture++) {

					Bitboard currBitboard = (moveOrCapture == 0) ? pieceTypePosMoves.posBB[j].moveBitboard : pieceTypePosMoves.posBB[j].capBitboard;
					thisMove.moveOrCapture = (bool)moveOrCapture;

					while (currBitboard != 0) {
						//cout << "HELLO" << endl;

						int posOfNextBit = _tzcnt_u64(currBitboard);
						
						// i = pieceType, j = piece
						//If its a pawn, and it is moving and its original x is not the same as the x to where it is moving it is en passant
						if (i == pieceToNumber['p'] && moveOrCapture == 0 && (_tzcnt_u64(colorCurrPositions.pieceTypes[i].posBB[j]) % 8) != (posOfNextBit % 8)) {
							enPassant++;
						}
						thisMove.posOfMove = posOfNextBit;
						setBitTo(&currBitboard, posOfNextBit, 0);

						AllCurrPositions newPositionsAfterMove = allCurrPositions;
						ZobristHash localZobristHash = newPositionsAfterMove.applyMove(thisMove, currZobristHash);
						calcCombinedPos(newPositionsAfterMove);
						//cout << "Depth: " << depthCD << endl;
						if (depthCD == 0) {
							//cout << "Depth = 0" << endl;
							if (moveOrCapture==1) {
								captures++;
							}
						}
						totalOfLeafsCaused += perft(newPositionsAfterMove, !color, depthCD, localZobristHash);

						//REMOVE AFTER
						//(1billion)
						if (totalOfLeafsCaused > 1000000000) {
							string stringRepOfPos = convertToString(allPositionBitboardsToMatrix(allCurrPositions), 8, 8);
							string stringRepOfPosChanged = convertToString(allPositionBitboardsToMatrix(newPositionsAfterMove), 8, 8);
							cout << "Adding 1 bill" << endl;
							cout << "I hate breakpoints. " << endl;
						}
					}
				}
			}

		}
		LeafNodesAndCurrPos ttData;
		ttData.leafNodes = totalOfLeafsCaused;
		ttData.depth = depthCD;
		transpositionTablePerft[currZobristHash] = ttData;
		return totalOfLeafsCaused;
	} else {
		//amountOfLeafNodes++;
		return 1;
	}
}

ZobristHash applyMovesTo(AllCurrPositions& allCurrPositions, vector<MoveDesc> movesTo, ZobristHash currZobristHash) {
	//May have to reverse for loop
	for (int i = movesTo.size() - 1; i >=0; i--) {
		MoveDesc move = movesTo[i];
		allCurrPositions.applyMove(move, currZobristHash);
	}
	return currZobristHash;
}

double simpleEval(AllCurrPositions allCurrPositions, bool colorToMove, ZobristHash currZobristHash) {
	unordered_map<char, double> pieceTypeToVal = {
		{'r', 5.00},
		{'n', 3.00},
		{'b', 3.00},
		{'q', 9.00},
		{'p', 1.00}
	};
	double total = 0;
	for (int color = 0; color < 2; color++) {
		int mult = (color) ? 1 : -1;
		OneColorCurrPositions colorCurrPositions = allCurrPositions.colorBitboards[color];
		for (int i = 0; i < 6; i++) {
			if (pieceToNumber['k'] == i) {
				continue;
			}
			PieceTypeCurrPositions pieceTypeCurrPositions = colorCurrPositions.pieceTypes[i];
			total += (pieceTypeCurrPositions.posBB.size() * pieceTypeToVal[pieces[i]]) * mult;
		}
	}
	AllPosMoves posMoves = fullMoveGenLoop(colorToMove, allCurrPositions, currZobristHash);
	if (posMoves.combinedCapBB == 0 && posMoves.combinedMoveBB == 0) {
		if (checkChecks(allCurrPositions, colorToMove).numOfChecks != 0) {
			//If it is white to move and it has no moves, and it is in check(Thus checkmate) returns -INFINITY, opp for black
			return colorToMove ? -INFINITY : INFINITY;
		} else {
			//If it is anyone to move and it has no moves but it isn't in check(Thus stalemate) returns 0;
			return 0;
		}
	}
	return total;
}
