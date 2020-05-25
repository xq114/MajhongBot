#include <iostream>
#include <torch/torch.h>

torch::nn::Conv2dOptions conv_options(int64_t in_planes, int64_t out_planes,
                                      int64_t kernel_size, int64_t stride = 1,
                                      int64_t padding = 0) {
    torch::nn::Conv2dOptions conv_options =
        torch::nn::Conv2dOptions(in_planes, out_planes, kernel_size);
    conv_options.stride(stride);
    conv_options.padding(padding);
    conv_options.bias(false);
    return conv_options;
}

struct BasicBlock : public torch::nn::Module {

    static const int expansion;

    int64_t stride;
    torch::nn::Conv2d conv1;
    torch::nn::BatchNorm2d bn1;
    torch::nn::Conv2d conv2;
    torch::nn::BatchNorm2d bn2;
    torch::nn::Sequential downsample;

    BasicBlock(int64_t inplanes, int64_t planes, int64_t stride_ = 1)
        : conv1(conv_options(inplanes, planes, 3, stride_, 1)), bn1(planes),
          conv2(conv_options(planes, planes, 3, 1, 1)), bn2(planes) {
        register_module("conv1", conv1);
        register_module("bn1", bn1);
        register_module("conv2", conv2);
        register_module("bn2", bn2);
        stride = stride_;
    }

    torch::Tensor forward(torch::Tensor x) {
        at::Tensor residual(x.clone());

        x = conv1->forward(x);
        x = bn1->forward(x);
        x = torch::relu(x);

        x = conv2->forward(x);
        x = bn2->forward(x);

        x += residual;
        x = torch::relu(x);

        return x;
    }
};

const int BasicBlock::expansion = 1;

struct ResNet : public torch::nn::Module {
    int64_t inplanes = 64;
    torch::nn::Conv2d conv1;
    torch::nn::BatchNorm2d bn1;
    torch::nn::ReLU relu;
    torch::nn::Sequential layer1;
    torch::nn::Linear fc;

    ResNet(int64_t layer_num, int64_t num_classes)
        : conv1(conv_options(150, 64, 3, 1, 1)), bn1(64),
          layer1(_make_layer(64, layer_num)), fc(64 * 4 * 34, num_classes) {
        register_module("conv1", conv1);
        register_module("bn1", bn1);
        register_module("relu", relu);
        register_module("layer1", layer1);
        register_module("fc", fc);
    }

    torch::Tensor forward(torch::Tensor x) {
        x = conv1->forward(x);
        x = bn1->forward(x);
        x = relu->forward(x);

        x = layer1->forward(x);

        x = torch::flatten(x, 1);
        x = torch::nn::functional::normalize(x);
        x = fc->forward(x);

        return x;
    }

    torch::nn::Sequential _make_layer(int64_t planes, int64_t blocks,
                                      int64_t stride = 1) {
        torch::nn::Sequential layers;
        layers->push_back(BasicBlock(planes, planes, stride));
        for (int64_t i = 1; i < blocks; i++) {
            layers->push_back(BasicBlock(planes, planes));
        }

        return layers;
    }
};

ResNet DiscardNet() { return ResNet(24, 34); }

// int main() {
//     torch::Device device("cpu");
//     if (torch::cuda::is_available()) {
//         device = torch::Device("cuda:0");
//     }

//     torch::Tensor t = torch::rand({2, 150, 4, 34}).to(device);
//     ResNet<BasicBlock> resnet = DiscardNet();
//     resnet.to(device);

//     t = resnet.forward(t);
//     std::cout << t.sizes() << std::endl;

//     size_t total_num = 0;
//     for (auto p : resnet.parameters()) {
//         total_num += p.numel();
//     }
//     std::cout << "total parameters: " << total_num << std::endl;

//     size_t trainable_num = 0;
//     for (auto p : resnet.parameters()) {
//         if (p.requires_grad())
//             trainable_num += p.numel();
//     }
//     std::cout << "trainable parameters: " << trainable_num << std::endl;
// }