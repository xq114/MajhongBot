#include <cassert>
#include <cstdio>

// 牌的表示方法
using tile_n = int;

// 定义channel和unit
constexpr int CHANNEL_SIZE = 9 + 9 + 9 + 4 + 3; // 34
using channel = bool[CHANNEL_SIZE];
using unit = channel[4];

// 关于channel的一系列操作
void set(channel &c, tile_n i) { c[i] = true; }
void reset(channel &c, tile_n i) { c[i] = false; }
bool istrue(channel &c) {
    int s = 0;
    for (int i = 0; i < CHANNEL_SIZE; ++i)
        if (c[i])
            ++s;
    return s * 2 >= CHANNEL_SIZE;
}
bool first(channel &c) { return c[0]; }

// 关于unit的操作
void set(unit &u, int displace, tile_n i) { u[displace][i] = true; }
void set(unit &u, tile_n i) {
#ifndef _BOTZONE_ONLINE
    assert(!u[3][i]);
#endif
    int j = 0;
    while (u[j][i])
        ++j;
    set(u[j], i);
}
void reset(unit &u, tile_n i) {
#ifndef _BOTZONE_ONLINE
    assert(u[0][i]);
#endif
    if (u[3][i]) {
        reset(u[3], i);
        return;
    }
    int j = 0;
    while (u[j][i])
        ++j;
    reset(u[j - 1], i);
}
bool first(unit &u) { return u[0][0]; }

struct State {
    // 手牌
    unit hand_tiles;

    // 当前操作
    unit current_feng;
    unit current_mo;
    unit current_da;

    // 明牌
    unit ming[4];
    unit ming_info[4];

    // 风位
    unit feng_men;
    unit feng_quan;

    // 场上牌
    unit discard[29][4];

    // 各种牌的出现次数
    int tile_count[34]; // 需之后转换为tensor

    // 各家打出牌的数量
    int discard_count[4];

    // 缓存：上一次打出的牌与风位
    int cache_feng; // -1表示第一次打出牌/上次牌被吃碰杠了
    tile_n cache_tile;

    /**
     * @brief 打牌 & 看别人打牌
     * @param feng 风位
     * @param tile_num 打出牌的编号
     */
    void discard(int feng, tile_n tile_num) {
        if (cache_feng != -1) {
            // 上次操作牌也是打出
            int nplace, ntile;
            nplace = discard_count[cache_feng];
            ntile = tile_count[cache_tile];
            if (first(feng_men[cache_feng])) {
                // 自己打牌
                reset(hand_tiles, cache_tile);
            } else {
                // 别人打牌
                discard_count[cache_feng]++;
                tile_count[cache_tile]++;
            }
            set(discard[nplace][feng][ntile], cache_tile);
        }
        cache_feng = feng;
        cache_tile = tile_num;
    }

    /**
     * @brief 摸牌
     * @param tile_num 摸到牌的编号
     */
    void mo(tile_n tile_num) {}

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
            if (itmp)
                pack.push_back(make_pair(
                    "PENG", make_pair(tile_num2str(i), id2jia(itmp))));
            itmp = gang[myPlayerID][i];
            if (itmp)
                pack.push_back(make_pair(
                    "GANG", make_pair(tile_num2str(i), id2jia(itmp))));
        }
        return pack;
    }
    int id2jia(int id) {
        // 判断是上下对家
        // 1为上家，2为对家，3为下家
        return (4 + myPlayerID - id) % 4;
    }

    int chiPos(int mid) {
        //计算吃了第几张牌
        return lastCard - mid + 2;
    }
};