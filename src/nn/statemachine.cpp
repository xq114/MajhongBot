#include <cassert>
#include <cstdio>
#include <torch/script.h>

#include "statemachine.h"

// 关于channel的一系列操作
inline void set(channel &c, tile_n i) { c[i] = true; }
inline void reset(channel &c, tile_n i) { c[i] = false; }
inline bool istrue(channel &c) {
    int s = 0;
    for (int i = 0; i < CHANNEL_SIZE; ++i)
        if (c[i])
            ++s;
    return s * 2 >= CHANNEL_SIZE;
}
inline bool first(channel &c) { return c[0]; }

// 关于unit的操作
inline void set(unit &u, int displace, tile_n i) { u[displace][i] = true; }
inline int count(const unit &u, tile_n i) {
    int j = 0;
    while (u[j][i])
        ++j;
    return j;
}
inline int set(unit &u, tile_n i) {
#ifndef _BOTZONE_ONLINE
    assert(!u[3][i]);
#endif
    int j = 0;
    while (u[j][i])
        ++j;
    set(u[j], i);
    return j;
}
inline int reset(unit &u, tile_n i) {
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
    --j;
    reset(u[j], i);
    return j;
}
inline bool first(unit &u) { return u[0][0]; }

void State::init_feng(int men, int quan) {
    auto set_row = [](unit &u, int i) {
        for (int j = 0; j < 34; ++j)
            u[i][j] = true;
    };
    set_row(feng_men, men);
    set_row(feng_quan, quan);
}

void State::init_hand(tile_n *tiles) {
    for (int i = 0; i < 13; ++i) {
        tile_n tn = tiles[i];
        set(hand_tiles, tn);
        set(tile_count, tn);
    }
}

torch::Tensor State::totensor() const {
    torch::Tensor ret = torch::zeros({150, 4, 34});
    auto copy = [&ret](int p, const unit &u) {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 34; ++j) {
                ret[p][i][j] = u[i][j];
            }
        return;
    };
    // Basic information
    copy(0, hand_tiles);
    copy(1, current_mo);
    copy(2, current_feng);
    copy(3, current_da);
    for (int i = 0; i < 4; ++i) {
        copy(4 + 2 * i, ming[i]);
        copy(4 + 2 * i + 1, ming_info[i]);
    }
    copy(12, feng_men);
    copy(13, feng_quan);
    for (int i = 0; i < 29; ++i) {
        for (int j = 0; j < 4; ++j) {
            copy(14 + 4 * i + j, discard[i][j]);
        }
    }
    // Look-ahead information
    copy(130, tile_count);
    return ret.clone();
}

void State::discard_s(int feng, tile_n tile_num) {
    if (cache_feng != -1) {
        // 上次操作牌也是打出
        int nplace, ntile;
        nplace = discard_count[cache_feng];
        if (first(feng_men[cache_feng])) {
            // 自己打牌
            reset(hand_tiles, cache_tile);
            ntile = count(tile_count, tile_num) - 1;
        } else {
            // 别人打牌
            discard_count[cache_feng]++;
            ntile = set(tile_count, tile_num);
        }
        set(discard[nplace][feng][ntile], cache_tile);
    }
    cache_feng = feng;
    cache_tile = tile_num;
}