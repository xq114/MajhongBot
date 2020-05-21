#include "txt2batch.h"
#include <iostream>
using namespace std;
using namespace torch::indexing;

int main() {
    Loader ld(1);
    int i = 0;
    ld.init();
    while (ld.is_valid()) {
        ld.next();
        ++i;
    }
    return 0;
}