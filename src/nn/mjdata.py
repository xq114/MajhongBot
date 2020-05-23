import os
import sys
import random
import torch


def get_filenames_split(file_num, output_path="./model/", val_ratio=0.2):
    data_path = os.path.join(".", "data", "MO")
    all_files = os.listdir(data_path)
    file_list = random.sample(all_files, file_num)
    random.shuffle(file_list)
    val_num = int(file_num * val_ratio)
    train_num = file_num - val_num
    train_path = os.path.join(".", "model", "train_files.txt")
    val_path = os.path.join(".", "model", "val_files.txt")
    with open(train_path, "w") as f:
        for fil in file_list[:train_num]:
            f.write('{}/{}\n'.format(data_path, fil))
    with open(val_path, "w") as f:
        for fil in file_list[train_num:]:
            f.write('{}/{}\n'.format(data_path, fil))


if __name__ == '__main__':
    if len(sys.argv) == 3 and sys.argv[1] == 'sample':
        print('sampling {} data files...'.format(sys.argv[2]))
        nsamples = int(sys.argv[2])
        get_filenames_split(nsamples, 0.2 if nsamples < 100 else 0.1)
    elif len(sys.argv) == 2 and sys.argv[1] == 'sample':
        print('sampling 10 data files...')
        get_filenames_split(10)
    else:
        torch.classes.load_library("txt2batch.so")
        print(torch.classes.loaded_libraries)
        loader = torch.classes.mahjong.Loader(1)
        print(loader.length())
        inputs, labels = loader.next()
        print(inputs[10, 130, ...])
        print(labels)
        loader.next()
        if loader.is_valid():
            print("Loader is valid!")
        else:
            print("Loader is not valid!")
