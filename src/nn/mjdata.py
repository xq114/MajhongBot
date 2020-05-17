import torch
from torch.utils.data import IterableDataset
from torch.utils.data import get_worker_info


class DiscardDataset(IterableDataset):
    def __init__(self, start, end):
        super().__init__()
        assert end > start, "this example code only works with end >= start"
        self.start = start
        self.end = end

    def __iter__(self):
        worker_info = get_worker_info()
        if worker_info is None:
            iter_start = self.start
            iter_end = self.end
        else:
            per_worker = 1
            worker_id = worker_info().id
            iter_start = self.start+worker_id*per_worker
            iter_end = min(iter_start + per_worker, self.end)
        return iter(range(iter_start, iter_end))


if __name__ == '__main__':
    ds = DiscardDataset(1, 10)
    dsit = iter(ds)
    next(dsit)
    print(next(dsit))
