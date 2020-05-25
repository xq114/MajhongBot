// This is not supported by PyTorch and is thus deprecated.

#include <iostream>
#include <sstream>
#include <torch/torch.h>
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

// #include "algo/algo.cpp"
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
        return t;
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

#include "nn/statemachine.cpp"
#include "resnet50_64.cpp"

#define SIMPLEIO 0
//由玩家自己定义，0表示JSON交互，1表示简单交互。

using namespace std;

vector<string> request, response;
vector<string> hand;

MahjongAlgo al;

/**
 * @brief Find self fixed packs.
 * @param packs [out] position to storage packs
 */
void find_packs(State &st, mahjong::pack_t *packs) {
    memcpy(packs, st.packs, st.pack_num * sizeof(pack_t));
}

int canHu(State &st, bool zimo) {
    mahjong::calculate_param_t calculate_param;
    memset(&calculate_param, 0, sizeof(mahjong::calculate_param_t));

    mahjong::tile_t *standing_tiles = calculate_param.hand_tiles.standing_tiles;
    intptr_t &cnt = calculate_param.hand_tiles.tile_count;
    for (int i = 0; i < 34; ++i)
        for (int j = 0; j < 4; ++j) {
            if (st.hand_tiles[j][i])
                standing_tiles[cnt++] = al.num2tile_t(i);
        }
    mahjong::tile_t wt;
    int feng;
    if (zimo)
        wt = get(st.current_mo[0]);
    else {
        feng = get(st.current_feng);
        wt = get(st.current_da[feng]);
    }
    bool basicfw = mahjong::is_basic_form_win(standing_tiles, cnt, wt);
    bool sevenpw = mahjong::is_seven_pairs_win(standing_tiles, cnt, wt);
    bool thirtow = mahjong::is_thirteen_orphans_win(standing_tiles, cnt, wt);
    bool knittsw = mahjong::is_knitted_straight_win(standing_tiles, cnt, wt);
    bool honorkw =
        mahjong::is_honors_and_knitted_tiles_win(standing_tiles, cnt, wt);
    if (basicfw || sevenpw || thirtow || knittsw || honorkw) {
        calculate_param.win_tile = wt;
        /* TODO: add more win_flag detection */
        if (zimo)
            calculate_param.win_flag |= WIN_FLAG_SELF_DRAWN;

        calculate_param.seat_wind = (mahjong::wind_t)get(st.feng_men);
        calculate_param.prevalent_wind =
            (mahjong::wind_t)(get(st.feng_relquan) + get(st.feng_men));

        calculate_param.flower_count = 0;
        calculate_param.hand_tiles.pack_count = st.pack_num;
        find_packs(st, calculate_param.hand_tiles.fixed_packs);

        mahjong::fan_table_t fan_table;
        memset(&fan_table, 0, sizeof(mahjong::fan_table_t));
        int re = mahjong::calculate_fan(&calculate_param, &fan_table);

#ifndef _BOTZONE_ONLINE
        if (re == -1) {
            throw runtime_error("ERROR_WRONG_TILES_COUNT");
        } else if (re == -2) {
            throw runtime_error("ERROR_TILE_COUNT_GREATER_THAN_4");
        } else if (re == -3) {
            throw runtime_error("ERROR_NOT_WIN");
        }
#endif

        int totalFan = 0;
        for (int i = 0; i < mahjong::FAN_TABLE_SIZE; i++) {
            if (fan_table[i] > 0) {
                totalFan += fan_table[i] * mahjong::fan_value_table[i];
            }
        }

        if (totalFan >= 8)
            return totalFan;
        else
            return 0;
    } else
        return 0;
}

using namespace torch::indexing;

int main() {
    int turnID;

    Json::Value inputJSON;
    string resp;
    char tmp[100];
    istringstream sin;
    // ostringstream sout;
    cin.getline(tmp, 100);
    sin.str(tmp);
    Json::CharReaderBuilder rbuilder;
    rbuilder["collectComments"] = false;
    std::string errs;
    bool ok = Json::parseFromStream(rbuilder, sin, &inputJSON, &errs);
    if (!ok) {
        cout << "parse error" << endl;
        return 1;
    }
    turnID = inputJSON["responses"].size();
    string req = inputJSON["requests"][turnID].asString();

    State st;
    memset(&st, 0, sizeof(State));
    MahjongInit();
    int itmp, myPlayerID, quan, last_feng = -1, last_tile;
    sin.str(req);

    // torch::jit::script::Module module;
    ResNet model = DiscardNet();
    try {
        torch::load(model, "./data/discard_50_64.pyt");
        // DONOT SUPPORTED BY PYTORCH!!!(2020/5/25)
    } catch (const c10::Error &e) {
        std::cerr << "error loading the model\n";
        return -1;
    }

    string stmp, stmp2;

    while (cin) {
        sin >> itmp;
        
        if (itmp == 0) {
            sin >> myPlayerID >> quan;
            st.init_feng(myPlayerID, quan);
            resp = "PASS";
        } else if (itmp == 1) {
            tile_n hand_tiles[13];
            for (int i = 0; i < 4; ++i)
                sin >> stmp;
            for (int j = 0; j < 13; j++) {
                sin >> stmp;
                hand_tiles[j] = al.tile_str2num(stmp);
            }
            st.init_hand(hand_tiles, false);
            resp = "PASS";
        } else if (itmp == 2) {
            sin >> stmp;
            tile_n tile = al.tile_str2num(stmp);
            /* TODO: add on_gang detection */
            st.mo_s(tile, false);
            /* 摸牌后的决策 */
            int fan = canHu(st, true);
            if (fan)
                resp = "HU";
            else {
                /* discard model */
                torch::Tensor t = torch::zeros({1, 150, 4, 34});
                st.totensor(t, 0);
                torch::Tensor output = model.forward(t);
                torch::Tensor mask = t.index({0, 0, 0, Slice(None, None)});
                mask.index_put_({tile}, 1);
                torch::Tensor result = mask * output;
                auto preds = get<1>(torch::max(result, 1));
                int tile = preds.item().toInt();
                string tile_str = al.tile_num2str(tile);
                resp = "PLAY " + tile_str;
            }
        } else {
            sin >> itmp >> stmp;
            int tile;
            if (stmp == "BUHUA") {
                resp = "PASS";
            } else if (stmp == "DRAW") {
                resp = "PASS";
            } else if (stmp == "PLAY") {
                sin >> stmp;
                tile = al.tile_str2num(stmp);
                st.discard_s(itmp, tile);
                /* 桌上有牌的决策 */
                last_feng = itmp;
                last_tile = tile;
                if (myPlayerID!=itmp && canHu(st, false))
                    resp = "HU";
                else {
                    // resp = decisionTILE(itmp, tile);
                    resp = "PASS";
                }
            } else if (stmp == "PENG") {
                sin >> stmp;
                tile = al.tile_str2num(stmp);
                st.peng_s(itmp, last_feng, last_tile);

                sin >> stmp;
                tile = al.tile_str2num(stmp);
                st.discard_s(itmp, tile);
                /* 桌上有牌的决策 */
                // resp = decisionTILE(itmp, tile);
                last_feng = itmp;
                last_tile = tile;
                resp = "PASS";
            } else if (stmp == "CHI") {
                sin >> stmp;
                tile = al.tile_str2num(stmp);
                st.chi_s(itmp, tile, last_tile);

                sin >> stmp;
                tile = al.tile_str2num(stmp);
                st.discard_s(itmp, tile);
                /* 桌上有牌的决策 */
                // resp = decisionTILE(itmp, tile);
                last_feng = itmp;
                last_tile = tile;
                resp = "PASS";
            } else if (stmp == "GANG") {
                st.gang_s(itmp, last_feng, last_tile);
                resp = "PASS";
            } else { // stmp=="BUGANG"
                sin >> stmp;
                tile = al.tile_str2num(stmp);
                st.bugang_s(itmp, tile);
                /*抢杠胡决策*/
                // resp = decisionQIANGGANG(tile);
                resp = "PASS";
            }
        }

        Json::Value outputJSON;
        outputJSON["response"] = resp;
        Json::StreamWriterBuilder builder;
        // 调整json为单行输出模式
        builder["indentation"] = ""; // 注释掉本行以得到复杂输出
        Json::StreamWriter *writer{builder.newStreamWriter()};
        writer->write(outputJSON, &cout);
        cout << endl;

        cout << ">>>BOTZONE_REQUEST_KEEP_RUNNING<<<" << endl;

        cin.getline(tmp, 100);
        sin.clear();
        sin.str(tmp);
    }

    return 0;
}