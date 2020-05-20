#include <cstdio>
#include <string>
#include <torch/script.h>
#include <vector>

#include "statemachine.h"
#include "txt2batch.h"

Loader::Loader(int64_t mode) : i{0}, len{0} {
    ftable = fopen(files[mode], "r");
    while (fgets(buff, 50, ftable) != NULL)
        ++len;
    fseek(ftable, 0, SEEK_SET);
    fgets(buff, 50, ftable);
}

Loader::~Loader() { fclose(ftable); }

c10::intrusive_ptr<Loader> Loader::clone() const {
    return c10::make_intrusive<Loader>(mode);
}

int64_t Loader::length() { return len; }

constexpr char fdong[] = "东";
constexpr char fnan[] = "南";
constexpr char fxi[] = "西";
constexpr char fbei[] = "北";

constexpr char dapai[] = "打牌";
constexpr char mopai[] = "摸牌";
constexpr char chi[] = "吃";
constexpr char peng[] = "碰";
constexpr char gang[] = "杠";
constexpr char buhua[] = "补花";
constexpr char bhmopai[] = "补花后摸牌";
constexpr char bugang[] = "补杠";
constexpr char ghmopai[] = "杠后摸牌";
constexpr char hupai[] = "和牌";

tile_n _parse_tile(char *s) {
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

void _parse(State *sts, int player, char *p) {
    tile_n tmp;
    if (strcmp(p, mopai) == 0) {
        p = strtok(NULL, "\t[',]\n");
        tmp = _parse_tile(p);
        sts[player].mo_s(tmp, false);
    } else if (strcmp(p, chi) == 0) {
        p = strtok(NULL, "\t[',]\n");
        p = strtok(NULL, "\t[',]\n");
        tmp = _parse_tile(p);
        p = strtok(NULL, "\t[',]\n");

    } else if (strcmp(p, peng) == 0) {

    } else if (strcmp(p, gang) == 0) {

    } else if (strcmp(p, buhua) == 0) {

    } else if (strcmp(p, bugang) == 0) {

    } else if (strcmp(p, hupai) == 0) {
        return;
    }
}

std::vector<torch::Tensor> Loader::next() {
    long lSize;
    char *buffer;
    size_t result;

    /* 若要一个byte不漏地读入整个文件，只能采用二进制方式打开 */
    current = fopen(buff, "rb");
    if (current == NULL) {
        fputs("File error", stderr);
        return std::vector<torch::Tensor>();
    }

    /* 获取文件大小 */
    fseek(current, 0, SEEK_END);
    lSize = ftell(current);
    rewind(current);

    /* 分配内存存储整个文件 */
    buffer = (char *)malloc(sizeof(char) * lSize);
    if (buffer == NULL) {
        fputs("Memory error", stderr);
        return std::vector<torch::Tensor>();
    }

    /* 将文件拷贝到buffer中 */
    result = fread(buffer, 1, lSize, current);
    if (result != lSize) {
        fputs("Reading error", stderr);
        return std::vector<torch::Tensor>();
    }
    fclose(current);

    /* 统计打牌操作出现次数 */
    int L = 0;
    char *p, *tmp;
    p = strtok(buffer, "\n");
    while (p != NULL) {
        if (strcmp(p, dapai) == 0)
            ++L;
        p = strtok(NULL, "\t");
    }

    torch::Tensor inputs = torch::empty({L, 150, 4, 34});
    torch::Tensor labels = torch::empty({L});

    // TODO: finish the parse
    /* 启动状态机 */
    State sts[4];
    memset(sts, 0, 4 * sizeof(State));
    tile_n tiles[14];
    p = strtok(buffer, "\n");
    p = strtok(NULL, "\n");
    for (int j = 0; j < 4; ++j) {
        int lim = 0;
        p = strtok(NULL, "\t");
        p = strtok(NULL, "\t[',]");
        while (isalpha(*p)) {
            tiles[lim++] = _parse_tile(p);
            p = strtok(NULL, "\t[',]\n");
        }
        if (lim == 13)
            sts[j].init_hand(tiles, false);
        else {
            sts[j].init_hand(tiles, true);
            for (int k = 0; k < 4; ++k)
                sts[k].init_feng(k, j);
        }
    }
    /* 输出状态 */
    int player, i = 0;
    tile_n label;
    p = strtok(NULL, "\n");
    while (p != NULL) {
        player = *p - '0';
        p = strtok(NULL, "\t");
        if (strcmp(p, dapai) == 0) {
            sts[player].totensor(inputs, i);
            p = strtok(NULL, "\t[',]");
            label = _parse_tile(p);
            labels.index_put_({i}, label);
            ++i;
            for (int i = 0; i < 4; ++i)
                sts[i].discard_s(player, label);
        } else {
            _parse(sts, player, p);
        }
        p = strtok(NULL, "\t");
    }

    fgets(buff, 50, ftable);
    return std::vector<torch::Tensor>{inputs.clone(), labels.clone()};
}

static auto testLoader = torch::class_<Loader>("mahjong", "Loader")
                             .def(torch::init<int64_t>())
                             .def("length", &Loader::length)
                             .def("next", &Loader::next);