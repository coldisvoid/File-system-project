#pragma once
#define _CRT_SECURE_NO_DEPRECATE
#pragma warning(disable:4996)
#include <stdio.h>
#include <memory.h>
#include <string>
#include <iostream>
#include <vector>
#include "bitmap.h"

using namespace std;
// 0 超级块 1inode位  2-14 block位  15 inode 16- data
/*常量设置*/
const char* FilePath = "theDisk";//模拟磁盘文件名
const int BlockSize = 1024;       //盘块大小
const int OPEN_MAX = 5;          //能打开最多的文件数
const int BlockCount = 102400;   //盘块数
const int DiskSize = BlockSize * BlockCount;    //磁盘大小
const int block_bitmap_count = 1 + BlockCount / 8 / BlockSize;//块位图占用块数量
const int inodeCount = 50;//inode数量
const int data_start = 16;//数据开始块 
FILE *fp;      //磁盘文件地址
string output = "";
char * BaseAddr;
string cmd;      //输入指令

//超级块
struct super_block {
	int s_inodes_count;      /* inodes 计数 */
	int s_blocks_count;      /* blocks 计数 */
	int s_log_block_size;    /* block 的大小 */
	void init() {
		s_inodes_count = inodeCount;
		s_blocks_count = BlockCount;
		s_log_block_size = BlockSize;
	}
};
//i节点位图
struct inode_bitmap {
	int bitmap[1 + inodeCount / BITS_PER_WORD];
	void init() {
		::memset(bitmap, 0, (1 + inodeCount / BITS_PER_WORD) * 4);
		for (int i = 0; i < inodeCount; i++)
			clr(i, bitmap);

	}
};
//块位图
struct block_bitmap {
	int bitmap[1 + BlockCount / BITS_PER_WORD];
	void init() {
		memset(bitmap, 0, (1 + BlockCount / BITS_PER_WORD) * 4);
		for (int i = 0; i < BlockCount; i++)
			clr(i, bitmap);
	}
};
//i节点
struct inode {
	int block = 0;//所在块
	char type = '-';//类型
	int Size = 0;//大小
	int Uid = 0;//
	int Inode = 0;//
	void init() {
		block = 0;
		Size = 0;
		type = '-';
		Uid = 0;
		Inode = 0;
	}
};
struct inodes {

	inode list[50];
	void init() {
		memset(list, 0, 50 * sizeof(inode));
		for (int i = 0; i < 50; i++) {
			list[i].init();
		}
	}
};
//文件夹
struct dir {
	int count = 0;
	char name[51][16];
	int inode[51];
	void init() {
		count = 0;
		memset(name, 0, 51 * 16);
		memset(inode, 0, 51 * 4);
	}
};
//数据
struct datafile {
	char data[1024];
	void init() {

		memset(data, 0, 1024);
	}
};


super_block * sb;//0 超级块
inode_bitmap * ib;//1 inode位图
block_bitmap *bb;//2-14  block位图
inodes * in;//15  inode

dir * di;//当前目录
int currentinode = 0;//当前路目录inode号
int userid;//当前用户
vector<int> path;//当前路径
vector<string> pathname;//当前路径


vector<int> userid_s;//存放所有用户id

vector < vector<string>> pathname_s;//存放所有用户的路径

//字符串分割
vector<string> split(const string& str, const string& delim) {
	vector<string> res;
	if ("" == str) return res;   	//先将要切割的字符串从string类型转换为char*类型   	
	char * strs = new char[str.length() + 1];
	strcpy(strs, str.c_str());
	char * d = new char[delim.length() + 1];
	strcpy(d, delim.c_str());
	char *p = strtok(strs, d);
	while (p) {
		string s = p; //分割得到的字符串转换为string类型   		
		res.push_back(s); //存入结果数组   		
		p = strtok(NULL, d);
	}
	return res;
}
//读块
void read_block(char *data, FILE *fp, int num, int count = 1) {
	fp = fopen(FilePath, "r");
	fseek(fp, num*BlockSize, SEEK_SET);
	fread(data, sizeof(char), BlockSize*count, fp);

	fclose(fp);

}
//写块
void write_block(char *data, FILE *fp, int num, int count = 1) {

	fp = fopen(FilePath, "r+");
	fseek(fp, num*BlockSize, SEEK_SET);
	BaseAddr = (char *)malloc(BlockSize*count);
	memset(BaseAddr, 0, BlockSize*count);
	fwrite(BaseAddr, sizeof(char), BlockSize*count, fp);
	fseek(fp, num*BlockSize, SEEK_SET);

	fwrite(data, sizeof(char), BlockSize*count, fp);
	fclose(fp);

}
//更改路径
void cd(char*name) {

	if (strcmp(name, "..") == 0) {
		if (path.size() != 0) {
			path.pop_back();
			pathname.pop_back();
			char* ww = (char *)malloc(BlockSize);
			if (path.size() == 0) {
				read_block(ww, fp, 16);
				di = (dir*)ww;
				currentinode = 0;
			}
			else {
				read_block(ww, fp, in->list[path[path.size() - 1]].block);
				di = (dir*)ww;
				currentinode = path[path.size() - 1];
			}

		}
	}
	else {
		for (int i = 0; i < di->count; i++) {
			if (strcmp(name, di->name[i]) == 0 && in->list[di->inode[i]].type == 'd') {
				currentinode = di->inode[i];
				path.push_back(currentinode);
				string nam = name;
				pathname.push_back(nam);
				int blockid = in->list[currentinode].block;
				char *ww = (char *)malloc(BlockSize);
				read_block(ww, fp, blockid);
				dir * dinew = (dir*)ww;
				di = dinew;
				break;
			}
		}
	}

}
int changed(string r1) {
	//改变目录
	char  name[16];
	vector<string>dirs = split(r1, "/");

	if (r1[0] == '/') {
		vector<string>save;
		for (int i = 0; i < pathname.size(); i++) {
			save.push_back(pathname[i]);
		}
		for (int i = 0; i < save.size(); i++) {
			strcpy(name, "..");
			cd(name);
		}
		for (int i = 0; i < dirs.size(); i++) {
			strcpy(name, dirs[i].c_str());
			int ps = path.size();
			cd(name);
			if (path.size() != ps + 1) {
				output += "changing directory failed\n";
				for (int j = 0; j < i; j++) {
					strcpy(name, "..");
					cd(name);
				}
				for (int i = 0; i < save.size(); i++) {
					strcpy(name, save[i].c_str());
					cd(name);
				}
				return 0;
				break;
			}
		}

	}

	else if (r1 == "..") {
		strcpy(name, "..");
		cd(name);
	}
	else {
		for (int i = 0; i < dirs.size(); i++) {
			strcpy(name, dirs[i].c_str());
			int ps = path.size();
			cd(name);
			if (path.size() != ps + 1) {
				output += "changing directory failed\n";
				for (int j = 0; j < i; j++) {
					strcpy(name, "..");
					cd(name);
				}
				return 0;
			}
		}
	}
	return 1;
}
//创建文件
int newfile(char*name) {//-1重名
	int inodeid = 0;
	int blockid = 0;
	char *ww;
	for (int i = 0; i < di->count; i++) {
		if (strcmp(name, di->name[i]) == 0) {
			if (in->list[di->inode[i]].type == '-') {
				if (in->list[di->inode[i]].Uid != userid) {
					blockid = -1;
					output += "has a same name file with no permission\n";
					goto end1;
				}
				else {
					blockid = in->list[di->inode[i]].block;
					output += "has a same name file\n";
					goto end1;
				}

			}
			
			
		}
	}
	for (int i = 0; i < inodeCount; i++) {
		if (test(i, ib->bitmap) == 0) {
			inodeid = i;
			break;
		}
	}
	for (int i = 16; i < BlockCount; i++) {
		if (test(i, bb->bitmap) == 0) {
			blockid = i;
			break;
		}
	}
	set(inodeid, ib->bitmap);
	set(blockid, bb->bitmap);
	in->list[inodeid].type = '-';
	in->list[inodeid].block = blockid;
	in->list[inodeid].Uid = userid;
	di->inode[di->count] = inodeid;
	strcpy(di->name[di->count], name);
	di->count = di->count + 1;

	write_block((char*)sb, fp, 0);
	write_block((char*)ib, fp, 1);
	write_block((char*)bb, fp, 2, 13);
	write_block((char*)in, fp, 15);
	write_block((char*)di, fp, in->list[currentinode].block);
	ww = (char *)malloc(BlockSize);
	memset(ww, 0, BlockSize);
	write_block(ww, fp, blockid);
end1:cout << endl;
	return blockid;
}
//删除文件
void del(char*name,int per=0) {
	int inodeid = 0;
	int blockid = 0;
	for (int i = 0; i < di->count; i++) {
		if (strcmp(name, di->name[i]) == 0) {
			if (per) {
				if (in->list[di->inode[i]].Uid == userid) {

				}
				else {
					output += "no permission\n";
					return;
				}
			}
			clr(di->inode[i], ib->bitmap);
			clr(in->list[di->inode[i]].block, bb->bitmap);
			if (in->list[di->inode[i]].type == '-') {
				for (int j = i; j < di->count - 1; j++) {
					di->inode[j] = di->inode[j + 1];
					strcpy(di->name[j], di->name[j + 1]);

				}di->count -= 1;
				//
				write_block((char*)sb, fp, 0);
				write_block((char*)ib, fp, 1);
				write_block((char*)bb, fp, 2, 13);
				write_block((char*)in, fp, 15);
				write_block((char*)di, fp, in->list[currentinode].block);
				break;
			}

		}
	}
}
//创建目录
void md(char*name) {
	int inodeid = 0;
	int blockid = 0;
	char *ww;
	dir * dinew;
	dir *diz;
	for (int i = 0; i < di->count; i++) {
		if (strcmp(name, di->name[i]) == 0) {
			output += "already has a directory file with same name\n";
			goto end;
		}
	}
	for (int i = 0; i < inodeCount; i++) {
		if (test(i, ib->bitmap) == 0) {
			inodeid = i;
			break;
		}
	}
	for (int i = 16; i < BlockCount; i++) {
		if (test(i, bb->bitmap) == 0) {
			blockid = i;
			break;
		}
	}
	set(inodeid, ib->bitmap);
	set(blockid, bb->bitmap);
	in->list[inodeid].type = 'd';
	in->list[inodeid].Uid = userid;
	in->list[inodeid].block = blockid;
	di->inode[di->count] = inodeid;
	strcpy(di->name[di->count], name);

	di->count = di->count + 1;
	//
	ww = (char *)malloc(BlockSize);
	read_block(ww, fp, 16);
	diz = (dir*)ww;

	write_block((char*)sb, fp, 0);

	write_block((char*)ib, fp, 1);

	write_block((char*)bb, fp, 2, 13);
	//***********************************************************

	write_block((char*)in, fp, 15);

	write_block((char*)di, fp, in->list[currentinode].block);

	ww = (char *)malloc(BlockSize);
	dinew = (dir*)ww;
	dinew->init();
	write_block((char*)ww, fp, blockid);

end:cout << endl;
}
//删除目录
void rd(char*name) {
	int inodeid = 0;
	int blockid = 0;
	for (int i = 0; i < di->count; i++) {
		if (strcmp(name, di->name[i]) == 0) {
			clr(di->inode[i], ib->bitmap);
			clr(in->list[di->inode[i]].block, bb->bitmap);
			if (in->list[di->inode[i]].type == 'd') {
				for (int j = i; j < di->count - 1; j++) {
					di->inode[j] = di->inode[j + 1];
					strcpy(di->name[j], di->name[j + 1]);

				}di->count = di->count - 1;
				//
				write_block((char*)sb, fp, 0);
				write_block((char*)ib, fp, 1);
				write_block((char*)bb, fp, 2, 13);
				write_block((char*)in, fp, 15);
				write_block((char*)di, fp, in->list[currentinode].block);
				break;
			}


		}
	}
}
//清除目录下文件
void clear() {
	for (int i = 0; i < di->count; i++) {
		if (in->list[di->inode[i]].type == 'd') {
			cd(di->name[i]);
			clear();
			char back[3] = "..";
			cd(back);
			rd(di->name[i]);
		}
		else if (in->list[di->inode[i]].type == '-') {
			del(di->name[i]);
		}
	}
}

void init() {
	if ((fp = fopen(FilePath, "r")) != NULL) {
		//读取超级块
		char *ww = (char *)malloc(BlockSize);
		read_block(ww, fp, 0);
		sb = (super_block*)ww;
		ww = (char *)malloc(BlockSize);
		read_block(ww, fp, 1);
		ib = (inode_bitmap*)ww;
		ww = (char *)malloc(BlockSize*block_bitmap_count);
		ww = (char *)malloc(BlockSize*block_bitmap_count);
		read_block(ww, fp, 2, block_bitmap_count);
		bb = (block_bitmap*)ww;
		ww = (char *)malloc(BlockSize);
		read_block(ww, fp, 15);
		in = (inodes*)ww;
		ww = (char *)malloc(BlockSize);
		read_block(ww, fp, 16);
		di = (dir*)ww;
		cout << endl;
	}
	else {

		//创建盘
		BaseAddr = (char *)malloc(DiskSize);
		fp = fopen(FilePath, "w");

		fwrite(BaseAddr, sizeof(char), DiskSize, fp);

		fclose(fp);
		//写入超级块
		char *ww = (char *)malloc(BlockSize);
		memset(ww, 0, BlockSize);
		super_block *a = (super_block*)ww;
		a->init();
		write_block(ww, fp, 0);
		sb = a;
		//写入inode_bitmap
		ww = (char *)malloc(BlockSize);
		memset(ww, 0, BlockSize);
		inode_bitmap *b = (inode_bitmap*)ww;
		b->init();
		write_block(ww, fp, 1);
		ib = b;
		//写入block_bitmap
		ww = (char *)malloc(BlockSize*block_bitmap_count);
		memset(ww, 0, BlockSize*block_bitmap_count);
		block_bitmap *c = (block_bitmap*)ww;
		c->init();
		write_block(ww, fp, 2, block_bitmap_count);
		bb = c;
		//写入inode
		ww = (char *)malloc(BlockSize);
		memset(ww, 0, BlockSize);
		inodes *d;
		d = (inodes*)ww;
		d->init();
		d->list[0].block = 16;
		d->list[0].type = 'd';
		write_block((char*)d, fp, 15);
		//设置in
		in = d;
		//写入ib
		set(0, ib->bitmap);
		write_block((char*)ib, fp, 1);
		//写入bb
		set(16, bb->bitmap);
		write_block(ww, fp, 2, block_bitmap_count);
		//写入data
		ww = (char *)malloc(BlockSize);
		memset(ww, 0, BlockSize);
		di = (dir*)ww;
		di->init();
		write_block(ww, fp, 16);
	}

}
//打印目录
void showd() {
	for (int i = 0; i < di->count; i++) {

		output += di->name[i];
		output += "   ";
		output += in->list[di->inode[i]].type;
		output += "   uid：";
		output += to_string(in->list[di->inode[i]].Uid);
		output += "\n";

	}
}
//打开文件
void cat(char* name) {
	for (int i = 0; i < di->count; i++) {// && in->list[di->inode[i]].type == '-'
		if (strcmp(di->name[i], name) == 0) {

			char *ww = (char *)malloc(BlockSize);
			memset(ww, 0, BlockSize);
			read_block(ww, fp, in->list[di->inode[i]].block);
			datafile* a = (datafile*)ww;

			output += ww;
			output += '\n';
			return;
			//cout << endl;
		}
	}
	output += "failed to find file\n";
}

//切换目录
void changeuser() {
	string home = "/";

	changed(home);

	for (int i = 0; i < userid_s.size(); i++) {
		if (userid_s[i] == userid) {
			vector<string> lis = pathname_s[i];
			for (int j = 0; j < lis.size(); j++) {
				changed(lis[j]);
			}

			return;
		}
	}
	userid_s.push_back(userid);
	pathname_s.push_back(pathname);

}
//保存路径
void saved() {

	for (int i = 0; i < userid_s.size(); i++) {
		if (userid_s[i] == userid) {
			pathname_s[i] = pathname;
			return;
		}
	}

}
