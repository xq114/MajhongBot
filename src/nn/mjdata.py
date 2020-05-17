import os
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
        f.write('\n'.join(file_list[:train_num]))
    with open(val_path, "w") as f:
        f.write('\n'.join(file_list[train_num:]))


# class DiscardDataset(IterableDataset):
#     def __init__(self, start, end):
#         super().__init__()
#         assert end > start, "this example code only works with end >= start"
#         self.start = start
#         self.end = end

#     def __iter__(self):
#         worker_info = get_worker_info()
#         if worker_info is None:
#             iter_start = self.start
#             iter_end = self.end
#         else:
#             per_worker = 1
#             worker_id = worker_info().id
#             iter_start = self.start+worker_id*per_worker
#             iter_end = min(iter_start + per_worker, self.end)
#         return iter(range(iter_start, iter_end))


if __name__ == '__main__':
    torch.classes.load_library("txt2batch.so")
    print(torch.classes.loaded_libraries)
    loader = torch.classes.mahjong.Loader(0)
    print(loader.length())
