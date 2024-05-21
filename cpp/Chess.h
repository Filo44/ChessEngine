#pragma once
#ifndef Chess_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define Chess_H

#include <iostream>
#include "httplib.h"
#include <string>
#include "Eigen/Core"

using Bitboard = Eigen::Matrix<bool, 8, 8>;
using namespace std;

char** fenToMatrix(std::string fen);
void delete2DArray(char** arr, int rows);
string convertToString(char** arr, int cols, int rows);
string convertToJSArr(char** arr, int cols, int rows);
string convertVofBBJS(vector<Bitboard> matrixVector);

#endif