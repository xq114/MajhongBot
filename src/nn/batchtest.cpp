#include "txt2batch.h"
#include <iostream>
#include <stdexcept>
using namespace std;
using namespace torch::indexing;

int main() {
    Loader ld(0);
    int i = 0;
    ld.init();
    while (ld.is_valid()) {
        ++i;
        try {
            cout << i << endl;
            ld.next();
        } catch (exception e) {
            cout << "Error file index: " << i << endl;
            cout << e.what() << endl;
        }
    }
    return 0;
}