#include <iostream>
#include <torch/torch.h>
using namespace std;

int main() {
    torch::Tensor tensor = torch::rand({2, 3});
    cout << tensor << endl;
}