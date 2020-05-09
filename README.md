# MajhongBot
第12届北京大学游戏对抗赛麻将AI

# 第三方库说明
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