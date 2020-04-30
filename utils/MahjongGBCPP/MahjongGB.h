#ifndef MAHJONG_H
#define MAHJONG_H

#include <string>
#include <utility>
#include <vector>

// CPP
#include "../MahjongGB/fan_calculator.cpp"
#include "../MahjongGB/shanten.cpp"
#include "MahjongGB.cpp"
// Dangerous

using namespace std;

void MahjongInit();

vector<pair<int, string>>
MahjongFanCalculator(vector<pair<string, pair<string, int>>> pack,
                     vector<string> hand, string winTile, int flowerCount,
                     bool isZIMO, bool isJUEZHANG, bool isGANG, bool isLAST,
                     int menFeng, int quanFeng);

#endif
