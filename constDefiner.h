#pragma once
#include<vector>
#include<string>
typedef char Byte;

using File = char[1024];

File* disk = nullptr;//操作磁盘的主要结构
File* inode_head = nullptr;//i结点的起点
File* fileBitmap_head;//文件位示图的起点
File* inodeBitmap_head;//i结点位示图的起点
File* file_head = nullptr;//文件（夹）区起点

//尺寸常量
// 超级块位于第一个块 size:1k
constexpr int inodeSize = 1024;//i结点区大小为 16*1024*64/1024 1024k
constexpr int blockMapStart = 1025;//位图区起始点
constexpr int blockBitmapSize = 13;//文件块位图 100*1024/(1024*8) 12.5k ~ 13k
constexpr int inodeMapStart = 1038;
constexpr int inodeBitmapSize = 2;//i结点位图 16*1024/（1024*8） 2k
constexpr int fileStart = 1040;
constexpr int fileSize = 101360;//dir file region 100*1024 - 1040

//数量常量
constexpr int iNodeNum = 16 * 1024;
constexpr int blockNum = 100 * 1024;

//字符串
std::vector<std::string> argv;//命令解释器解析出的参数
std::vector<std::string> curPath;//保存当前所在的路径

enum class BitmapOp {
    ReadOnly, // 0
    SetTrue, // 1
    SetFalse,
    Reverse
};