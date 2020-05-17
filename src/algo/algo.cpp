#include <torch/script.h>
#include <torch/custom_class.h>
#include <string>

#include "../../utils/MahjongGB/fan_calculator.h"

using namespace std;

struct MahjongAlgo {

int tile_str2num(string s) {
    // 把string表示的手牌转化为存储数字
    if (s[0] == 'W')
        return s[1] - '1';
    if (s[0] == 'B')
        return 9 + s[1] - '1';
    if (s[0] == 'T')
        return 18 + s[1] - '1';
    if (s[0] == 'F')
        return 27 + s[1] - '1';
    if (s[0] == 'J')
        return 31 + s[1] - '1';
    return 34;
}

string tile_num2str(int n) {
    // 把int表示的手牌转化为str
    int a = n / 9, b = n % 9;
    char c[3];
    c[2] = '\0';
    c[1] = b + '1';
    if (a == 0)
        c[0] = 'W';
    else if (a == 1)
        c[0] = 'B';
    else if (a == 2)
        c[0] = 'T';
    else if (b < 4)
        c[0] = 'F';
    else {
        c[0] = 'J';
        c[1] = b - 4 + '1';
    }
    return c;
}

mahjong::tile_t str2tile_t(string s) {
    mahjong::suit_t su;
    switch (s[0]) {
    case 'W':
        su = TILE_SUIT_CHARACTERS;
        break;
    case 'B':
        su = TILE_SUIT_BAMBOO;
        break;
    case 'T':
        su = TILE_SUIT_DOTS;
        break;
    case 'F':
    case 'J':
        su = TILE_SUIT_HONORS;
        break;
    };
    mahjong::rank_t ra = s[1] - '1';
    mahjong::tile_t t = mahjong::make_tile(su, ra);
}

int tile_t2num(mahjong::tile_t t) {
    mahjong::suit_t su = mahjong::tile_get_suit(t);
    mahjong::rank_t ra = mahjong::tile_get_rank(t);
    return su * 9 + ra - 10;
}

mahjong::tile_t num2tile_t(int n) {
    int a = n / 9 + 1, b = n % 9 + 1;
    mahjong::suit_t su = a;
    mahjong::rank_t ra = b;
    return mahjong::make_tile(su, ra);
}

string tile_t2str(mahjong::tile_t t) {
    mahjong::suit_t su = mahjong::tile_get_suit(t);
    mahjong::rank_t ra = mahjong::tile_get_rank(t);
    int a = su - 1, b = ra - 1;
    char c[3];
    c[2] = '\0';
    c[1] = '1' + b;
    if (a == 0)
        c[0] = 'W';
    else if (a == 1)
        c[0] = 'B';
    else if (a == 2)
        c[0] = 'T';
    else if (b < 4)
        c[0] = 'F';
    else {
        c[0] = 'J';
        c[1] = b - 4 + '1';
    }
    return c;
}

};

static auto algo = torch::class_<MahjongAlgo>("mahjong", "Algo")
    .def(torch::init())
    .def("tile_str2num", &MahjongAlgo::tile_str2num)
    .def("tile_num2str", &MahjongAlgo::tile_num2str)
    .def("tile_t2num", &MahjongAlgo::tile_t2num)
    .def("tile_t2str", &MahjongAlgo::tile_t2str)
    .def("num2tile_t", &MahjongAlgo::num2tile_t)
    .def("str2tile_t", &MahjongAlgo::str2tile_t);