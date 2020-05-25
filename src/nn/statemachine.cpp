#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <torch/torch.h>

#include "statemachine.h"

// 关于channel的一系列操作
inline void set_c(channel &c, tile_n i) { c[i] = true; }
inline void reset_c(channel &c, tile_n i) { c[i] = false; }
inline bool istrue(channel &c) {
    int s = 0;
    for (int i = 0; i < CHANNEL_SIZE; ++i)
        if (c[i])
            ++s;
    return s * 2 >= CHANNEL_SIZE;
}
inline bool first(channel &c) { return c[0]; }
inline int get(channel &c) {
    int s = 0;
    while (!c[s])
        ++s;
    return s;
}

// 关于unit的操作
inline void set_c(unit &u, int displace, tile_n i) { u[displace][i] = true; }
inline int count(const unit &u, tile_n i) {
    int j = 0;
    while (u[j][i])
        ++j;
    return j;
}
inline void set_c_row(unit &u, int i) {
    for (int j = 0; j < 34; ++j)
        u[i][j] = true;
}
inline int set_c(unit &u, tile_n i) {
#ifndef _BOTZONE_ONLINE
    assert(!u[3][i]);
#endif
    int j = 0;
    while (u[j][i])
        ++j;
    set_c(u[j], i);
    return j;
}
inline int reset_c(unit &u, tile_n i) {
#ifndef _BOTZONE_ONLINE
    assert(u[0][i]);
#endif
    if (u[3][i]) {
        reset_c(u[3], i);
        return 3;
    }
    int j = 0;
    while (u[j][i])
        ++j;
    --j;
    reset_c(u[j], i);
    return j;
}
inline bool first(unit &u) { return u[0][0]; }
inline int get(unit &u) {
    int i = 0;
    while (!u[i][0])
        ++i;
    return i;
}

/* copy from algo.cpp */
tile_t num2tile_t(int n) {
    int a = n / 9 + 1, b = n % 9 + 1;
    return (((a & 0xF) << 4) | (b & 0xF));
}

void State::_flush_cache() {
    int nplace, ntile;
    nplace = discard_count[cache_feng];
    ntile = count(tile_count, cache_tile);
    discard_count[cache_feng]++;
    set_c(discard[nplace][cache_feng][ntile], cache_tile);
    cache_feng = -1;
}

void State::_reset_current() { memset(current_feng, 0, 3 * sizeof(unit)); }

void State::init_feng(int men, int quan) {
    set_c_row(feng_men, men);
    set_c_row(feng_relquan, (quan - men + 4) % 4);
    cache_feng = -1;
    cache_mo = false;
}

void State::init_hand(tile_n *tiles, bool zhuang) {
    int lim = zhuang ? 14 : 13;
    for (int i = 0; i < lim; ++i) {
        tile_n tn = tiles[i];
        if (tn != -1) {
            set_c(hand_tiles, tn);
            set_c(tile_count, tn);
        }
    }
    cache_tile = -1;
}

void State::totensor(torch::Tensor &ret, int ii) const {
    // torch::Tensor ret = torch::zeros({150, 4, 34});
    auto copy = [&ret, ii](int p, const unit &u) {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 34; ++j) {
                ret.index_put_({ii, p, i, j}, u[i][j]);
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
    // return ret.clone();
    return;
}

/**
 * 需要考虑的情况：
 * 1. 别家开局打出一张牌
 * 2. 别家的上家打一张牌，别家摸一张牌（无对应），打出一张牌
 * 3. 别家补杠/暗杠，别家摸一张牌（无对应），打出一张牌
 * 4. 别家碰/吃/明杠，打出一张牌
 * 5. 自己摸一张牌，打出一张牌
 * 6. 自己碰/吃/明杠，打出一张牌
 * 7. 自己开局打出一张牌
 */
void State::discard_s(int feng, tile_n tile_num) {
    int ntile;
    bool self = first(feng_men[feng]);
    if (cache_tile == -1) {
        cache_feng = feng;
        cache_tile = tile_num;
        if (self) {
            // 7
            reset_c(hand_tiles, tile_num);
        } else {
            // 1
            ntile = set_c(tile_count, tile_num);
            set_c_row(current_feng, feng);
            set_c(current_da[ntile], tile_num);
        }
        cache_mo = false;
        return;
    }
    if (self) {
        if (cache_mo) {
            // 5
            cache_feng = feng;
            set_c(hand_tiles, cache_tile);
            cache_tile = tile_num;
        } else {
            // 6
            cache_feng = feng;
            cache_tile = tile_num;
        }
        reset_c(hand_tiles, tile_num);
    } else {
        _reset_current();
        ntile = set_c(tile_count, tile_num);
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
        set_c_row(current_feng, feng);
        set_c(current_da[ntile], tile_num);
    }
    cache_mo = false;
}

/**
 * 需要考虑的情况：
 * 1. 上家打出一张牌，自己摸一张牌
 * 2. 补杠/暗杠，自己摸一张牌
 * 3. 摸牌, 补花后再摸牌
 */
void State::mo_s(tile_n tile_num, bool on_gang) {
    _reset_current();
    set_c(current_mo[0], tile_num);
    if (cache_mo) {
        // 3
        set_c(hand_tiles, cache_tile);
    }
    if (on_gang) {
        // 2
        int feng = get(feng_men);
        set_c_row(current_feng, feng);
    }
    if (cache_feng != -1) {
        // 1
        _flush_cache();
    }
    set_c(tile_count, tile_num);
    cache_feng = -1;
    cache_tile = tile_num;
    cache_mo = true;
}

/**
 * 需要考虑的情况：
 * 1. 别人上家打一张牌，别人吃这张牌
 * 2. 自己上家打一张牌，吃这张牌
 */
void State::chi_s(int feng, tile_n center, tile_n tile_num) {
    bool self = first(feng_men[feng]);
    if (self) {
        // 2
        for (int i = -1; i <= 1; ++i) {
            if (center + i != tile_num) {
                reset_c(hand_tiles, center + i);
            }
        }
        packs[pack_num++] =
            make_pack((feng + 2) % 4, PACK_TYPE_CHOW, num2tile_t(center));
    } else {
        // 1
        for (int i = -1; i <= 1; ++i)
            if (center + i != tile_num)
                set_c(tile_count, center + i);
    }
    set_c(ming[feng], center - 1);
    set_c(ming[feng], center);
    set_c(ming[feng], center + 1);
    set_c(ming_info[feng][0], tile_num);
    cache_feng = -1;
    cache_mo = false;
}

/**
 * 需要考虑的情况：
 * 1. 别人打一张牌，别人碰这张牌
 * 2. 别人打一张牌，自己碰这张牌
 */
void State::peng_s(int feng, int provider_feng, tile_n tile_num) {
    bool self = first(feng_men[feng]);
    if (self) {
        // 2
        reset_c(hand_tiles, tile_num);
        reset_c(hand_tiles, tile_num);
        packs[pack_num++] =
            make_pack(provider_feng, PACK_TYPE_PUNG, num2tile_t(tile_num));
    } else {
        // 1
        set_c(tile_count, tile_num);
        set_c(tile_count, tile_num);
    }
    set_c(ming[feng], tile_num);
    set_c(ming[feng], tile_num);
    set_c(ming[feng], tile_num);
    set_c(ming_info[feng][(provider_feng + 4 - feng) % 4], tile_num);
    cache_feng = -1;
    cache_mo = false;
}

/**
 * 需要考虑的情况：
 * 1. 自己/别人打一张牌，别人杠这张牌
 * 2. 别人打一张牌，自己杠这张牌
 * 3. 自己摸一张牌，开暗杠
 * 4. 自己碰/杠/吃，开暗杠
 */
void State::gang_s(int feng, int provider_feng, tile_n tile_num) {
    bool self = first(feng_men[feng]);
    if (self) {
        if (cache_mo) {
            // 3
            set_c(hand_tiles, cache_tile);
            reset_c(hand_tiles, tile_num);
            reset_c(hand_tiles, tile_num);
            reset_c(hand_tiles, tile_num);
            reset_c(hand_tiles, tile_num);
            packs[pack_num++] =
                make_pack(provider_feng, PACK_TYPE_KONG, num2tile_t(tile_num));
        } else if (provider_feng == feng) {
            // 4
            reset_c(hand_tiles, tile_num);
            reset_c(hand_tiles, tile_num);
            reset_c(hand_tiles, tile_num);
            packs[pack_num++] =
                make_pack(provider_feng, PACK_TYPE_KONG, num2tile_t(tile_num));
        } else {
            // 2
            reset_c(hand_tiles, tile_num);
            reset_c(hand_tiles, tile_num);
            reset_c(hand_tiles, tile_num);
            packs[pack_num++] =
                make_pack(provider_feng, PACK_TYPE_KONG, num2tile_t(tile_num));
        }
    } else {
        // 1
        if (provider_feng == feng)
            set_c(tile_count, tile_num);
        set_c(tile_count, tile_num);
        set_c(tile_count, tile_num);
        set_c(tile_count, tile_num);
    }
    set_c(ming[feng], tile_num);
    set_c(ming[feng], tile_num);
    set_c(ming[feng], tile_num);
    set_c(ming[feng], tile_num);
    set_c(ming_info[feng][0], tile_num);
    set_c(ming_info[feng][(provider_feng + 4 - feng) % 4], tile_num);
    cache_feng = -1;
    cache_mo = false;
}

/**
 * 需要考虑的情况：
 * 1. 别人补杠
 * 2. 自己摸一张牌，自己补杠
 * 3. 自己碰/杠/吃，自己补杠
 */
void State::bugang_s(int feng, tile_n tile_num) {
    bool self = first(feng_men[feng]);
    if (self) {
        if (cache_mo) {
            // 2
            set_c(hand_tiles, cache_tile);
            reset_c(hand_tiles, tile_num);
        } else {
            // 3
            reset_c(hand_tiles, tile_num);
        }
        tile_t t = num2tile_t(tile_num);
        for (int i = 0; i < pack_num; ++i) {
            if (t == ((packs[i]) & 0xFF)) {
                packs[i] |= 0x4300;
                break;
            }
        }
    } else {
        // 1
        set_c(tile_count, tile_num);
    }
    set_c(ming[feng], tile_num);
    set_c(ming_info[feng][0], tile_num);
    cache_feng = -1;
    cache_mo = false;
}