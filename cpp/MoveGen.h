#pragma once
#ifndef MoveGen_H
#define MoveGen_H

#include "iostream"
#include "string"
#include <vector>
#include <array>
#include "Eigen/Core"
#include <cctype>

using namespace std;

using Bitboard = Eigen::Matrix<bool, 8, 8>;
using MoveMag = array<int, 3>;

int* isChecked(char** board, char color);
array<Bitboard, 2> genBitboard(char piece, int x, int y, Bitboard oppColorBB);
Bitboard genEmptyBitboard();
//Bitboard genAllBitBoards(char** board, char color,bool attacking);
bool checkBounds(int x, int y);
vector<Bitboard> arrayToVector(array<Bitboard, 2> arr);
vector<Bitboard> main2();

#endif