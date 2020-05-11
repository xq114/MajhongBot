// 信息记录中牌的顺序为万条饼风箭，且记录玩家信息中123分别表示是上家/对家/下家提供的
hand_tiles_t info2hand_tiles(info &info) {
    hand_tiles_t ht;
    for (int i = 0; i < 34; i++) {
        if (!chi[0][i].empty()) {
            for (auto j = chi[0][i].begin(); j != chi[0][i].end(); j++) {
                ++ht.pack_count;
                ht.pack_t[ht.pack_count] =
                    make_pack(*j, 1, maketile(i / 9 + 1, i % 9));
            }
        }
        if (peng[0][i] != 0) {
            ++ht.pack_count;
            ht.pack_t[ht.pack_count] =
                make_pack(peng[0][i], 2, maketile(i / 9 + 1, i % 9));
        }
        if (gang[0][j] != 0) {
            ++ht.pack_count;
            ht.pack_t[ht.pack_count] =
                make_pack(gang[0][i], 3, maketile(i / 9 + 1, i % 9));
        }
        if (angang[i]) {
            ++ht.pack_count;
            ht.pack_t[ht.pack_count] =
                make_pack(0, 3, make_pack(i / 9 + 1, i % 9));
        }
        for (int j = 0; j < hand[i]; j++) {
            ht.tile_count++;
            ht.standing_tiles[ht.tile_count] = maketile(i / 9 + 1, i % 9);
        }
    }
    return ht;
}
typedef struct {
    uint8_t form_flag;
    int shanten;
    int dicard_tile;
    int useful_cnt;
} shant;
int count(useful_table_t useful_table) {
    int ans;
    for (int i = 0; i < TILE_TABLE_SIZE; i++)
        ans += useful_table_t[i];
    return ans;
}
//如果和牌（番数大于等于8还没写），那么停止枚举
//如果当前切牌方式能减小上听数，则记录当前切牌方式
//如果当前切牌方式上听数不变但第一类有效牌增多，记录当前切牌方式
bool enum_callback(shant *context, majong::enum_result_t *result) {
    if (result->shanten < context < -shanten) {
        context->form_flag = result->form_flag;
        context->shanten = result->shanten;
        context->dicard_tile = result->dicard_tile;
        context->useful_cnt = count(result->useful_table);
    }
    if (result->shanten == context < -shanten) {
        if (count(result->useful_table) > context->useful_cnt) {
            context->form_flag = result->form_flag;
            context->shanten = result->shanten;
            context->dicard_tile = result->dicard_tile;
            context->useful_cnt = count(result->useful_table);
        }
    }
    if (result->shanten == -1)
        return false;
}
shant *context;
context->shanten = 13;
context->useful_cnt = 0;
hand_tiles_t hand_tiles = info2hand_tiles(info);
hand_tiles_t *phand_tiles = &hand_tiles;
enum_discard_tile(phand_tiles, serving_tile, form_flag, context, enum_callback);
