#include <string>
#include <torch/script.h>
#include <vector>

constexpr char files[2][25]{"./model/train_files.txt", "./model/val_files.txt"};

struct Loader : torch::CustomClassHolder {
    int64_t mode; // 0 for train and 1 for val
    char buff[50];
    int64_t ifile;
    int64_t len;
    FILE *ftable;
    FILE *current;
    char buffer[5000], buffer2[5000];

    Loader(int64_t mode);
    ~Loader();

    void init();

    c10::intrusive_ptr<Loader> clone() const;

    /**
     * @brief get length of the Loader.
     */
    int64_t length();

    /**
     * @brief Read a txt file and return an array of tensor batch (inputs,
     * labels)
     * @return vector of tensors, `inputs` is of the dim Nx150x4x34, `labels` is
     * of the dim Nx1
     */
    std::vector<torch::Tensor> next();

    bool is_valid() const { return ifile < len; }
};