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
    else if (s[0] == 'B')
        return 9 + s[1] - '1';
    else if (s[0] == 'T')
        return 18 + s[1] - '1';
    else if (s[0] == 'F')
        return 27 + s[1] - '1';
    else if (s[0] == 'J')
        return 31 + s[1] - '1';
    return 34;
}

void _parse(State *sts, int player, char **p, char **psaveptr) {
    tile_n tmp, info;
    if (strcmp(*p, mopai) == 0 || strcmp(*p, bhmopai) == 0) {
        *p = strtok_r(NULL, "\t[',]\n", psaveptr);
        if (**p == 'H') {
            return;
        }
        tmp = _parse_tile(*p);
        sts[player].mo_s(tmp, false);
    } else if (strcmp(*p, ghmopai) == 0) {
        *p = strtok_r(NULL, "\t[',]\n", psaveptr);
        if (**p == 'H') {
            return;
        }
        tmp = _parse_tile(*p);
        sts[player].mo_s(tmp, true);
    } else if (strcmp(*p, buhua) == 0) {
        *p = strtok_r(NULL, "\t[',]\n", psaveptr);
        return;
    } else if (strcmp(*p, chi) == 0) {
        *p = strtok_r(NULL, "\t[',]\n", psaveptr);
        *p = strtok_r(NULL, "\t[',]\n", psaveptr);
        info = _parse_tile(*p);
        *p = strtok_r(NULL, "\t[',]\n", psaveptr);
        *p = strtok_r(NULL, "\t[',]\n", psaveptr);
        tmp = _parse_tile(*p);
        for (int i = 0; i < 4; ++i)
            sts[i].chi_s(player, info, tmp);
        *p = strtok_r(NULL, "\t[',]\n", psaveptr);
    } else if (strcmp(*p, peng) == 0) {
        for (int _ = 0; _ < 4; ++_)
            *p = strtok_r(NULL, "\t[',]\n", psaveptr);
        tmp = _parse_tile(*p);
        *p = strtok_r(NULL, "\t[',]\n", psaveptr);
        int provider = **p - '0';
        for (int i = 0; i < 4; ++i)
            sts[i].peng_s(player, provider, tmp);
    } else if (strcmp(*p, gang) == 0) {
        for (int _ = 0; _ < 5; ++_)
            *p = strtok_r(NULL, "\t[',]\n", psaveptr);
        tmp = _parse_tile(*p);
        *p = strtok_r(NULL, "\t[',]\n", psaveptr);
        int provider = **p - '0';
        for (int i = 0; i < 4; ++i)
            sts[i].gang_s(player, provider, tmp);
    } else if (strcmp(*p, angang) == 0) {
        for (int _ = 0; _ < 5; ++_)
            *p = strtok_r(NULL, "\t[',]\n", psaveptr);
        tmp = _parse_tile(*p);
        *p = strtok_r(NULL, "\t[',]\n", psaveptr);
        for (int i = 0; i < 4; ++i)
            sts[i].gang_s(player, player, tmp);
    } else if (strcmp(*p, bugang) == 0) {
        for (int _ = 0; _ < 5; ++_)
            *p = strtok_r(NULL, "\t[',]\n", psaveptr);
        tmp = _parse_tile(*p);
        *p = strtok_r(NULL, "\t[',]\n", psaveptr);
        for (int i = 0; i < 4; ++i)
            sts[i].bugang_s(player, tmp);
    } else if (strcmp(*p, hupai) == 0) {
        *p = strtok_r(NULL, "\n", psaveptr);
        *p = strtok_r(NULL, "\n", psaveptr);
        *p = strtok_r(NULL, "\n", psaveptr);
    }
    return;
}

std::vector<torch::Tensor> Loader::next() {
    if (ifile == len)
        return std::vector<torch::Tensor>{};

    long lSize;
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
    if (lSize >= 5000) {
        fputs("Memory error", stderr);
        return std::vector<torch::Tensor>();
    }
    // buffer = new char[lSize];
    // if (buffer == NULL) {
    //     fputs("Memory error", stderr);
    //     return std::vector<torch::Tensor>();
    // }

    /* 将文件拷贝到buffer中 */
    result = fread(buffer, 1, lSize, current);
    if (result != lSize) {
        fputs("Reading error", stderr);
        return std::vector<torch::Tensor>();
    }
    fclose(current);

    /* 统计打牌操作出现次数 */
    // buffer2 = new char[lSize];
    memcpy(buffer2, buffer, sizeof(char) * 5000);
    int L = 0;
    char *p, *saveptr;
    p = strtok_r(buffer2, "\n", &saveptr);
    while (p != NULL) {
        if (strcmp(p, dapai) == 0)
            ++L;
        p = strtok_r(NULL, "\t", &saveptr);
    }
    // delete[] buffer2;

    torch::Tensor inputs = torch::zeros({L, 150, 4, 34});
    torch::Tensor labels = torch::zeros({L}, {torch::dtype<long>()});

    /* 启动状态机 */
    State sts[4];
    memset(sts, 0, 4 * sizeof(State));
    tile_n tiles[14];
    constexpr char delim[] = "\t[',]\n";
    p = strtok_r(buffer, "\n", &saveptr);
    p = strtok_r(NULL, "\n", &saveptr);
    for (int j = 0; j < 4; ++j) {
        int lim = 0;
        p = strtok_r(NULL, delim, &saveptr);
        p = strtok_r(NULL, delim, &saveptr);
        while (isalpha(*p)) {
            if (_parse_tile(p) != 34)
                tiles[lim] = _parse_tile(p);
            else
                tiles[lim] = -1;
            ++lim;
            p = strtok_r(NULL, delim, &saveptr);
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
    p = strtok_r(NULL, delim, &saveptr);
    while (p != NULL && il < L) {
        player = *p - '0';
        p = strtok_r(NULL, delim, &saveptr);
        if (p == NULL) {
            fprintf(stderr, "Unexpected Error p==NULL, i/L=%d/%d\n", il, L);
            break;
        }
        if (strcmp(p, dapai) == 0) {
            assert(il < L);
            sts[player].totensor(inputs, il);
            p = strtok_r(NULL, delim, &saveptr);
            label = _parse_tile(p);
            labels.index_put_({il}, label);
            ++il;
            for (int i = 0; i < 4; ++i)
                sts[i].discard_s(player, label);
        } else {
            _parse(sts, player, &p, &saveptr);
        }
        if (p != NULL)
            p = strtok_r(NULL, delim, &saveptr);
    }

    // delete[] buffer;
    if (ifile < len - 1) {
        fgets(buff, 50, ftable);
        int l = strlen(buff);
        buff[l - 1] = '\0';
    }
    ++ifile;
    return std::vector<torch::Tensor>{inputs.clone(), labels.clone()};
    // return std::vector<torch::Tensor>();
}

static auto testLoader = torch::class_<Loader>("mahjong", "Loader")
                             .def(torch::init<int64_t>())
                             .def("init", &Loader::init)
                             .def("length", &Loader::length)
                             .def("next", &Loader::next)
                             .def("is_valid", &Loader::is_valid);