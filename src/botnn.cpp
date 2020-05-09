#include <iostream>
#include <torch/torch.h>
using namespace std;

constexpr size_t CHANNEL_SIZE = 9 + 9 + 9 + 4 + 3; // 34
using channel = bool[CHANNEL_SIZE];

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

struct NNOutput {
    channel output[4];
};

enum class TAgent { discard, chi, peng, gong, hu };
class Agent {
    NNInput input;

  public:
    Agent() {}
};

int main() {
    torch::Tensor tensor = torch::rand({2, 3});
    cout << tensor << endl;
    cout << sizeof(channel) << " " << sizeof(NNInput) << endl;
}