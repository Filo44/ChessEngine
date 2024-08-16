#include "Chess.h"
#include "MoveGen.h"
#include "Classes.h"
#include "SearchAlgorithm.h"
#include "ZobristSeed.h"

using namespace std;

int main(int argc, char* argv[]) {
    int depth = 5;
    int port = 8080;
    bool color = false;
    //string lFen = "8/8/8/2k5/2pP4/8/B7/4K3 b - d3";
    //string lFen = "8/3k3r/8/8/6N1/8/8/2K5 b - -";
    string lFen = "r1bqkbnr/ppp1pppp/n2N4/8/8/8/PPPPPPPP/R1BQKBNR b KQkq -";
    if (argc > 1) {
        depth = stoi(argv[1]);
        if (argc > 2) {
            lFen = argv[2];
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
    //bool color = gameState.color;

    ZobristHash currZobristHash = genInitZobristHash(allPositionBitboards);
    cout << "Calculated the zobrist hash" << endl;
    AllPosMoves posMoves = fullMoveGenLoop(color, allPositionBitboards, currZobristHash);

    //MoveDesc move;
    //move.pieceMovingColor = 1;
    //move.pieceType = 1;
    //move.posOfMove = 21;
    //move.moveOrCapture = 0;
    //move.piece = 0;

    
    /*MoveDesc move;
    move.pieceMovingColor = 1;
    move.moveOrCapture = 0;
    move.piece = 0;
    move.pieceType = pieceToNumber['p'];
    move.posOfMove = 35; 
    currZobristHash = allPositionBitboards.applyMove(move, currZobristHash);

    posMoves = fullMoveGenLoop(!color, allPositionBitboards, currZobristHash);*/
    //amountOfLeafNodes = 0;
    //captures = 0;
    //enPassant = 0;
    //totPos = 0;
    ////amOfEnPassantXORAdds = 0;
    ////amOfEnPassantXORRemovals = 0;
    //hypos = 0;
    //transpositionTablePerft = {};
    //uint64_t actualAmountOfLeafNodes = perft(allPositionBitboards, color, depth, currZobristHash);
    //cout << "actualAmountOfLeafNodes: " << actualAmountOfLeafNodes << endl;
    //cout << "enPassant: " << enPassant << endl;
    //cout << "captures: " << captures << endl;
    //cout << "totPos: " << totPos << endl;
    ////cout << "amOfEnPassantXORAdds : " << amOfEnPassantXORAdds << endl;
    ////cout << "amOfEnPassantXORRemovals: " << amOfEnPassantXORRemovals<< endl;
    ////cout << "standingEnPassantXORs: " << amOfEnPassantXORAdds - amOfEnPassantXORRemovals<< endl;
    //cout << "hypos: " << hypos << endl;
    //return actualAmountOfLeafNodes;
    
    transpositionTable = {};
    /*EvalAndBestMove res = minMax(allPositionBitboards, color, depth, currZobristHash);
    cout << "Eval " << res.eval << endl;
    cout << "Hi" << endl;*/


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
        res.set_content("0", "text/plain");
        res.set_header("Access-Control-Allow-Origin", "*");
     });

    svr.Get("/getBitboards", [posMoves](const httplib::Request& /*req*/, httplib::Response& res) {
        res.set_content(allPosMovesToMatrix(posMoves), "text/plain");
        res.set_header("Access-Control-Allow-Origin", "*");
    });
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
    if (moveStr.contains("xFrom")) {
        move.piece = allCurrPositions.searchPieceByPosAndType((int)moveStr["xFrom"] + ((int)moveStr["yFrom"] * 8), moveStr["pieceType"], moveStr["pieceMovingColor"]);
    } else {
        move.piece = moveStr["piece"];
    }
    cout << "move.piece: " << move.piece << endl;
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
    res += ", \"piece\":";
    res += to_string(move.piece);
    res += "}";
    return res;
}

double timeManagementFunction(double timeRemaining) {
    return 10.00;
}

EvalAndBestMove iterativeSearch(AllCurrPositions allCurrPositions, bool color, ZobristHash currZobristHash, double timeAvailable) {
    double cutOffTime = (double)time(nullptr) + timeAvailable;

    EvalAndBestMove res;
    int depth = 2;
    while (time(nullptr)<cutOffTime){
        cout << "Depth: " << depth << endl;
        
        transpositionTable = {};
        EvalAndBestMove searchResults = minMax(allCurrPositions, color, depth, currZobristHash, cutOffTime);

        //Currently ignores aborted searches
        if (!searchResults.abortedDueToTime && !searchResults.bestMove.nullMove) {
            res.eval = searchResults.eval;
            res.bestMove = searchResults.bestMove;

        }

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
        + ", \"canWhiteCastleKSide\":" + (allPositionBitboards.colorBitboards[1].canCastleKSide ? "true" : "false")
        + ", \"canWhiteCastleQSide\":" + (allPositionBitboards.colorBitboards[1].canCastleQSide ? "true" : "false")
        + ", \"canBlackCastleKSide\":" + (allPositionBitboards.colorBitboards[0].canCastleKSide ? "true" : "false")
        + ", \"canBlackCastleQSide\":" + (allPositionBitboards.colorBitboards[0].canCastleQSide ? "true" : "false")
        + "}";
    cout << "Responding: " << res;
    return res;
}