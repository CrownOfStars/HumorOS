#pragma once
#include<deque>
#include<string>
#include<fstream>
#include<sstream>
typedef char Byte;

using File = char[1024];
using ll = long long;

//�ߴ糣��
// ������λ�ڵ�һ���� size:1k
constexpr int inodeStart = 1;
constexpr int inodeSize = 1024;//i�������СΪ 16*1024*64/1024 1024k
constexpr int fileBmStart = 1025;//λͼ����ʼ��
constexpr int fileBmSize = 13;//�ļ���λͼ 100*1024/(1024*8) 12.5k ~ 13k
constexpr int inodeBmStart = 1038;
constexpr int inodeBmSize = 2;
constexpr int fileStart = 1040;
constexpr int fileSize = 101360;//dir file region 100*1024 - 1040

constexpr bool fileBm = true;
constexpr bool inodeBm = false;

std::string userName = "shu";

//��������
constexpr int iNodeNum = 16 * 1024;
constexpr int blockNum = 100 * 1024;


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

std::deque<std::pair<std::string, int>> pathDeque;//����Ӹ�Ŀ¼����ǰĿ¼֮���Ŀ¼

int lastFreeFile = 2;

int lastFreeInode = 1;

std::string dec2hex(int i)
{
	std::stringstream ioss;     //�����ַ�����
	std::string s_temp;         //���ת�����ַ�
	ioss << std::hex << i;      //��ʮ������ʽ���
	ioss >> s_temp;

	if (5 > s_temp.size())
	{
		std::string s_0(5 - s_temp.size(), '0');      //λ��������0
		s_temp = s_0 + s_temp;                            //�ϲ�
	}

	std::string s = s_temp.substr(s_temp.length() - 5, s_temp.length());    //ȡ��widthλ
	return "0x" + s;
}

std::string fileSizeAlign(int fileSize)
{
	std::string value;
	return value;
	//TODO:���ļ��Ĵ�С�������������\t�ĸ���
}