#include "Chess.h"
#include "MoveGen.h"
#include "Classes.h"
#include "SearchAlgorithm.h"

using namespace std;



int main() {
    cout << "Started" << endl;
    httplib::Server svr;
    string lFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
    //string lFen = "3k4/8/8/8/4p3/8/3P4/3K4";
    AllCurrPositions allPositionBitboards = fenToPosBitboards(lFen);

    //allPositionBitboards.colorBitboards[0].canCastleKSide = false;
    //allPositionBitboards.colorBitboards[0].canCastleQSide = false;
    //allPositionBitboards.colorBitboards[1].canCastleKSide = false;
    //allPositionBitboards.colorBitboards[1].canCastleQSide = false;

    AllPosMoves posMoves = fullMoveGenLoop(1, allPositionBitboards);

    amountOfLeafNodes = 0;
    captures = 0;
    enPassant = 0;
    perft(allPositionBitboards, 1, 5);
    cout << "amountOfLeafNodes: " << amountOfLeafNodes << endl;
    cout << "enPassant: " << enPassant << endl;
    cout << "captures: " << captures << endl;
    return 1;

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
        //cout << allPosMovesToMatrix(posMoves) << endl;
        //searchRes.set_content(convertVofBBJS(posMoves.pieceTypes[pieceToNumber['n']].fetchBitboards(false)), "text/plain");
        res.set_content(allPosMovesToMatrix(posMoves), "text/plain");
        res.set_header("Access-Control-Allow-Origin", "*");
    });

    // Run the server
    svr.listen("localhost", 8080);
 

    return 0;
}

AllCurrPositions fenToPosBitboards(std::string fen) {
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

    int actualPos = -1;
    for (int i = 0; i < fen.length(); i++) {
        if (fen[i] == '/') {
            continue;
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

    return allPositionBitboards;
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
            //Add const before the Bitboard bitboard, optimize
            for (Bitboard bitboard : allPositionBitboardsL.colorBitboards[color].pieceTypes[pieceI].posBB) {
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
        ss << (getBit(curBB, bit % 8, bit / 8) ? "true" : "false");
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
