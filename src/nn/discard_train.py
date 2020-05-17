import torch
import torch.nn as nn
import torch.optim as optim
import time
import copy

from resnet34 import DiscardNet


PATH = "./model/discard.pyt"


def train_model(model, dataloaders, criterion, optimizer, scheduler, num_epochs=20):
    since = time.time()
    if torch.cuda.is_available():
        device = torch.device("cuda:0")
        print("Training on GPU!!")
    else:
        device = torch.device("cpu")

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

            for inputs, labels in dataloaders[phase]:
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
            
            if phase == 'train':
                scheduler.step()
            
            epoch_loss = running_loss / dataset_sizes[phase]
            epoch_acc = running_corrects.double() / dataset_sizes[phase]

            print('{} Loss: {:.4f} Acc: {:.4f}'.format(phase, epoch_loss, epoch_acc))
            time_elapsed = time.time() - since
            print('Time elapsed: {:.0f}m {:.0f}s'.format(time_elapsed//60, time_elapsed%60))

            if phase == 'val' and epoch_acc > best_acc:
                best_acc = epoch_acc
                best_model_wts = copy.deepcopy(model.state_dict())
        
        print()
    
    time_elapsed = time.time() - since
    print('Training complete in {:.0f}m {:.0f}s'.format(time_elapsed//60, time_elapsed%60))
    print('Best val Acc: {:.4f}'.format(best_acc))

    model.load_state_dict(best_model_wts)
    return model

if __name__ == '__main__':
    dataloaders = {}
    # TODO: finish dataloaders
    # dataloaders['train'] = ?
    # dataloaders['val'] = ?
    net = DiscardNet()

    criterion = nn.CrossEntropyLoss()
    optimizer = optim.SGD(net.parameters(), lr=0.001, momentum=0.9)
    exp_lr_scheduler = optim.lr_scheduler.StepLR(optimizer, step_size=7, gamma=0.1)

    net.load_state_dict(torch.load(PATH))
    net = train_model(net, dataloaders, criterion, optimizer, exp_lr_scheduler)
    torch.save(net.state_dict(), PATH)
