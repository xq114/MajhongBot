#include <cstdio>
#include <string>
#include <torch/script.h>
#include <vector>

#include "txt2batch.h"
// #include "statemachine.h"

Loader::Loader(int64_t mode) : i{0}, len{0} {
    ftable = fopen(files[mode], "r");
    while (fgets(buff, 50, ftable) != NULL)
        ++len;
    fseek(ftable, 0, SEEK_SET);
    fgets(buff, 50, ftable);
}

Loader::~Loader() { fclose(ftable); }

c10::intrusive_ptr<Loader> Loader::clone() const {
    return c10::make_intrusive<Loader>(mode);
}

int64_t Loader::length() { return len; }

constexpr char dapai[] = "打牌";


std::vector<torch::Tensor> Loader::next() {
    current = fopen(buff, "r");
    long lSize;
    char * buffer;
    size_t result;
    
    /* 若要一个byte不漏地读入整个文件，只能采用二进制方式打开 */ 
    current = fopen ("test.txt", "rb" );
    if (current==NULL)
    {
        fputs ("File error",stderr);
        exit (1);
    }
 
    /* 获取文件大小 */
    fseek (current , 0 , SEEK_END);
    lSize = ftell (current);
    rewind (current);
 
    /* 分配内存存储整个文件 */ 
    buffer = (char*) malloc (sizeof(char)*lSize);
    if (buffer == NULL)
    {
        fputs ("Memory error",stderr); 
        exit (2);
    }
 
    /* 将文件拷贝到buffer中 */
    result = fread (buffer,1,lSize,current);
    if (result != lSize)
    {
        fputs ("Reading error",stderr);
        exit (3);
    }
    fclose(current);

    /* 统计打牌操作出现次数 */
    int L = 0, i=0;
    char *p, *tmp;
    p = strtok(buffer, "\t");
    while(p!=NULL) {
        if(strcmp(p , dapai)==0)
            ++L;
        p =strtok(NULL, "\t");
    }

    torch::Tensor inputs=torch::empty({L, 150, 4, 34});
    torch::Tensor labels=torch::empty({L});

    // TODO: finish the parse
    fgets(buff, 50, ftable);
    return std::vector<torch::Tensor>{inputs.clone(), labels.clone()};
}

static auto testLoader = torch::class_<Loader>("mahjong", "Loader")
                             .def(torch::init<int64_t>())
                             .def("length", &Loader::length)
                             .def("next", &Loader::next);