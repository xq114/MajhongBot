# MajhongBot
第12届北京大学游戏对抗赛麻将AI

## 目录及第三方库说明
主要源码和测试数据放在src目录下；

用到麻将的算番库，源码依赖，放在utils目录下；

用到jsoncpp库，可以使用包管理器安装；

用到libtorch库，安装方法：

```shell
# linux
cd /usr/local/src
wget https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-1.5.0%2Bcpu.zip
unzip libtorch-cxx11-abi-shared-with-deps-1.5.0%2Bcpu.zip
```

![Mac OS 预编译包](https://download.pytorch.org/libtorch/cpu/libtorch-macos-1.5.0.zip)，解压到/usr/local/src即可（未测试）

## 注释说明
需要进一步工作的地方使用`// TODO:xxx`注释，可以搜索代码或使用编辑器插件高亮此注释

函数的注释建议使用C/C++/Java系列格式，可以用编辑器插件高亮
```cpp
/**
 * @brief Brief introduction.
 *
 * @param parameter1
 * @param [out] parameter2 (serve as output, usually a pointer/reference)
 * @retval Variable to be returned.
 */
```

## 代码规范
遵从`.clang-format`文件指示，兼容clang-format9.0+

使用方法：
```shell
clang-format -i src/bot.cpp
```

建议使用代码检查工具（如VS code自带、Clang Static Analyzer、CCLS等）检查后再提交，不要在代码中出现大量的低级拼写错误！

## 编译说明
提交要求输出单文件，输出到build/fbot.cpp（目前单文件不需要，直接提交bot.cpp即可）

编译输出src/bot可执行文件（自动探测使用clang++/g++）：

```shell
cd src
make
```

清理输出文件：

```shell
cd src
make clean
```

amalgamation fbot.cpp：(Windows only)

```shell
cd src
make amal
```

## 代码统计
```
cloc . --fullpath --not-match-d=./nn/data/
      11 text files.
      11 unique files.
      13 files ignored.

github.com/AlDanial/cloc v 1.84  T=5.23 s (2.1 files/s, 277.9 lines/s)
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
C++                              5             79             75            926
Python                           3             57             29            203
make                             1             18              0             46
JSON                             1              0              0             12
C/C++ Header                     1              3              0              5
-------------------------------------------------------------------------------
SUM:                            11            157            104           1192
-------------------------------------------------------------------------------
```