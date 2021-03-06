#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef _BOTZONE_ONLINE
#include "MahjongGB/MahjongGB.h"
#include "MahjongGB/fan_calculator.cpp"
#include "MahjongGB/shanten.cpp"
#include "jsoncpp/json.h"
#elif defined __linux__
#include "../utils/MahjongGBCPP/MahjongGB.cpp"
#include <jsoncpp/json/json.h>
#else
#include "../utils/MahjongGBCPP/MahjongGB.cpp"
#include <json/json.h>
#endif

#define SIMPLEIO 0
//由玩家自己定义，0表示JSON交互，1表示简单交互。

using namespace std;

vector<string> request, response;

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

unordered_map<string, mahjong::tile_t> str2tile_t;
void MahjongInit_t() {
    for (int i = 1; i <= 9; i++) {
        str2tile_t["W" + to_string(i)] =
            mahjong::make_tile(TILE_SUIT_CHARACTERS, i);
        str2tile_t["B" + to_string(i)] = mahjong::make_tile(TILE_SUIT_DOTS, i);
        str2tile_t["T" + to_string(i)] =
            mahjong::make_tile(TILE_SUIT_BAMBOO, i);
    }
    for (int i = 1; i <= 4; i++) {
        str2tile_t["F" + to_string((i))] =
            mahjong::make_tile(TILE_SUIT_HONORS, i);
    }
    for (int i = 1; i <= 3; i++) {
        str2tile_t["J" + to_string((i))] =
            mahjong::make_tile(TILE_SUIT_HONORS, i + 4);
    }
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

typedef struct {
    uint8_t form_flag;
    int shanten;
    int discard_tile;
    int useful_cnt;
} shant;

int count(const mahjong::useful_table_t useful_table) {
    int ans = 0;
    for (int i = 0; i < mahjong::TILE_TABLE_SIZE; i++)
        ans += useful_table[i];
    return ans;
}

//如果和牌（番数大于等于8还没写），那么停止枚举(return false)，没和则继续
//如果当前切牌方式能减小上听数，则记录当前切牌方式
//如果当前切牌方式上听数不变但第一类有效牌增多，记录当前切牌方式
bool enum_callback_info(shant *context, const mahjong::enum_result_t *result) {
    if (result->shanten < context->shanten) {
        context->form_flag = result->form_flag;
        context->shanten = result->shanten;
        context->discard_tile = result->discard_tile;
        context->useful_cnt = count(result->useful_table);
    }
    if (result->shanten == context->shanten) {
        if (count(result->useful_table) > context->useful_cnt) {
            context->form_flag = result->form_flag;
            context->shanten = result->shanten;
            context->discard_tile = result->discard_tile;
            context->useful_cnt = count(result->useful_table);
        }
    }
    if (result->shanten == -1)
        return false;
    return true;
}

struct info {
    int myPlayerID = 0; //即门风
    int quan = 0;
    int hua[4] = {0};   // 玩家花牌数
    int hand[34] = {0}; // hand存储的顺序: W1-9, B1-9, T1-9, F1-4, J1-3

    // 记录所有玩家碰杠吃的情况
    int peng[4][34];       // 记录的是喂牌的player id
    int gang[4][34];       // 不包括暗杠
    int angang[34] = {0};        //只记录了自己的暗杠
    int angang_ = 0;             //记录自己打算杠的牌
    vector<int> chi[4][34] = {}; //表示吃了第几张牌

    int play[4][34] = {0}; // 记录玩家打出过的牌

    int lastCard = 0;   // 上一局桌上的牌，如果是摸牌则为34
    int lastPlayer = 0; // 上一局桌上的牌是谁打的

    int leftTile = 144; //桌上还有多少牌

    bool bupai = 0; //记录这局的自摸是不是杠后补牌

    info(){
        memset(gang,-1,136*sizeof(int));
        memset(peng,-1,136*sizeof(int));
    }
    
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
            gang[itmp][lastCard] = lastPlayer;
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
        // isGANG:关于杠，复合点和时为枪杠和
        vector<string> hand_str = hand_num2str();
        mahjong::tile_t standing_tiles[13];
        int cnt = 0;
        for (string s : hand_str) {
            standing_tiles[cnt++] = str2tile_t[s];
        }
        string wtstr = tile_num2str(winTile);
        mahjong::tile_t wt = str2tile_t[wtstr];
        bool basicfw = mahjong::is_basic_form_win(standing_tiles, cnt, wt);
        bool sevenpw = mahjong::is_seven_pairs_win(standing_tiles, cnt, wt);
        bool thirtow =
            mahjong::is_thirteen_orphans_win(standing_tiles, cnt, wt);
        bool knittsw =
            mahjong::is_knitted_straight_win(standing_tiles, cnt, wt);
        bool honorkw =
            mahjong::is_honors_and_knitted_tiles_win(standing_tiles, cnt, wt);
        if (basicfw || sevenpw || thirtow || knittsw || honorkw) {
            vector<pair<int, string>> h = MahjongFanCalculator(
                pack(), hand_num2str(), tile_num2str(winTile), hua[myPlayerID],
                isZIMO, isJUEZHANG(winTile), isGANG, leftTile == 0, myPlayerID,
                quan);
            int totalFan = 0;
            for (auto i = h.begin(); i != h.end(); ++i) {
                totalFan += i->first;
            }
            if (totalFan >= 8)
                return totalFan;
            else
                return 0;
        } else
            return 0;
    }

    /*算番辅助函数*/
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
                    pack.push_back(
                        make_pair("CHI", make_pair(tile_num2str(i), *j)));
            }
            itmp = peng[myPlayerID][i];
            if (itmp!=-1)
                pack.push_back(make_pair(
                    "PENG", make_pair(tile_num2str(i), id2jia(itmp))));
            itmp = gang[myPlayerID][i];
            if (itmp!=-1)
                pack.push_back(make_pair(
                    "GANG", make_pair(tile_num2str(i), id2jia(itmp))));
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
        // TODO: update this function
        return 0;
    }

    /**
     * @brief 转化为MahjongGB库的手牌结构
     *
     * @retval 手牌结构
     */
    mahjong::hand_tiles_t parseHandTiles() {
        // 信息记录中牌的顺序为万条饼风箭，且记录玩家信息中123分别表示是上家/对家/下家提供的
        mahjong::hand_tiles_t ht;
        memset(&ht, 0, sizeof(mahjong::hand_tiles_t));
        for (int i = 0; i < 34; i++) {
            if (!chi[myPlayerID][i].empty()) {
                for (auto j = chi[myPlayerID][i].begin(); j != chi[myPlayerID][i].end(); j++) {
                    ++ht.pack_count;
                    ht.fixed_packs[ht.pack_count] =
                        mahjong::make_pack(1, 1, num2tile_t(i));
                }
            }
            if (peng[myPlayerID][i] != -1) {
                ++ht.pack_count;
                ht.fixed_packs[ht.pack_count] =
                    mahjong::make_pack(id2jia(peng[myPlayerID][i]), 2, num2tile_t(i));
            }
            if (gang[myPlayerID][i] != -1) {
                ++ht.pack_count;
                ht.fixed_packs[ht.pack_count] =
                    mahjong::make_pack(id2jia(gang[myPlayerID][i]), 3, num2tile_t(i));
            }
            if (angang[i]) {
                ++ht.pack_count;
                ht.fixed_packs[ht.pack_count] =
                    mahjong::make_pack(0, 3, num2tile_t(i));
            }
            for (int j = 0; j < hand[i]; j++) {
                ht.standing_tiles[ht.tile_count++] = num2tile_t(i);
            }
        }
        return ht;
    }

    /**
     * @brief 模型1：决定打哪张牌
     *
     * @param tile 摸到的牌，优先考虑打这张
     * @retval 要打的牌的编号
     */
    int decisionPLAY(int tile) {
        shant context;
        mahjong::hand_tiles_t hand_tiles = parseHandTiles();
        mahjong::tile_t serving_tile = num2tile_t(tile);
        mahjong::win_flag_t form_flag = FORM_FLAG_ALL;
        mahjong::useful_table_t useful_table;
        context.discard_tile = num2tile_t(tile);
        context.shanten = mahjong::basic_form_shanten(
            hand_tiles.standing_tiles, hand_tiles.tile_count, &useful_table);
        context.useful_cnt = count(useful_table);
        enum_discard_tile(
            &hand_tiles, serving_tile, form_flag, &context,
            reinterpret_cast<mahjong::enum_callback_t>(enum_callback_info));
        mahjong::tile_t ti = context.discard_tile;
        return tile_t2num(ti);
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

        string resp;
        int itmp;
        itmp = canAnGang();
        if (itmp != 34) {
            //在暗杠和出牌里选
            angang_ = itmp;
            resp = "GANG " + tile_num2str(tile);
        } else if (peng[myPlayerID][tile]!=-1) {
            //在补杠和出牌里选
            resp = "BUGANG " + tile_num2str(tile);
        } else {
            //出牌策略
            // TODO: 解决听牌不到8番时无法胡牌的问题。可行思路：不胡基本牌型
            int i = decisionPLAY(tile);
            ++hand[tile];
            assert(hand[i] != 0);
            // --hand[i];
            resp = "PLAY " + tile_num2str(i);
        }
        return resp;
    }

    string decisionTILE(int itmp, int tile) {
        if (itmp==myPlayerID) return "PASS";
        //桌上有一张牌的决策，可能是别人自摸/碰/吃后出的牌
        //可能的决策：胡，明杠，碰，吃，pass
        if (canHu(tile, 0, 0))
            return "HU";

        string resp;
        //是否可以明杠
    /*
        if (canMingGang(tile, itmp))
            return "GANG";
        //是否可以碰
        // TODO: 发现有碰而不碰。似乎不应该？虽说只胡特殊牌型确实不需要碰
        // TODO: 碰完打牌也应调用打牌模型
        if (canPeng(tile, itmp)) {
            int i=decisionPLAY(tile);
            if (i!=tile) {
            resp="PENG "+tile_num2str(i);
            return resp;}
        }
        //是否可以吃
        vector<int> v = canChi(tile, itmp);
        if (!v.empty()) {
            resp = "CHI " + tile_num2str(v.front()) + " ";
            int i=decisionPLAY(tile);
            if (i != v.front() && i != v.front() + 1 && i != v.front() - 1){
                resp += tile_num2str(i);
                return resp;
            }
        }
        //决定是吃碰杠还是pass */
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
#ifdef _BOTZONE_ONLINE
    cin >> inputJSON;
#else
    /* 测试数据 */
    istringstream iss("{\
    \"requests\":[\"0 1 1\",\"1 0 0 0 0 W4 W7 W5 T6 B6 W6 J3 F4 W6 T9 T2 F3 W5\",\"3 0 DRAW\",\"3 0 PLAY F1\",\"2 F1\",\"3 1 PLAY F1\",\"3 2 DRAW\",\"3 2 PLAY J3\",\"3 3 DRAW\",\"3 3 PLAY W9\",\"3 0 DRAW\",\"3 0 PLAY W3\",\"2 B7\",\"3 1 PLAY T9\",\"3 2 DRAW\",\"3 2 PLAY W1\",\"3 3 DRAW\",\"3 3 PLAY B4\",\"3 0 DRAW\",\"3 0 PLAY W4\",\"2 W6\",\"3 1 PLAY F3\",\"3 2 DRAW\",\"3 2 PLAY B1\",\"3 3 DRAW\",\"3 3 PLAY F2\",\"3 0 DRAW\",\"3 0 PLAY T5\",\"2 F4\",\"3 1 PLAY J3\",\"3 2 DRAW\",\"3 2 PLAY F1\",\"3 3 DRAW\",\"3 3 PLAY W2\",\"3 0 DRAW\",\"3 0 PLAY F3\",\"2 B8\",\"3 1 PLAY T2\",\"3 2 DRAW\",\"3 2 PLAY F4\",\"3 3 DRAW\",\"3 3 PLAY W1\",\"3 0 DRAW\",\"3 0 PLAY W8\",\"2 T3\",\"3 1 PLAY T3\",\"3 2 DRAW\",\"3 2 PLAY B8\",\"3 3 DRAW\",\"3 3 PLAY B7\",\"3 0 DRAW\",\"3 0 PLAY T1\",\"2 T1\",\"3 1 PLAY T1\",\"3 2 DRAW\",\"3 2 PLAY F1\",\"3 3 DRAW\",\"3 3 PLAY W6\",\"3 0 DRAW\",\"3 0 PLAY B2\",\"2 T8\",\"3 1 PLAY W6\",\"3 2 DRAW\",\"3 2 PLAY B1\",\"3 3 DRAW\",\"3 3 PLAY F3\",\"3 0 DRAW\",\"3 0 PLAY F2\",\"2 J3\",\"3 1 PLAY J3\",\"3 2 DRAW\",\"3 2 PLAY T7\"],\"responses\":[\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PLAY F1\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PLAY T9\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PLAY F3\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PLAY J3\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PLAY T2\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PLAY T3\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PLAY T1\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PLAY W6\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PASS\",\"PLAY J3\",\"PASS\",\"PASS\"],\
    \"data\": \"1 1 0 34 1 57 0 0 0 0 0 0 0 0 1 2 2 1 0 0 0 0 0 0 0 1 1 1 0 0 0 0 0 0 1 0 1 0 0 0 0 2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1 0 0 0 1 0 0 1 0 0 0 0 0 0 0 1 0 0 0 1 0 0 0 0 1 1 1 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 1 1 1 0 0 0 0 0 1 1 0 1 0 0 0 2 1 0 0 0 0 0 0 0 0 2 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 2 0 0 1 0 0 1 1 1 0 0 0 1 0 0 1 0 0 0 1 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 1 1 0 0 0 0 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 \"\
}");
    iss >> inputJSON;
#endif
    turnID = inputJSON["responses"].size();
    req = inputJSON["requests"][turnID].asString();
    if (turnID > 0 && inputJSON["data"] != Json::nullValue)
        info.loadInfo(inputJSON["data"].asString());
#endif
    int itmp;
    ostringstream sout;
    istringstream sin;
    MahjongInit();
    MahjongInit_t();
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
        /*摸牌后的决策*/
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
