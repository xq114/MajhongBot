#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifdef _BOTZONE_ONLINE
#include "MahjongGB/MahjongGB.h"
#include "jsoncpp/json.h"
#else
#include "../utils/MahjongGBCPP/MahjongGB.cpp"
#include <json/json.h>
#endif

#define SIMPLEIO 0
//由玩家自己定义，0表示JSON交互，1表示简单交互。
#define fr first
#define sc second
#define mp make_pair

using namespace std;

vector<string> request, response;

int tile_str2num(string s) {
    //    把string表示的手牌转化为存储数字
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
    //    把int表示的手牌转化为str
    int a = n / 9, b = n % 9;
    char c[3];
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

struct info {
    int myPlayerID = 0; //即门风
    int quan = 0;
    int hua[4] = {0};   // 玩家花牌数
    int hand[34] = {0}; // hand存储的顺序: W1-9, B1-9, T1-9, F1-4, J1-3

    // 记录所有玩家碰杠吃的情况
    int peng[4][34] = {0};       // 记录的是喂牌的player id
    int gang[4][34] = {0};       // 不包括暗杠
    int angang[34] = {0};        //只记录了自己的暗杠
    int angang_ = 0;             //记录自己打算杠的牌
    vector<int> chi[4][34] = {}; //表示吃了第几张牌

    int play[4][34] = {0}; // 记录玩家打出过的牌

    int lastCard = 0;   // 上一局桌上的牌，如果是摸牌则为34
    int lastPlayer = 0; // 上一局桌上的牌是谁打的

    int leftTile = 144; //桌上还有多少牌

    int canAnGang() {
        // return一个可以杠的牌，没有的话return34
        for (int i = 0; i < 34; ++i) {
            if (hand[i] == 4)
                return i;
        }
        return 34;
    }

    bool canMingGang(int tile, int playerID) {
        // return是否可杠
        if (playerID==myPlayerID) return 0;
        if (hand[tile] == 3)
            return 1;
        return 0;
    }

    bool canPeng(int tile, int playerID) {
        // return是否可碰
        if (playerID==myPlayerID) return 0;
        if (hand[tile] == 2)
            return 1;
        return 0;
    }

    vector<int> canChi(int tile, int playerID) {
        // return吃的中间牌，不能吃return空vector
        vector<int> v;
        if (id2jia(playerID)!=1) return v;
        if (tile < 27) {
            int k = tile % 9;
            if (k != 7 && k != 8) {
                if (hand[tile + 1] && hand[tile + 2])
                    v.push_back(tile + 1);
            }
            if (k != 0 && k != 1) {
                if (hand[tile - 1] && hand[tile - 2])
                    v.push_back(tile - 1);
            }
            if (k != 0 && k != 8) {
                if (hand[tile + 1] && hand[tile - 1])
                    v.push_back(tile);
            }
        }
        return v;
    }

    int canHu(int winTile, bool isZIMO, bool isGANG) {
        // return 胡牌番数，不能胡return0
        vector<pair<int, string>> h =
            MahjongFanCalculator(pack(), hand_num2str(), tile_num2str(winTile),
                                 hua[myPlayerID], isZIMO, isJUEZHANG(winTile),
                                 isGANG, leftTile == 0, myPlayerID, quan);
        int totalFan = 0;
        for (auto i = h.begin(); i != h.end(); ++i) {
            totalFan += i->fr;
        }
        if (totalFan >= 8)
            return totalFan;
        else
            return 0;
    }

    vector<string> hand_num2str() {
        //把手牌转成计算番数需要的形式
        vector<string> hand_str;
        for (int i = 0; i < 34; ++i) {
            for (int j = 0; j < hand[i]; ++j)
                hand_str.push_back(tile_num2str(i));
        }
        return hand_str;
    }

    vector<pair<string, pair<string, int>>> pack() {
        //把吃碰杠的情况转化为算番需要的形式
        int itmp;
        vector<pair<string, pair<string, int>>> pack;
        for (int i = 0; i < 34; ++i) {
            if (!chi[myPlayerID][i].empty()) {
                for (auto j = chi[myPlayerID][i].begin();
                     j != chi[myPlayerID][i].end(); ++j)
                    pack.push_back(mp("CHI", mp(tile_num2str(i), *j)));
            }
            itmp = peng[myPlayerID][i];
            if (itmp)
                pack.push_back(mp("PENG", mp(tile_num2str(i), id2jia(itmp))));
            itmp = gang[myPlayerID][i];
            if (itmp)
                pack.push_back(mp("GANG", mp(tile_num2str(i), id2jia(itmp))));
        }
        return pack;
    }
    int id2jia(int id) {
        //判断是上下对家
        // 1为上家，2为对家，3为下家
        return (4 + myPlayerID - id) % 4;
    }

    int chiPos(int mid) {
        //计算吃了第几张牌
        return lastCard - mid + 2;
    }

    bool isJUEZHANG(int tile) {
        for (int i = 0; i < 4; ++i) {
            if (peng[i])
                return 1;
        }
        return 0;
    }

    void loadInfo(string data) {
        int tmp;
        istringstream sin(data);
        sin >> myPlayerID >> quan >> angang_ >> lastCard >> lastPlayer >>
            leftTile;
        for (int i = 0; i < 4; ++i)
            sin >> hua[i];
        for (int i = 0; i < 34; ++i)
            sin >> hand[i];
        for (int i = 0; i < 34; ++i)
            sin >> angang[i];
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 34; ++j) {
                sin >> peng[i][j];
            }
        }
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 34; ++j) {
                sin >> gang[i][j];
            }
        }
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 34; ++j) {
                sin >> play[i][j];
            }
        }
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 34; ++j) {
                sin >> tmp;
                while (tmp != 5) {
                    chi[i][j].push_back(tmp);
                    sin >> tmp;
                }
            }
        }
    }

    string saveInfo() {
        ostringstream sout;
        sout << myPlayerID << " " << quan << " " << angang_ << " " << lastCard
             << " " << lastPlayer << " " << leftTile << " ";
        for (int i = 0; i < 4; ++i)
            sout << hua[i] << " ";
        for (int i = 0; i < 34; ++i)
            sout << hand[i] << " ";
        for (int i = 0; i < 34; ++i)
            sout << angang[i] << " ";
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 34; ++j) {
                sout << peng[i][j] << " ";
            }
        }
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 34; ++j) {
                sout << gang[i][j] << " ";
            }
        }
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 34; ++j) {
                sout << play[i][j] << " ";
            }
        }
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 34; ++j) {
                for (auto k = chi[i][j].begin(); k < chi[i][j].end(); ++k)
                    sout << *k << " ";
                sout << 5 << " ";
            }
        }
        return sout.str();
    }
} info;

int main() {
    int turnID;
    string stmp;
    string req;  // current request
    string resp; // current response
#if SIMPLEIO
    cin >> turnID;
    turnID--;
    getline(cin, stmp);
    for (int i = 0; i < turnID; i++) {
        getline(cin, stmp);
        request.push_back(stmp);
        getline(cin, stmp);
        response.push_back(stmp);
    }
    getline(cin, stmp);
    request.push_back(stmp);
    req = stmp;
#else
    Json::Value inputJSON;
    cin >> inputJSON;
    turnID = inputJSON["responses"].size();
    req = inputJSON["requests"][turnID].asString();
    if (turnID > 0 && inputJSON["data"])
        info.loadInfo(inputJSON["data"].asString());
#endif
    int itmp;
    ostringstream sout;
    istringstream sin;
    MahjongInit();
    sin.str(req);
    sin >> itmp;
    if (itmp == 0) {
        sin >> info.myPlayerID >> info.quan;
        resp = "PASS";
    } else if (itmp == 1) {
        for (int j = 0; j < 4; j++) {
            sin >> info.hua[j];
            info.leftTile -= info.hua[j];
        }
        for (int j = 0; j < 13; j++) {
            sin >> stmp;
            ++info.hand[tile_str2num(stmp)];
        }
        info.leftTile -= 52;
        resp = "PASS";
    } else if (itmp == 2) {
        sin >> stmp;
        --info.leftTile;
        int tile = tile_str2num(stmp);
        ++info.hand[tile];
        //杠
//        if (info.canHu(tile, 1, 0))
//            resp = "HU";
//        else {
//
//            itmp = info.canAnGang();
//            if (itmp != 34) {
//                info.angang_ = itmp;
//                resp = "GANG " + stmp;
//            }
//            //补杠
//            else if (info.peng[info.myPlayerID][tile])
//                resp = "BUGANG " + stmp;
//            //出牌//出最大的一张qaaaq
//            else {
                for (int i = 33; i >= 0; --i) {
                    if (info.hand[i] != 0){
                        resp = "PLAY " + tile_num2str(i);
                        break;
                    }
                }
//            }
//        }
    } else {
        sin >> itmp >> stmp;
        int t;
        if (stmp == "BUHUA") {
            --info.leftTile;
            ++info.hua[itmp];
            info.lastCard = 34;
            resp = "PASS";
        } else if (stmp == "DRAW") {
            --info.leftTile;
            info.lastCard = 34;
            resp = "PASS";
        } else if (stmp == "PLAY") {
            sin >> stmp;
            t = tile_str2num(stmp);
            if (itmp == info.myPlayerID)
                --info.hand[t];
            ++info.play[itmp][t];
            info.lastCard = t;
            info.lastPlayer = itmp;
            // 是否要碰吃杠//胡还没写qaaq
            if (info.canMingGang(t,itmp))
                resp = "GANG";
            else if (info.canPeng(t,itmp)) {
                resp = "PENG ";
                for (int i = 33; i >= 0; --i) {
                    if (i != t && info.hand[i] != 0){
                        resp += tile_num2str(i);
                        break;
                    }
                }
            } else {
                vector<int> v = info.canChi(t,itmp);
                if (!v.empty()) {
                    resp = "CHI " + tile_num2str(v.front()) + " ";
                    for (int i = 33; i >= 0; --i) {
                        if (i != v.front() && i != v.front() + 1 &&
                            i != v.front() - 1 && info.hand[i] != 0){
                            resp += tile_num2str(i);
                            break;
                        }
                    }

                } else {
                    resp = "PASS";
                }
            }
        } else if (stmp == "PENG") {
            info.peng[itmp][info.lastCard] = info.lastPlayer;
            sin >> stmp;
            t = tile_str2num(stmp);
            if (itmp == info.myPlayerID) {
                info.hand[info.lastCard] -= 2;
                --info.hand[t];
            }
            ++info.play[itmp][t];
            info.lastCard = t;
            info.lastPlayer = itmp;
            // 是否要碰吃杠胡
            resp = "PASS";
        } else if (stmp == "CHI") {
            sin >> stmp;
            t = tile_str2num(stmp);
            int p = info.chiPos(t);
            info.chi[itmp][t].push_back(p);
            if (itmp == info.myPlayerID) {
                for (int i = 1; i <= 3; ++i) {
                    if (i != p)
                        --info.hand[t + i - 2];
                }
            }
            sin >> stmp;
            t = tile_str2num(stmp);
            ++info.play[itmp][t];
            if (itmp == info.myPlayerID)
                --info.hand[t];
            info.lastCard = t;
            info.lastPlayer = itmp;
            // 是否要碰吃杠胡
            resp = "PASS";
        } else if (stmp == "GANG") {
            if (info.lastCard != 34) { //明杠
                ++info.gang[itmp][info.lastCard];
                if (itmp == info.myPlayerID)
                    info.hand[info.lastCard] = 0;
            } else { //暗杠
                if (itmp == info.myPlayerID) {
                    info.hand[info.angang_] = 0;
                    info.angang[info.angang_] = 1;
                }
            }
            resp = "PASS";
        } else { // stmp=="BUGANG"
            sin >> stmp;
            t = tile_str2num(stmp);
            if (itmp == info.myPlayerID)
                --info.hand[t];
            info.gang[itmp][t] = info.peng[itmp][t];
            info.peng[itmp][t] = 0;
            //抢杠胡
            resp = "PASS";
        }
    }

#if SIMPLEIO
    cout << resp << endl;
#else
    Json::Value outputJSON;
    outputJSON["data"] = info.saveInfo();
    outputJSON["debug"] = outputJSON["data"];
    outputJSON["response"] = resp;
    Json::StreamWriterBuilder builder;
    // 调整json为单行输出模式
    builder["indentation"] = ""; // 注释掉本行以得到复杂输出
    Json::StreamWriter *writer{builder.newStreamWriter()};
    writer->write(outputJSON, &cout);
    cout << endl;
#endif
    return 0;
}
