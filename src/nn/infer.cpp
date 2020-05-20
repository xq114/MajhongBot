#include <iostream>
#include <torch/torch.h>

#include "statemachine.h"
using namespace std;

class Net : public torch::nn::Module {
    torch::nn::Conv1d conv1 = torch::nn::Conv1d();

  public:
    Net() : torch::nn::Module() {}
    torch::Tensor forward(torch::Tensor &input) {}
};

enum class TAgent { discard, chi, peng, gong, hu };
class Agent {
    TAgent type;
    torch::Tensor input;
    torch::Tensor output;
    Net net;

  public:
    Agent(TAgent t) : type{t} { memset(&input, 0, sizeof(input)); }
};

int main() {
    torch::Tensor tensor = torch::rand({2, 3});
    cout << tensor << endl;
    cout << sizeof(channel) << " " << sizeof(torch::Tensor) << endl;
}