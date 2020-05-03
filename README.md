# MajhongBot
第12届北京大学游戏对抗赛麻将AI

# 编译说明
提交要求输出单文件，输出到build/fbot.cpp（目前单文件不需要，直接提交bot.cpp即可）

编译输出src/bot可执行文件（自动探测使用clang++/g++）：

```shell
cd src
make
```

amalgamation fbot.cpp：

```shell
cd src
make amal
```

清理输出文件：

```shell
cd src
make clean
```