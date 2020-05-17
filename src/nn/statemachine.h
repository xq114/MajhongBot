#include <torch/script.h>

// 牌的表示方法
using tile_n = int;

// 定义channel和unit
constexpr int CHANNEL_SIZE = 9 + 9 + 9 + 4 + 3; // 34
using channel = bool[CHANNEL_SIZE];
using unit = channel[4];

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

    // 各种牌的出现
    unit tile_count; // 需之后转换为tensor

    // 各家打出牌的数量
    int discard_count[4];

    // 缓存：上一次打出的牌与风位
    int cache_feng; // -1表示第一次打出牌/上次牌被吃碰杠了
    tile_n cache_tile;

    void init_feng(int men, int quan);

    void init_hand(tile_n *tiles);

    /**
     * @brief
     */
    torch::Tensor totensor() const;

    /**
     * @brief 打牌 & 看别人打牌
     * @param feng 风位
     * @param tile_num 打出牌的编号
     */
    void discard_s(int feng, tile_n tile_num);

    /**
     * @brief 摸牌
     * @param tile_num 摸到牌的编号
     */
    void mo_s(tile_n tile_num);

    /**
     * @brief 吃牌
     */
    void chi_s(int feng, tile_n tile_num);

    /**
     * @brief 吃牌
     */
    void peng_s(int feng, tile_n tile_num);

    /**
     * @brief 吃牌
     */
    void gang_s(int feng, tile_n tile_num);

    /**
     * @brief 吃牌
     */
    void bugang_s(int feng, tile_n tile_num);
};