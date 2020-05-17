#include <iostream>
#include <torch/torch.h>

#include "statemachine.h"
using namespace std;

struct NNInput {
    channel hand[4];
    channel pack[4][4];
    channel menfeng[4];
    channel quanfeng[4];
    channel discarded[4][30][4];
    channel ifeng[4];
    channel idiscard[4];
    channel imo[4];

    channel lshanten[5];
    channel dshanten[5];
    channel probten[10];
    channel averagefan[10];
    channel shanten[5][10];
};

using NNOutput = channel;

class Net : public torch::nn::Module {
    torch::nn::Conv1d conv1 = torch::nn::Conv1d();

  public:
    Net() : torch::nn::Module() {}
    NNOutput &forward(NNInput &input) {}
};

enum class TAgent { discard, chi, peng, gong, hu };
class Agent {
    TAgent type;
    NNInput input;
    NNOutput output;
    Net net;

  public:
    Agent(TAgent t) : type{t} { memset(&input, 0, sizeof(input)); }
};

int main() {
    torch::Tensor tensor = torch::rand({2, 3});
    cout << tensor << endl;
    cout << sizeof(channel) << " " << sizeof(NNInput) << endl;
}