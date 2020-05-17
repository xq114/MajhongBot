#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <torch/torch.h>

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
inline void set_row(unit &u, int i) {
    for (int j = 0; j < 34; ++j)
            u[i][j] = true;
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
inline int get(unit &u) { int i=0;while(!u[i][0])++i;return i; }

void State::_flush_cache() {
    int nplace, ntile;
    nplace = discard_count[cache_feng];
    ntile = count(tile_count, cache_tile);
    discard_count[cache_feng]++;
    set(discard[nplace][cache_feng][ntile], cache_tile);
    cache_feng = -1;
}

void State::_reset_current() {
    memset(current_feng, 0, 3*sizeof(unit));
}

void State::init_feng(int men, int quan) {
    set_row(feng_men, men);
    set_row(feng_relquan, (quan-men+4)%4);
    cache_feng = -1;
}

void State::init_hand(tile_n *tiles) {
    for (int i = 0; i < 13; ++i) {
        tile_n tn = tiles[i];
        set(hand_tiles, tn);
        set(tile_count, tn);
    }
    cache_tile = -1;
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
    copy(13, feng_relquan);
    for (int i = 0; i < 29; ++i) {
        for (int j = 0; j < 4; ++j) {
            copy(14 + 4 * i + j, discard[i][j]);
        }
    }
    // Look-ahead information
    copy(130, tile_count);
    return ret.clone();
}

/**
 * 需要考虑的情况：
 * 1. 别家开局摸一张牌（无对应），打出一张牌
 * 2. 别家的上家打一张牌，别家摸一张牌（无对应），打出一张牌
 * 3. 别家补杠/暗杠，别家摸一张牌（无对应），打出一张牌
 * 4. 别家碰/吃/明杠，打出一张牌
 * 5. 自己摸一张牌，打出一张牌
 * 6. 自己碰/吃/明杠，打出一张牌
 */
void State::discard_s(int feng, tile_n tile_num) {
    int ntile;
    if (cache_tile == -1) {
        // 1
        cache_feng = feng;
        cache_tile = tile_num;
        ntile = count(tile_count, tile_num) - 1;
        set_row(current_feng, feng);
        set(current_da[ntile], tile_num);
        return;
    }
    bool self = first(feng_men[cache_feng]);
    if (self) {
        if (cache_feng == feng) {
            // 5
            set(hand_tiles, cache_tile);
            cache_tile = tile_num;
        } else {
            // 6
            cache_feng = feng;
            cache_tile = tile_num;
        }
        reset(hand_tiles, tile_num); 
    } else {
        _reset_current();
        ntile = set(tile_count, tile_num);
        if (cache_feng == feng) {
            // 3
            cache_tile = tile_num;
        } else if (cache_feng == -1) {
            // 4
            cache_feng = feng;
            cache_tile = tile_num;
        } else {
            // 2
            _flush_cache();
            cache_feng = feng;
            cache_tile = tile_num;
        }
        set_row(current_feng, feng);
        set(current_da[ntile], tile_num);
    }
}

/**
 * 需要考虑的情况：
 * 1. 开局摸一张牌
 * 2. 上家打出一张牌，自己摸一张牌
 * 3. 补杠/暗杠，自己摸一张牌
 */
void State::mo_s(tile_n tile_num, bool on_gang) {
    if (cache_tile == -1) {
        // 1
        cache_tile = tile_num;
        cache_feng = get(feng_men);
        set_row(current_feng, cache_feng);
        set(current_mo[0], tile_num);
        set(tile_count, tile_num);
    }
}

void State::chi_s(int feng, tile_n center, tile_n tile_num) {

}