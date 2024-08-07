#include "Chess.h"
#include "MoveGen.h"
#include "Classes.h"
#include "SearchAlgorithm.h"
#include "ZobristSeed.h"

using namespace std;

int main(int argc, char* argv[]) {
    int depth = 5;
    string lFen = "8/8/8/2k5/2pP4/8/B7/4K3 b - d3";
    //string lFen = "6k1/8/8/8/2pP4/8/8/1B3K2 b - -";
    if (argc > 1) {
        depth = stoi(argv[1]);
        if (argc > 2) {
            lFen = argv[2];
        }
    }

    cout << "Started" << endl;
    httplib::Server svr;

    PosAndColor gameState = fenToPosBitboards(lFen);
    AllCurrPositions allPositionBitboards = gameState.allCurrPositions;
    bool color = gameState.color;

    //cout << "allPositionBitboards.colorBitboards[0].canCastleKSide: " << allPositionBitboards.colorBitboards[0].canCastleKSide << endl;
    //cout << "allPositionBitboards.colorBitboards[1].canCastleKSide: " << allPositionBitboards.colorBitboards[1].canCastleKSide << endl;
    //cout << "Pawn who double moved: " << allPositionBitboards.pawnWhoDoubleMoved << endl;
    //cout << convertToString(allPositionBitboardsToMatrix(allPositionBitboards), 8, 8) << endl;

    ZobristHash currZobristHash = genInitZobristHash(allPositionBitboards);
    cout << "Calculated the zobrist hash" << endl;
    //cout << "-----------------" << endl;
    AllPosMoves posMoves = fullMoveGenLoop(color, allPositionBitboards, currZobristHash);

    
    /*MoveDesc move;
    move.pieceMovingColor = 1;
    move.moveOrCapture = 0;
    move.piece = 0;
    move.pieceType = pieceToNumber['p'];
    move.posOfMove = 35; 
    currZobristHash = allPositionBitboards.applyMove(move, currZobristHash);

    posMoves = fullMoveGenLoop(!color, allPositionBitboards, currZobristHash);*/

    amountOfLeafNodes = 0;
    captures = 0;
    enPassant = 0;
    totPos = 0;
    //amOfEnPassantXORAdds = 0;
    //amOfEnPassantXORRemovals = 0;
    hypos = 0;
    transpositionTablePerft = {};
    uint64_t actualAmountOfLeafNodes = perft(allPositionBitboards, color, depth, currZobristHash);
    cout << "actualAmountOfLeafNodes: " << actualAmountOfLeafNodes << endl;
    cout << "enPassant: " << enPassant << endl;
    cout << "captures: " << captures << endl;
    cout << "totPos: " << totPos << endl;
    //cout << "amOfEnPassantXORAdds : " << amOfEnPassantXORAdds << endl;
    //cout << "amOfEnPassantXORRemovals: " << amOfEnPassantXORRemovals<< endl;
    //cout << "standingEnPassantXORs: " << amOfEnPassantXORAdds - amOfEnPassantXORRemovals<< endl;
    cout << "hypos: " << hypos << endl;
    return actualAmountOfLeafNodes;

    /*cout << "Evaluation: " << searchRes.eval << endl;

    cout << "posOfMove: " << searchRes.movesTo[searchRes.movesTo.size()-1].posOfMove << endl;
    cout << "pieceType: " << pieces[searchRes.movesTo[searchRes.movesTo.size() - 1].pieceType] << endl;

    allPositionBitboards.applyMove(searchRes.movesTo[searchRes.movesTo.size() - 1]);*/

    //MoveDesc move;
    //move.pieceMovingColor = 1;
    //move.moveOrCapture = 0;
    //move.piece = 0;
    //move.pieceType = pieceToNumber['p'];
    //move.posOfMove = 35;
    //allPositionBitboards.applyMove(move);

    //posMoves = fullMoveGenLoop(0, allPositionBitboards);

    //cout << "---------------" << endl;
    //MoveDesc move1;
    //move1.pieceMovingColor = 0;
    //move1.moveOrCapture = 0;
    //move1.piece = 0;
    //move1.pieceType = pieceToNumber['p'];
    //move1.posOfMove = _tzcnt_u64(posMoves.pieceTypes[move.pieceType].posBB[move.piece].moveBitboard);
    //allPositionBitboards.applyMove(move1);
    //cout << "---------------" << endl;

    //posMoves = fullMoveGenLoop(1, allPositionBitboards);

    // Handle GET requests
    svr.Get("/data", [&allPositionBitboards, &posMoves](const httplib::Request& /*req*/, httplib::Response& res) {
        char** arr = allPositionBitboardsToMatrix(allPositionBitboards);
        res.set_content(convertToJSArr(arr, 8, 8), "text/plain");
        res.set_header("Access-Control-Allow-Origin", "*");
        delete2DArray(arr, 8);
    });
    //svr.Get("/eval", [searchRes](const httplib::Request& /*req*/, httplib::Response& res) {
    //    string eval = to_string(searchRes.eval);
    //    res.set_content(eval, "text/plain");
    //    res.set_header("Access-Control-Allow-Origin", "*");
    //});
    //Dummy function
    svr.Get("/eval", [](const httplib::Request& /*req*/, httplib::Response& res) {
        res.set_content("1", "text/plain");
        res.set_header("Access-Control-Allow-Origin", "*");
     });

    svr.Get("/getBitboards", [posMoves](const httplib::Request& /*req*/, httplib::Response& res) {
        res.set_content(allPosMovesToMatrix(posMoves), "text/plain");
        res.set_header("Access-Control-Allow-Origin", "*");
    });

    // Run the server
    svr.listen("localhost", 8080);
 

    return 0;
}

PosAndColor fenToPosBitboards(std::string fen) {
    PosAndColor res = PosAndColor();
    OneColorCurrPositions blackBitboard;
    OneColorCurrPositions whiteBitboard;
    AllCurrPositions allPositionBitboards;
    allPositionBitboards.colorBitboards[0] = blackBitboard;
    allPositionBitboards.colorBitboards[1] = whiteBitboard;
    //Make this just 12 lines, more optimized.
    for (int color = 0; color < 2; color++) {
        for (int i = 0; i < 6; i++) {
            PieceTypeCurrPositions newPiece;
            newPiece.pieceType = pieces[i];
            allPositionBitboards.colorBitboards[color].pieceTypes[i] = newPiece; 
        }
    }

    int maxI = -1;
    int actualPos = -1;
    for (int i = 0; i < fen.length(); i++) {
        if (fen[i] == '/') {
            continue;
        } else if (fen[i] == ' ') {
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
        } else {
            //Else is useless, looks better though. Shush.
            int y = actualPos / 8;
            //Keeps as int therefore rounds down. 
            int x = actualPos % 8;
            //cout << pieceToNumber['r'];
            //Goes to the correct colour
            //Goes to the piece type by checking hte piece to number map
            //then pushes the position BB by making a bit board with one 1 and pushing it to the actualPos
            allPositionBitboards.colorBitboards[isupper(fen[i])]
                .pieceTypes[pieceToNumber[tolower(fen[i])]]
                .posBB.push_back(1ULL << actualPos);
        }
    }

    allPositionBitboards.colorBitboards[0].canCastleKSide = false;
    allPositionBitboards.colorBitboards[0].canCastleQSide = false;
    allPositionBitboards.colorBitboards[1].canCastleKSide = false;
    allPositionBitboards.colorBitboards[1].canCastleQSide = false;
    //cout << "maxI + 1:" << maxI + 1 << endl;
    //cout << "fen[maxI + 1]:" << fen[maxI + 1] << endl;
    //If no space detected, i.e. maxI=-1, it goes to the default values
    if (maxI != -1) {
        if (fen[maxI + 1] == 'b') {
            //cout << "Black" << endl;
            res.color = 0;
        } else {
            //cout << "White" << endl;
            res.color = 1;
        }
        int maxJ = -1;
        if (fen[maxI + 3] == '-') {
            //cout << "No castling rights" << endl;
            maxJ = 4;
        } else {
            for (int j = 3; j < 7; j++) {
                char el = fen[maxI + j];
                if (el == ' ') {
                    maxJ = j;
                    break;
                }
                bool castleRightsColor = isupper(el);
                if (tolower(el) == 'k') {
                    //cout << (castleRightsColor ? "White" : "Black") << " can castle king side. " << endl;
                    allPositionBitboards.colorBitboards[castleRightsColor].canCastleKSide = true;
                } else {
                    //cout << (castleRightsColor ? "White" : "Black") << " can castle queen side. " << endl;
                    allPositionBitboards.colorBitboards[castleRightsColor].canCastleQSide = true;
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
            allPositionBitboards.pawnWhoDoubleMoved = allPositionBitboards.searchPieceByPosAndType(x + (y * 8), whitePawn, !res.color);
        }
    } else {
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
    //No clue why each piece stores an entire bitboard when there can only be one bit which is on. Optimize this later.
    for (int color = 0; color < 2; ++color) {
        for (int pieceI = 0; pieceI < 6; ++pieceI) {
            for (const Bitboard bitboard : allPositionBitboardsL.colorBitboards[color].pieceTypes[pieceI].posBB) {
                //Finds the last, and in this case only, 1
                int pos = _tzcnt_u64(bitboard);
                
                //y, x for arrays
                arr[pos / 8][pos % 8] = color? toupper(pieces[pieceI]) : pieces[pieceI] ;
            }
        }
    }
    return arr;
}
string allPosMovesToMatrix(AllPosMoves posMoves) {
    string s="{ \"pieceTypes\": [";
    for (int i = 0; i < 6; i++) {
        PieceTypePosMoves pieceType = posMoves.pieceTypes[i];
        s += "{\"pieceChar\": ";
        string pieceTypeString{ pieceType.pieceType };
        s += "\""+pieceTypeString+"\"";
        s += ", \"combinedCapBB\":";
        s += convertBBJS(pieceType.pieceTypeCombinedCapBB).str();
        s += ", \"combinedMoveBB\":";
        s += convertBBJS(pieceType.pieceTypeCombinedMoveBB).str();
        s += ", \"pieces\":[  ";
        for (int j = 0; j < pieceType.posBB.size(); j++) {
            SinglePiecePosMoves piece = pieceType.posBB[j];
            s += "{\"capBitboard\": ";
            s += convertBBJS(piece.capBitboard).str();
            s += ", \"moveBitboard\": ";
            s += convertBBJS(piece.moveBitboard).str();
            s += "}";
            if (j != (pieceType.posBB.size() - 1)) {
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
ZobristHash genInitZobristHash(AllCurrPositions currPositions) {
    //When the first XOR is done, since it is instantiated to zero, the currZobristHash will become that number.
    ZobristHash currZobristHash = 0;
    uint8_t castlingKey = 0b00000000;
    int enPassantFile = -1;

    for (int color = 0; color < 2; color++) {
        for (int pieceType = 0; pieceType < 6; pieceType++) {
            int amOfPieces = currPositions.colorBitboards[color].pieceTypes[pieceType].posBB.size();
            for (int piece = 0; piece < amOfPieces; piece++) {
                int pos = _tzcnt_u64(currPositions.colorBitboards[color].pieceTypes[pieceType].posBB[piece]);
                currZobristHash ^= ZobristSeed[color ? pieceType + 6 : pieceType][pos];
            }
        }
        //If it is white it multiplies everything by 4, or shifts it up twice
        int colorMult = color ? 4 : 0;
        if (currPositions.colorBitboards[color].canCastleKSide) {
            castlingKey |= (0b00000001 * colorMult);
        }
        if (currPositions.colorBitboards[color].canCastleQSide) {
            castlingKey |= (0b00000010 * colorMult);
        }

        // Maybe don't store the piece but rather its position in terms of its file (Optimize)?
        if (currPositions.pawnWhoDoubleMoved != -1) {
            //White pawn because I have yet to change it to just store all the pieces in one array instead of per color
            //and white since now it means just pawn
            int pawnWhoDoubleMovedI = currPositions.pawnWhoDoubleMoved;
            int pawnWhoDoubleMovedRef = currPositions.colorBitboards[color].pieceTypes[whitePawn].posBB[pawnWhoDoubleMovedI];
            int enPassantVictimFile = _tzcnt_u64(pawnWhoDoubleMovedRef) % 8;
            //Since it(.pawnWhoDoubleMoved) is the position of the piece who can get taken, that works just as well as a square which can get en passanted into.
            currZobristHash ^= EnPassantFileSeed[enPassantVictimFile];
        }
    }

    currZobristHash ^= CastlingSeed[castlingKey];

    return currZobristHash; 
}

void delete2DArray(char** arr, int rows) {
    for (int i = 0; i < rows; ++i) {
        delete[] arr[i];
    }
    delete[] arr;
}
string convertToString(char** a, int cols, int rows)
{
    string s = "";
    for (int i = 0; i < cols; i++) {
        s = s + "\n";
        for (int j = 0; j < rows; j++) {
            s = s + a[i][j];
        }
    }
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
        }else {
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
        } else {
            ss << "]";
            if (bit / 8 != 7) {
                ss << ",";
            }
        }
    }
    ss << "]";
    return ss;
}

