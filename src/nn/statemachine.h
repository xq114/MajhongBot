#include <torch/torch.h>

// 牌的表示方法
using tile_n = int;

// 定义channel和unit
constexpr int CHANNEL_SIZE = 9 + 9 + 9 + 4 + 3; // 34
using channel = bool[CHANNEL_SIZE];
using unit = channel[4];

#define PACK_TYPE_CHOW 1 ///< 顺子
#define PACK_TYPE_PUNG 2 ///< 刻子
#define PACK_TYPE_KONG 3 ///< 杠

/* copy from tile.h */
typedef uint8_t tile_t;
typedef uint16_t pack_t;
static inline pack_t make_pack(uint8_t offer, uint8_t type, tile_t tile) {
    return (offer << 12 | (type << 8) | tile);
}

/* 使用前需要手动清空：`memset(s, 0, sizeof(State));` */
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
    unit feng_relquan;

    // 场上牌
    unit discard[29][4];

    // 各种牌的出现
    unit tile_count; // 需之后转换为tensor

    // 各家打出牌的数量
    int discard_count[4];

    // 缓存：上一次打出的牌与风位
    int cache_feng;    // -1表示第一次打出牌/上次牌被吃碰杠了
    bool cache_mo;     // 上次操作是否为摸牌
    tile_n cache_tile; // 摸牌/打牌的缓存

    // 明牌的详细记录
    int pack_num;
    pack_t packs[4];

    /**
     * @brief 将cache里的牌扔到场上
     */
    void _flush_cache();

    /**
     * @brief 将current的状态清空
     */
    void _reset_current();

    /**
     * @brief 初始化风位
     * @param men 门风
     * @param quan 圈风
     */
    void init_feng(int men, int quan);

    /**
     * @brief 初始化手牌
     * @param tiles 手牌数组，大小为13/14
     */
    void init_hand(tile_n *tiles, bool zhuang);

    /**
     * @brief 将当前状态转成Tensor, 存储在第i维
     * @retval tensor Lx150x4x34
     */
    void totensor(torch::Tensor &ret, int i) const;
    // torch::Tensor totensor() const;

    /**
     * @brief 打牌 & 看别人打牌
     * @param feng 相对风位
     * @param tile_num 打出牌的编号
     */
    void discard_s(int feng, tile_n tile_num);

    /**
     * @brief 摸牌
     * @param tile_num 摸到牌的编号
     * @param on_gang 是否为杠上摸牌
     */
    void mo_s(tile_n tile_num, bool on_gang);

    /**
     * @brief 吃牌
     * @param feng 吃牌相对风位
     * @param center 顺子的中心牌
     * @param tile_num 被吃的牌
     */
    void chi_s(int feng, tile_n center, tile_n tile_num);

    /**
     * @brief 碰牌
     * @param feng 碰牌相对风位
     * @param provider_feng 被碰牌相对风位
     * @param tile_num 被碰牌
     */
    void peng_s(int feng, int provider_feng, tile_n tile_num);

    /**
     * @brief 杠牌
     * @param feng 杠牌相对风位
     * @param provider_feng 被杠牌相对风位，暗杠此值为feng
     * @param tile_num 被杠牌
     */
    void gang_s(int feng, int provider_feng, tile_n tile_num);

    /**
     * @brief 补杠牌
     * @param feng 补杠相对风位
     * @param tile_num 补杠牌
     */
    void bugang_s(int feng, tile_n tile_num);
};