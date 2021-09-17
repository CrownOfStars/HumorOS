#pragma once
#include<vector>
#include<string>
typedef char Byte;

using File = char[1024];

File* disk = nullptr;//�������̵���Ҫ�ṹ
File* inode_head = nullptr;//i�������
File* fileBitmap_head;//�ļ�λʾͼ�����
File* inodeBitmap_head;//i���λʾͼ�����
File* file_head = nullptr;//�ļ����У������

//�ߴ糣��
// ������λ�ڵ�һ���� size:1k
constexpr int inodeSize = 1024;//i�������СΪ 16*1024*64/1024 1024k
constexpr int blockMapStart = 1025;//λͼ����ʼ��
constexpr int blockBitmapSize = 13;//�ļ���λͼ 100*1024/(1024*8) 12.5k ~ 13k
constexpr int inodeMapStart = 1038;
constexpr int inodeBitmapSize = 2;//i���λͼ 16*1024/��1024*8�� 2k
constexpr int fileStart = 1040;
constexpr int fileSize = 101360;//dir file region 100*1024 - 1040

//��������
constexpr int iNodeNum = 16 * 1024;
constexpr int blockNum = 100 * 1024;

//�ַ���
std::vector<std::string> argv;//����������������Ĳ���
std::vector<std::string> curPath;//���浱ǰ���ڵ�·��

enum class BitmapOp {
    ReadOnly, // 0
    SetTrue, // 1
    SetFalse,
    Reverse
};