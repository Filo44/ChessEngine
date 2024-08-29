#pragma once
#include "MoveGen.h"
#include "Chess.h"
#include "Classes.h"
#include "ZobristSeed.h"

using namespace std;

extern int amountOfLeafNodes;
extern int captures;
extern int enPassant;
extern int totPos;
extern unordered_map<ZobristHash, LeafNodesAndCurrPos> transpositionTablePerft;
extern unordered_map<ZobristHash, EvalAndBestMove> transpositionTable;
extern vector<string> movesAndLeafNodes;



EvalAndBestMove minMax(AllCurrPositions allCurrPositions, bool color, int depthCD, ZobristHash currZobristHash, double cutOfTime);
ZobristHash applyMovesTo(AllCurrPositions& allCurrPositions, vector<MoveDesc> movesTo, ZobristHash currZobristHash);
int perft(AllCurrPositions allCurrPositions, bool color, int depthCD, ZobristHash currZobristHash, int initDepth);
double simpleEval(AllCurrPositions allCurrPositions, bool colorToMove, ZobristHash currZobristHash);