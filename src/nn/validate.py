import torch
import time
import copy
import sys

from resnet34 import DiscardNet
from resnet34_128 import DiscardNet_34_128
from resnet50_64 import DiscardNet_50_64


def validate_model(model, dataloaders, device):
    since = time.time()

    print('Model Validation')
    print('-' * 10)

    model.eval()

    running_corrects = 0
    ds_sizes = {'val': 0}
    intermidiate = False

    batch_i = 0
    dataloaders['val'].init()
    while dataloaders['val'].is_valid():
        batch_i += 1
        inputs, labels = dataloaders['val'].next()
        ds_sizes['val'] += labels.size(0)
        inputs = inputs.to(device)
        labels = labels.to(device)

        with torch.set_grad_enabled(False):
            outputs = model(inputs)
            _, preds = torch.max(outputs, 1)

        running_corrects += torch.sum(preds == labels.data)

        if batch_i % 20 == 0:
            intermidiate = True
            temp_acc = running_corrects.double() / ds_sizes['val']
            print('{} batches Acc: {:.4f}'.format(
                'val', temp_acc))
            time_elapsed = time.time() - since
            print('Time elapsed: {:.0f}m {:.0f}s'.format(
                time_elapsed//60, time_elapsed % 60))

    best_acc = float(running_corrects) / ds_sizes['val']

    if intermidiate:
        print('-' * 10)

    time_elapsed = time.time() - since
    print('Validation complete in {:.0f}m {:.0f}s'.format(
        time_elapsed//60, time_elapsed % 60))
    print('Validation Acc: {:.4f}'.format(best_acc))

    return model


if __name__ == '__main__':
    torch.classes.load_library("txt2batch.so")

    dataloaders = {}
    dataloaders['val'] = torch.classes.mahjong.Loader(1)

    if len(sys.argv) == 2:
        if sys.argv[1] == 'discard_34_128':
            net = DiscardNet_34_128()
            PATH = "./model/discard_34_128.pyt"
        elif sys.argv[1] == 'discard_50_64':
            net = DiscardNet_50_64()
            PATH = "./model/discard_50_64.pyt"
        else:
            net = DiscardNet()
            PATH = "./model/discard.pyt"
    else:
        net = DiscardNet()
        PATH = "./model/discard.pyt"

    print(PATH)

    if torch.cuda.is_available():
        device = torch.device("cuda:0")
        print("Infering on GPU!!")
    else:
        device = torch.device("cpu")
    net = net.to(device)
    net.load_state_dict(torch.load(PATH))
    net = validate_model(net, dataloaders, device)
