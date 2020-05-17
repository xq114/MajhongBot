#include <cstdio>
#include <string>
#include <torch/script.h>

#include "txt2batch.h"
// #include "statemachine.h"


Loader::Loader(int64_t mode) : i{0}, len{0} {
    ftable = fopen(files[mode], "r");
    while(fgets(buff, 50, ftable)!=NULL)
        ++len;
    fseek(ftable, 0, SEEK_SET);
    fgets(buff, 50, ftable);
}

Loader::~Loader() {
    fclose(ftable);
}

int64_t Loader::length() {
    return len;
}

std::vector<torch::Tensor> Loader::next() {
    current = fopen(buff, "r");

    torch::Tensor inputs = torch::eye(3, 3);
    torch::Tensor labels = torch::ones(3);
    // TODO: finish the parse
    fclose(current);
    fgets(buff, 50, ftable);
    return std::vector<torch::Tensor>{inputs.clone(), labels.clone()};
}

static auto testLoader = torch::class_<Loader>("mahjong", "Loader")
    .def(torch::init<int64_t>())
    .def("length", &Loader::length)
    .def("next", &Loader::next);