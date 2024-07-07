#include "Chess.h"
#include "MoveGen.h"

using namespace std;


char** fenToMatrix(std::string fen);
void delete2DArray(char** arr, int rows);
string convertToString(char** arr, int cols, int rows);
string convertToJSArr(char** arr, int cols, int rows);
string convertVofBBJS(vector<Bitboard> matrixVector);
//BUG, remove if possible
vector<int> pieceCoordinates(char piece, char** board);



int main() {
    httplib::Server svr;

    string lFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";

    // Handle GET requests
    svr.Get("/data", [lFen](const httplib::Request& /*req*/, httplib::Response& res) {
        char** arr = fenToMatrix(lFen);
        res.set_content(convertToString(arr, 8, 8), "text/plain");
        res.set_header("Access-Control-Allow-Origin", "*");
        delete2DArray(arr, 8);
    });
    svr.Get("/fenArr", [lFen](const httplib::Request& /*req*/, httplib::Response& res) {
        char** arr = fenToMatrix(lFen);
        res.set_content(convertToJSArr(arr, 8, 8), "text/plain");
        res.set_header("Access-Control-Allow-Origin", "*");
        delete2DArray(arr, 8);
    });
    
    svr.Get("/getBitboards", [lFen](const httplib::Request& /*req*/, httplib::Response& res) {
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
        res.set_content(convertVofBBJS(main2()), "text/plain");
        res.set_header("Access-Control-Allow-Origin", "*");
    });

    // Run the server
    svr.listen("localhost", 8080);

    return 0;
}

char** fenToMatrix(std::string fen) {
    char** arr = new char * [8];
    for (int i = 0; i < 8; ++i) {
        arr[i] = new char[8];
    }
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            arr[i][j] = ' ';
        }
    }
    //Does this:
    /*char arr[8][8] = {
        {'','','','','','','',''},
        {'','','','','','','',''},
        {'','','','','','','',''},
        {'','','','','','','',''},
        {'','','','','','','',''},
        {'','','','','','','',''},
        {'','','','','','','',''},
        {'','','','','','','',''}
    };*/
    
    
    int actualPos = -1;
    for (int i = 0; i < fen.length(); i++) {
        if (fen[i] == '/') {
            continue;
        }
        actualPos++;
        //std::cout << "actualPos:" << actualPos << std::endl;
        if (isdigit(fen[i])) {
            int currNum = fen[i] - '0';
            actualPos += currNum-1;
            continue;
        }else{
            //Else is useless, looks better though. Shush.
            int y = actualPos / 8;
            //Keeps as int therefore rounds down. 
            int x = actualPos % 8;
            //It's y,x... 
            arr[y][x] = fen[i];
        }
    }
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            std::cout << arr[i][j] << " ";
        }
        std::cout << std::endl;
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

//string convertVofBBJS(vector<Bitboard> matrixVector) {
//    //cout << matrixVector[0] << endl;
//    std::stringstream ss;
//    ss << "[";
//    for (size_t i = 0; i < matrixVector.size(); ++i) {
//        ss << "[";
//        for (int row = 0; row < matrixVector[i].rows(); ++row) {
//            ss << "[";
//            for (int col = 0; col < matrixVector[i].cols(); ++col) {
//                //cout << "matrixVector[i](row, col):" << matrixVector[i](row, col) << ", which is"<< (matrixVector[i](row, col) ? "true" : "false") << endl;
//                ss << (matrixVector[i](row, col) ? "true" : "false");
//                if (col < matrixVector[i].cols() - 1) {
//                    ss << ",";
//                }
//            }
//            ss << "]";
//            if (row < matrixVector[i].rows() - 1) {
//                ss << ",";
//            }
//        }
//        ss << "]";
//        if (i < matrixVector.size() - 1) {
//            ss << ",";
//        }
//    }
//    ss << "]";
//    string jsonString = ss.str();
//    return jsonString;
//}