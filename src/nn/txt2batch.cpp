#include <cstdio>
#include <cstring>
#include <torch/custom_class.h>
#include <torch/script.h>
#include <vector>

#include "statemachine.h"
#include "txt2batch.h"

Loader::Loader(int64_t mode) : ifile{0}, len{0} {
    ftable = fopen(files[mode], "r");
    while (fgets(buff, 50, ftable) != NULL)
        ++len;
    // Need to call init() manually
}

void Loader::init() {
    ifile = 0;
    fseek(ftable, 0, SEEK_SET);
    fgets(buff, 50, ftable);
    int l = strlen(buff);
    if (buff[l - 1] == '\n')
        buff[l - 1] = '\0';
}

Loader::~Loader() { fclose(ftable); }

c10::intrusive_ptr<Loader> Loader::clone() const {
    return c10::make_intrusive<Loader>(mode);
}

int64_t Loader::length() { return len; }

// constexpr char fdong[] = "东";
// constexpr char fnan[] = "南";
// constexpr char fxi[] = "西";
// constexpr char fbei[] = "北";

constexpr char dapai[] = "打牌";
constexpr char mopai[] = "摸牌";
constexpr char chi[] = "吃";
constexpr char peng[] = "碰";
constexpr char gang[] = "明杠";
constexpr char angang[] = "暗杠";
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

void _parse(State *sts, int player, char **p) {
    tile_n tmp, info;
    if (strcmp(*p, mopai) == 0 || strcmp(*p, bhmopai) == 0) {
        *p = strtok(NULL, "\t[',]\n");
        if (**p == 'H') {
            return;
        }
        tmp = _parse_tile(*p);
        sts[player].mo_s(tmp, false);
    } else if (strcmp(*p, ghmopai) == 0) {
        *p = strtok(NULL, "\t[',]\n");
        if (**p == 'H') {
            return;
        }
        tmp = _parse_tile(*p);
        sts[player].mo_s(tmp, true);
    } else if (strcmp(*p, buhua) == 0) {
        *p = strtok(NULL, "\t[',]\n");
        return;
    } else if (strcmp(*p, chi) == 0) {
        *p = strtok(NULL, "\t[',]\n");
        *p = strtok(NULL, "\t[',]\n");
        info = _parse_tile(*p);
        *p = strtok(NULL, "\t[',]\n");
        *p = strtok(NULL, "\t[',]\n");
        tmp = _parse_tile(*p);
        for (int i = 0; i < 4; ++i)
            sts[i].chi_s(player, info, tmp);
        *p = strtok(NULL, "\t[',]\n");
    } else if (strcmp(*p, peng) == 0) {
        for (int _ = 0; _ < 4; ++_)
            *p = strtok(NULL, "\t[',]\n");
        tmp = _parse_tile(*p);
        *p = strtok(NULL, "\t[',]\n");
        int provider = **p - '0';
        for (int i = 0; i < 4; ++i)
            sts[i].peng_s(player, provider, tmp);
    } else if (strcmp(*p, gang) == 0) {
        for (int _ = 0; _ < 5; ++_)
            *p = strtok(NULL, "\t[',]\n");
        tmp = _parse_tile(*p);
        *p = strtok(NULL, "\t[',]\n");
        int provider = **p - '0';
        for (int i = 0; i < 4; ++i)
            sts[i].gang_s(player, provider, tmp);
    } else if (strcmp(*p, angang) == 0) {
        for (int _ = 0; _ < 5; ++_)
            *p = strtok(NULL, "\t[',]\n");
        tmp = _parse_tile(*p);
        *p = strtok(NULL, "\t[',]\n");
        for (int i = 0; i < 4; ++i)
            sts[i].gang_s(player, player, tmp);
    } else if (strcmp(*p, bugang) == 0) {
        for (int _ = 0; _ < 5; ++_)
            *p = strtok(NULL, "\t[',]\n");
        tmp = _parse_tile(*p);
        *p = strtok(NULL, "\t[',]\n");
        for (int i = 0; i < 4; ++i)
            sts[i].bugang_s(player, tmp);
    } else if (strcmp(*p, hupai) == 0) {
        *p = strtok(NULL, "\n");
        *p = strtok(NULL, "\n");
        *p = strtok(NULL, "\n");
    }
    return;
}

std::vector<torch::Tensor> Loader::next() {
    if (ifile == len)
        return std::vector<torch::Tensor>{};

    long lSize;
    char *buffer;
    long result;

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
    buffer = new char[lSize];
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
    char *buffer2 = new char[lSize];
    memcpy(buffer2, buffer, sizeof(char) * lSize);
    int L = 0;
    char *p;
    p = strtok(buffer2, "\n");
    while (p != NULL) {
        if (strcmp(p, dapai) == 0)
            ++L;
        p = strtok(NULL, "\t");
    }
    delete[] buffer2;

    torch::Tensor inputs = torch::empty({L, 150, 4, 34});
    torch::Tensor labels = torch::empty({L});

    /* 启动状态机 */
    State sts[4];
    memset(sts, 0, 4 * sizeof(State));
    tile_n tiles[14];
    constexpr char delim[] = "\t[',]\n";
    p = strtok(buffer, "\n");
    p = strtok(NULL, "\n");
    for (int j = 0; j < 4; ++j) {
        int lim = 0;
        p = strtok(NULL, delim);
        p = strtok(NULL, delim);
        while (isalpha(*p)) {
            tiles[lim++] = _parse_tile(p);
            p = strtok(NULL, delim);
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
    int player = -1, il = 0;
    tile_n label;
    p = strtok(NULL, delim);
    while (p != NULL) {
        player = *p - '0';
        p = strtok(NULL, delim);
        if (p == NULL) {
            fprintf(stderr, "Error unexpected p==NULL, i=%d\n", il);
            break;
        }
        if (strcmp(p, dapai) == 0) {
            assert(il < L);
            sts[player].totensor(inputs, il);
            p = strtok(NULL, delim);
            label = _parse_tile(p);
            labels.index_put_({il}, label);
            ++il;
            for (int i = 0; i < 4; ++i)
                sts[i].discard_s(player, label);
        } else {
            _parse(sts, player, &p);
        }
        p = strtok(NULL, delim);
    }

    delete[] buffer;
    if (ifile < len - 1) {
        fgets(buff, 50, ftable);
        int l = strlen(buff);
        buff[l - 1] = '\0';
    }
    ++ifile;
    return std::vector<torch::Tensor>{inputs.clone(), labels.clone()};
}

static auto testLoader = torch::class_<Loader>("mahjong", "Loader")
                             .def(torch::init<int64_t>())
                             .def("init", &Loader::init)
                             .def("length", &Loader::length)
                             .def("next", &Loader::next)
                             .def("is_valid", &Loader::is_valid);