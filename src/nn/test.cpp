#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main ()
{
    FILE * pFile;
    long lSize;
    char * buffer;
    size_t result;
    
    /* 若要一个byte不漏地读入整个文件，只能采用二进制方式打开 */ 
    pFile = fopen ("test.txt", "rb" );
    if (pFile==NULL)
    {
        fputs ("File error",stderr);
        exit (1);
    }
 
    /* 获取文件大小 */
    fseek (pFile , 0 , SEEK_END);
    lSize = ftell (pFile);
    rewind (pFile);
 
    /* 分配内存存储整个文件 */ 
    buffer = (char*) malloc (sizeof(char)*lSize);
    if (buffer == NULL)
    {
        fputs ("Memory error",stderr); 
        exit (2);
    }
 
    /* 将文件拷贝到buffer中 */
    result = fread (buffer,1,lSize,pFile);
    if (result != lSize)
    {
        fputs ("Reading error",stderr);
        exit (3);
    }
    /* 现在整个文件已经在buffer中，可由标准输出打印内容 */
    char dapai[] = "打牌";
    char *p, *tmp;
    p = strtok(buffer, "\t");
    while(p!=NULL) {
        if(strcmp(p , dapai)==0)
            printf("%s\n", p);
        p =strtok(NULL, "\t");
    }
    // printf("%s", buffer); 
 
    /* 结束演示，关闭文件并释放内存 */
    fclose (pFile);
    free (buffer);
    return 0;
}