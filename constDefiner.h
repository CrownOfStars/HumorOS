#pragma once
#include<deque>
#include<string>
#include<fstream>
#include<sstream>
typedef char Byte;

using File = char[1024];
using ll = long long;

//尺寸常量
// 超级块位于第一个块 size:1k
constexpr int inodeStart = 1;
constexpr int inodeSize = 1024;//i结点区大小为 16*1024*64/1024 1024k
constexpr int fileBmStart = 1025;//位图区起始点
constexpr int fileBmSize = 16;//文件块位图 128*1024/(1024*8) 16k
constexpr int inodeBmStart = 1041;
constexpr int inodeBmSize = 2;
constexpr int fileStart = 1043;

constexpr bool fileType = true;
constexpr bool inodeType = false;

constexpr bool dir_Inode = false;
constexpr bool file_Inode = true;

std::string userName = "shu";

unsigned char bitQuick[8]{
	unsigned char(0b10000000),
	unsigned char(0b11000000),
	unsigned char(0b11100000),
	unsigned char(0b11110000),
	unsigned char(0b11111000),
	unsigned char(0b11111100),
	unsigned char(0b11111110),
	unsigned char(0b11111111)
};

std::deque<std::pair<std::string, int>> pathDeque;//保存从根目录到当前目录之间的目录

int lastFreeFile = 0;//表示第一个位图所在的Byte

int lastFreeInode = 0;//表示第一个位图所在的Byte

char* emptyInode = new char[64];

int* rmFileids = new int[10];//移除的文件列表
int* rmInodeids = new int[10];//移除的inode列表

int rmFileNum = 0;
int rmInodeNum = 0;

size_t rmFileCap = 10;//移除的文件列表容量
size_t rmInodeCap = 10;//移除的inode列表容量

std::string dec2hex(int i)
{
	std::stringstream ioss;     //定义字符串流
	std::string s_temp;         //存放转化后字符
	ioss << std::hex << i;      //以十六制形式输出
	ioss >> s_temp;

	if (5 > s_temp.size())
	{
		std::string s_0(5 - s_temp.size(), '0');      //位数不够则补0
		s_temp = s_0 + s_temp;                            //合并
	}

	std::string s = s_temp.substr(s_temp.length() - 5, s_temp.length());    //取右width位
	return "0x" + s;
}