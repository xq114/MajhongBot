// 牌的表示方法
using tile_n = int;

// 定义channel和unit
constexpr int CHANNEL_SIZE = 9 + 9 + 9 + 4 + 3; // 34
using channel = bool[CHANNEL_SIZE];
using unit = channel[4];

struct State;