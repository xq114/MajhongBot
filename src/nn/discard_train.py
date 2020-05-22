import torch
import torch.nn as nn
import torch.optim as optim
import time
import copy
import sys

from resnet34 import DiscardNet


PATH = "./model/discard.pyt"


def train_model(model, dataloaders, criterion, optimizer, scheduler, device, num_epochs=20):
    since = time.time()

    best_model_wts = copy.deepcopy(model.state_dict())
    best_acc = 0.0

    for epoch in range(num_epochs):
        print('Epoch {}/{}'.format(epoch, num_epochs-1))
        print('-' * 10)

        for phase in ['train', 'val']:
            if phase == 'train':
                model.train()
            elif phase == 'val':
                model.eval()

            running_loss = 0.0
            running_corrects = 0
            ds_sizes = {'train': 0, 'val': 0}
            intermidiate = False

            batch_i = 0
            dataloaders[phase].init()
            while dataloaders[phase].is_valid():
                batch_i += 1
                # try:
                inputs, labels = dataloaders[phase].next()
                # except:
                #     print("Error file index:", batch_i)
                ds_sizes[phase] += labels.size(0)
                inputs = inputs.to(device)
                labels = labels.to(device)

                optimizer.zero_grad()
                with torch.set_grad_enabled(phase == 'train'):
                    outputs = model(inputs)
                    _, preds = torch.max(outputs, 1)
                    loss = criterion(outputs, labels)

                    if phase == 'train':
                        loss.backward()
                        optimizer.step()

                running_loss += loss.item() * inputs.size(0)
                running_corrects += torch.sum(preds == labels.data)

                if batch_i % 20 == 0 and phase == 'train':
                    intermidiate = True
                    temp_loss = running_loss / ds_sizes[phase]
                    temp_acc = running_corrects.double() / ds_sizes[phase]
                    print('{} batches Loss: {:.4f} Acc: {:.4f}'.format(
                        phase, temp_loss, temp_acc))
                    time_elapsed = time.time() - since
                    print('Time elapsed: {:.0f}m {:.0f}s'.format(
                        time_elapsed//60, time_elapsed % 60))

            if phase == 'train':
                scheduler.step()

            epoch_loss = running_loss / ds_sizes[phase]
            epoch_acc = float(running_corrects) / ds_sizes[phase]

            if intermidiate:
                print('-' * 10)
            print('{} Loss: {:.4f} Acc: {:.4f}'.format(
                phase, epoch_loss, epoch_acc))
            time_elapsed = time.time() - since
            print('Time elapsed: {:.0f}m {:.0f}s'.format(
                time_elapsed//60, time_elapsed % 60))

            if phase == 'val' and epoch_acc > best_acc:
                best_acc = epoch_acc
                best_model_wts = copy.deepcopy(model.state_dict())

        print()

    time_elapsed = time.time() - since
    print('Training complete in {:.0f}m {:.0f}s'.format(
        time_elapsed//60, time_elapsed % 60))
    print('Best val Acc: {:.4f}'.format(best_acc))

    model.load_state_dict(best_model_wts)
    return model


if __name__ == '__main__':
    num_epochs = 2
    if len(sys.argv) == 2:
        num_epochs = int(sys.argv[1])
    torch.classes.load_library("txt2batch.so")

    dataloaders = {}
    dataloaders['train'] = torch.classes.mahjong.Loader(0)
    dataloaders['val'] = torch.classes.mahjong.Loader(1)
    net = DiscardNet()

    criterion = nn.CrossEntropyLoss()
    optimizer = optim.Adam(net.parameters(), lr=0.1)
    exp_lr_scheduler = optim.lr_scheduler.CosineAnnealingLR(
        optimizer, T_max=64, eta_min=1e-5)

    if torch.cuda.is_available():
        device = torch.device("cuda:0")
        print("Training on GPU!!")
    else:
        device = torch.device("cpu")
    net = net.to(device)
    net.load_state_dict(torch.load(PATH))
    net = train_model(net, dataloaders, criterion, optimizer,
                      exp_lr_scheduler, device, num_epochs=num_epochs)
    torch.save(net.state_dict(), PATH)
