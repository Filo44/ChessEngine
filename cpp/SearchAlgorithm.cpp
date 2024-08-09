#include "SearchAlgorithm.h"
int amountOfLeafNodes = 0;
int captures = 0;
int enPassant = 0;
int totPos = 0;
//int amOfEnPassantXORAdds = 0;
//int amOfEnPassantXORRemovals = 0;
int hypos = 0;
unordered_map<ZobristHash, LeafNodesAndCurrPos> transpositionTablePerft = {};

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


//EvalAndMovesTo minMax(AllCurrPositions allCurrPositions, bool color, int depthCD) {
//	depthCD--;
//	if (depthCD != -1) {
//		AllPosMoves posMoves = fullMoveGenLoop(color, allCurrPositions);
//		calcCombinedMoves(posMoves);
//
//		OneColorCurrPositions colorCurrPositions = allCurrPositions.colorBitboards[color];
//		vector<MoveDesc> bestMoves;
//		double bestEval = color ? (-INFINITY) : (INFINITY);
//		for (int i = 0; i < 6; i++) {
//			PieceTypePosMoves& pieceTypePosMoves = posMoves.pieceTypes[i];
//
//			MoveDesc thisMove;
//			thisMove.pieceType = i;
//			thisMove.pieceMovingColor = color;
//
//			for (int j = 0; j < pieceTypePosMoves.posBB.size(); j++) {
//
//				thisMove.piece = j;
//
//				for (int moveOrCapture = 0; moveOrCapture < 2; moveOrCapture++) {
//
//					//THE FOLLOWING COMMENTS DON'T MATTER ANYMORE, as I now calculate the posmoves inside the function.
//					//CurrBitboard isn't a reference because we loop over it and remove bits until it equals 0. 
//					// We don't want these changes to reflect on the posMoves which we pass down in functions.
//					Bitboard currBitboard = (moveOrCapture==0) ? pieceTypePosMoves.posBB[j].moveBitboard : pieceTypePosMoves.posBB[j].capBitboard;
//					thisMove.moveOrCapture = (bool)moveOrCapture;
//
//					while (currBitboard!=0){
//						//cout << "HELLO" << endl;
//
//						int posOfNextBit = _tzcnt_u64(currBitboard);
//						thisMove.posOfMove = posOfNextBit;
//						setBitTo(&currBitboard, posOfNextBit, 0);
//
//						AllCurrPositions newPositionsAfterMove = allCurrPositions;
//						newPositionsAfterMove.applyMove(thisMove);
//						calcCombinedPos(newPositionsAfterMove);
//
//						EvalAndMovesTo res = minMax(newPositionsAfterMove, !color, depthCD);
//						if (color ? (res.eval > bestEval) : (res.eval < bestEval)) {
//							bestEval = res.eval;
//							vector<MoveDesc> resMovesTo = res.movesTo;
//							resMovesTo.push_back(thisMove);
//							bestMoves = resMovesTo;
//						}
//					}
//
//				}
//			}
//
//		}
//		EvalAndMovesTo res;
//		res.eval = bestEval;
//		res.movesTo = bestMoves;
//		return res;
//	} else {
//		EvalAndMovesTo res;
//		res.eval = simpleEval(allCurrPositions);
//		amountOfLeafNodes++;
//		return res;
//	}
//}
EvalAndBestMove minMax(AllCurrPositions allCurrPositions, bool color, int depthCD, ZobristHash currZobristHash) {
	if (transpositionTablePerft.find(currZobristHash) != transpositionTablePerft.end() && depthCD <= transpositionTablePerft[currZobristHash].depth) {
		return transpositionTablePerft[currZobristHash].leafNodes;
	}
	depthCD--;
	if (depthCD != -1) {
		double eval;
		AllPosMoves posMoves = fullMoveGenLoop(color, allCurrPositions, currZobristHash);

		OneColorCurrPositions colorCurrPositions = allCurrPositions.colorBitboards[color];
		double bestEval = color ? (-INFINITY) : (INFINITY);
		for (int i = 0; i < 6; i++) {
			PieceTypePosMoves& pieceTypePosMoves = posMoves.pieceTypes[i];

			MoveDesc thisMove;
			thisMove.pieceType = i;
			thisMove.pieceMovingColor = color;

			for (int j = 0; j < pieceTypePosMoves.posBB.size(); j++) {
				thisMove.piece = j;
				for (int moveOrCapture = 0; moveOrCapture < 2; moveOrCapture++) {

					//THE FOLLOWING COMMENTS DON'T MATTER ANYMORE, as I now calculate the posmoves inside the function.
					//CurrBitboard isn't a reference because we loop over it and remove bits until it equals 0. 
					// We don't want these changes to reflect on the posMoves which we pass down in functions.
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

						//If it's white, you want to take the best refutation, thus the lower val. Opposite for black
						eval = color ? min(minMax(newPositionsAfterMove, !color, depthCD, localZobristHash).eval, eval) 
							: max(minMax(newPositionsAfterMove, !color, depthCD, localZobristHash).eval, eval);
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
		double bestEval = color ? (-INFINITY) : (INFINITY);
		for (int i = 0; i < 6; i++) {
			PieceTypePosMoves& pieceTypePosMoves = posMoves.pieceTypes[i];

			MoveDesc thisMove;
			thisMove.pieceType = i;
			thisMove.pieceMovingColor = color;

			for (int j = 0; j < pieceTypePosMoves.posBB.size(); j++) {
				thisMove.piece = j;
				for (int moveOrCapture = 0; moveOrCapture < 2; moveOrCapture++) {

					//THE FOLLOWING COMMENTS DON'T MATTER ANYMORE, as I now calculate the posmoves inside the function.
					//CurrBitboard isn't a reference because we loop over it and remove bits until it equals 0. 
					// We don't want these changes to reflect on the posMoves which we pass down in functions.
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

		//REMOVE AFTER
		cout << "totalOfLeafsCaused: " << totalOfLeafsCaused << endl;
		//REMOVE AFTER
		if (totalOfLeafsCaused < 0) {
			cout << "Total leafs are negative, what is happening?" << endl;
			cout << "I hate breakpoints. " << endl;
		}
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

double simpleEval(AllCurrPositions allCurrPositions) {
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
	return total;
}
