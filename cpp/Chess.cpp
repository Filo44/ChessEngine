#include "Chess.h"
#include "MoveGen.h"
#include "Classes.h"

using namespace std;



int main() {
    httplib::Server svr;
    string lFen = "6k1/5p2/6p1/8/7p/8/6NP/6K1";
    AllCurrPositions allPositionBitboards = fenToPosBitboards(lFen);

    AllPosMoves posMoves = fullMoveGenLoop(1, allPositionBitboards);

    // Handle GET requests
    svr.Get("/data", [allPositionBitboards](const httplib::Request& /*req*/, httplib::Response& res) {
        char** arr = allPositionBitboardsToMatrix(allPositionBitboards);
        res.set_content(convertToJSArr(arr, 8, 8), "text/plain");
        res.set_header("Access-Control-Allow-Origin", "*");
        delete2DArray(arr, 8);
    });
    //svr.Get("/fenArr", [lFen](const httplib::Request& /*req*/, httplib::Response& res) {
    //    char** arr = fenToMatrix(lFen);
    //    res.set_content(convertToString(arr, 8, 8), "text/plain");
    //    res.set_header("Access-Control-Allow-Origin", "*");
    //    delete2DArray(arr, 8);
    //});
    
    svr.Get("/getBitboards", [posMoves](const httplib::Request& /*req*/, httplib::Response& res) {
        //std::vector<Bitboard> DUMMYbitboards;
        ////DUMMY BITBOARDS TO TEST FRONT END
        ////REPLACE
        //Bitboard randomBB;
        //randomBB = 0;
        //randomBB(0,0) = true;
        //Bitboard randomBB2;
        //randomBB2 = 0;
        //randomBB2(1, 0) = true;
        ////randomBB(1, 0) = true;
        ////randomBB(2, 0) = true;
        ////randomBB(3, 0) = true;
        //DUMMYbitboards.push_back(randomBB);
        //DUMMYbitboards.push_back(randomBB2);
        res.set_content(convertVofBBJS(posMoves.pieceTypes[pieceToNumber['n']].fetchBitboards(false)), "text/plain");
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
            cout << pieceToNumber['r'];
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
    for (size_t i = 0; i < matrixVector.size(); ++i) {
        ss << "[";
        for (int bit = 0; bit < 64; ++bit) {
            if (bit % 8 == 0) {
                ss << "[";
            }
            ss << (getBit(matrixVector[i], bit%8, bit/8) ? "true" : "false");
            if (bit%8!=7) {
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
        if (i < matrixVector.size() - 1) {
            ss << ",";
        }
    }
    ss << "]";
    string jsonString = ss.str();
    return jsonString;
}

