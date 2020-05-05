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

    bool bupai = 0; //记录这局的自摸是不是杠后补牌

    /*addxxx都是用来记录req的信息的*/
    void addZIMO(int tile) {
        --leftTile;
        ++hand[tile];
    }

    void addBUHUA(int itmp) {
        --leftTile;
        ++hua[itmp];
        lastCard = 34;
    }

    void addDRAW() {
        --leftTile;
        lastCard = 34;
    }

    void addPLAY(int itmp, int tile) {
        if (itmp == myPlayerID)
            --hand[tile];
        ++play[itmp][tile];
        lastCard = tile;
        lastPlayer = itmp;
    }

    void addPENG(int itmp, int tile) {
        peng[itmp][lastCard] = lastPlayer;
        if (itmp == myPlayerID) {
            hand[lastCard] -= 2;
            --hand[tile];
        }
        ++play[itmp][tile];
        lastCard = tile;
        lastPlayer = itmp;
    }

    void addCHI(int itmp, int pos, int tile) {
        chi[itmp][tile].push_back(pos);
        if (itmp == myPlayerID) {
            --hand[tile];
            for (int i = 1; i <= 3; ++i) {
                if (i != pos)
                    --hand[tile + i - 2];
            }
        }
        ++play[itmp][tile];
        lastCard = tile;
        lastPlayer = itmp;
    }

    void addGANG(int itmp) {
        if (lastCard != 34) { //明杠
            ++gang[itmp][lastCard];
            if (itmp == myPlayerID) {
                hand[lastCard] = 0;
                bupai = 1;
            }
        } else { //暗杠
            //这里可能还需要补记录其他人的暗杠情况
            if (itmp == myPlayerID) {
                hand[angang_] = 0;
                angang[angang_] = 1;
                bupai = 1;
            }
        }
    }

    void addBUGANG(int itmp, int tile) {
        if (itmp == myPlayerID)
            --hand[tile];
        gang[itmp][tile] = peng[itmp][tile];
        peng[itmp][tile] = 0;
    }

    /*判断是否可以碰吃杠胡的函数*/
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
        if (playerID == myPlayerID)
            return 0;
        if (hand[tile] == 3)
            return 1;
        return 0;
    }

    bool canPeng(int tile, int playerID) {
        // return是否可碰
        if (playerID == myPlayerID)
            return 0;
        if (hand[tile] == 2)
            return 1;
        return 0;
    }

    vector<int> canChi(int tile, int playerID) {
        // return吃的中间牌，不能吃return空vector
        vector<int> v;
        if (id2jia(playerID) != 1)
            return v;
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
        // isGANG:关于杠，复合点和时为枪杠和，复合自摸则为杠上开花
        // return 胡牌番数，不能胡return0
        vector<pair<int, string>> h = MahjongFanCalculator(
            pack(), hand_num2str(winTile), tile_num2str(winTile),
            hua[myPlayerID], isZIMO, isJUEZHANG(winTile), isGANG, leftTile == 0,
            myPlayerID, quan);
        int totalFan = 0;
        for (auto i = h.begin(); i != h.end(); ++i) {
            totalFan += i->fr;
        }
        if (totalFan >= 8)
            return totalFan;
        else
            return 0;
    }

    /*算番辅助函数*/
    vector<string> hand_num2str(int winTile) {
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
        //是不是桌上最后一张牌
        for (int i = 0; i < 4; ++i) {
            if (peng[i])
                return 1;
        }
        return 0;
    }

    /*决策函数*/
    string decisionZIMO(int tile) {
        //自摸记录后的决策
        //可能的操作：胡，暗杠，补杠，出牌
        //如果可以胡一定胡，但是另外三个的选择不确定

        --hand[tile];
        //杠上开花胡牌
        if (bupai) { //上一局是自己杠，这局补摸的胡了
            if (canHu(tile, 0, 1))
                return "HU";
            bupai = 0;
        }

        //普通自摸胡牌
        if (canHu(tile, 1, 0))
            return "HU";

        ++hand[tile];
        string resp;
        int itmp;
        itmp = canAnGang();
        if (itmp != 34) {
            //在暗杠和出牌里选
            angang_ = itmp;
            resp = "GANG " + tile_num2str(tile);
        } else if (peng[myPlayerID][tile]) {
            //在补杠和出牌里选
            resp = "BUGANG " + tile_num2str(tile);
        } else {
            //出牌策略
            for (int i = 33; i >= 0; --i) {
                if (hand[i] != 0) {
                    resp = "PLAY " + tile_num2str(i);
                    break;
                }
            }
        }
        return resp;
    }

    string decisionTILE(int itmp, int tile) {
        //桌上有一张牌的决策，可能是别人自摸/碰/吃后出的牌
        //可能的决策：胡，明杠，碰，吃，pass
        if (canHu(tile, 0, 0))
            return "HU";

        string resp;
        //是否可以明杠
        if (canMingGang(tile, itmp))
            resp = "GANG";
        //是否可以碰
        if (canPeng(tile, itmp)) {
            resp = "PENG ";
            for (int i = 33; i >= 0; --i) {
                if (i != tile && hand[i] != 0) {
                    resp += tile_num2str(i);
                    break;
                }
            }
        }
        //是否可以吃
        vector<int> v = canChi(tile, itmp);
        if (!v.empty()) {
            resp = "CHI " + tile_num2str(v.front()) + " ";
            for (int i = 33; i >= 0; --i) {
                if (i != v.front() && i != v.front() + 1 &&
                    i != v.front() - 1 && hand[i] != 0) {
                    resp += tile_num2str(i);
                    break;
                }
            }
        }
        //决定是吃碰杠还是pass
        resp = "PASS";

        return resp;
    }

    string decisionQIANGGANG(int tile) {
        //抢杠胡决策
        if (canHu(tile, 0, 1))
            return "HU";
        else
            return "PASS";
    }

    /*保存读取数据*/
    void loadInfo(string data) {
        int tmp;
        istringstream sin(data);
        sin >> myPlayerID >> quan >> angang_ >> lastCard >> lastPlayer >>
            leftTile >> bupai;
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
             << " " << lastPlayer << " " << leftTile << " " << bupai << " ";
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
    string stmp, stmp2;
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
        int tile = tile_str2num(stmp);
        info.addZIMO(tile);
        /*自摸后的决策*/
        resp = info.decisionZIMO(tile);

    } else {
        sin >> itmp >> stmp;
        int tile;
        if (stmp == "BUHUA") {
            info.addBUHUA(itmp);
            resp = "PASS";
        } else if (stmp == "DRAW") {
            info.addDRAW();
            resp = "PASS";
        } else if (stmp == "PLAY") {
            sin >> stmp;
            tile = tile_str2num(stmp);
            info.addPLAY(itmp, tile);
            /*桌上有牌的决策*/
            resp = info.decisionTILE(itmp, tile);
        } else if (stmp == "PENG") {
            sin >> stmp;
            tile = tile_str2num(stmp);
            info.addPENG(itmp, tile);
            /*桌上有牌的决策*/
            resp = info.decisionTILE(itmp, tile);
        } else if (stmp == "CHI") {
            sin >> stmp >> stmp2;
            tile = tile_str2num(stmp2);
            info.addCHI(itmp, info.chiPos(tile_str2num(stmp)), tile);
            /*桌上有牌的决策*/
            resp = info.decisionTILE(itmp, tile);
        } else if (stmp == "GANG") {
            info.addGANG(itmp);
            resp = "PASS";
        } else { // stmp=="BUGANG"
            sin >> stmp;
            tile = tile_str2num(stmp);
            info.addBUGANG(itmp, tile);
            /*抢杠胡决策*/
            resp = info.decisionQIANGGANG(tile);
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
