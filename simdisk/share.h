#pragma once
#include "main.h"
#include <windows.h>
#include <process.h>
#include <mutex>
#define BUF_SIZE 2048
wstring   m_cstrSzName[2];//共享内存名
wstring   m_ReadSemaphoreName[2];//读数据的信号量名
wstring   m_WriteSemaphoreName[2];//写数据的信号量名
HANDLE  m_hMapFile[2];//共享内存句柄
char    *m_pBuf[2];//读取数据的缓存
HANDLE	m_Read[2];//读数据的信号量句柄
HANDLE  m_Write[2];//写数据的信号量句柄
//控制读写
int read = 0;
int write = 1;
void CloseFileMap()
{
	for (int i = 0; i < 2; i++)
	{
		UnmapViewOfFile(m_pBuf[i]);
		CloseHandle(m_hMapFile[i]);
	}
}
void CreateFileMap()
{
	m_cstrSzName[0] = L"NameOfMappingObject0";//共享内存0的名字
	m_cstrSzName[1] = L"NameOfMappingObject1";//共享内存1的名字
	m_ReadSemaphoreName[0] = L"ReadSemaphoreName0";//读信号量0的名字
	m_ReadSemaphoreName[1] = L"ReadSemaphoreName1";//读信号量1的名字
	m_WriteSemaphoreName[0] = L"WriteSemaphoreName0";//写信号量0的名字
	m_WriteSemaphoreName[1] = L"WriteSemaphoreName1";//写信号量1的名字
	for (int i = 0; i < 2; i++)
	{
		//创建共享文件句柄 hMapFile
		m_hMapFile[i] = CreateFileMapping(
			INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE,
			0,
			BUF_SIZE,
			m_cstrSzName[i].c_str()
		);
		m_pBuf[i] = (char*)MapViewOfFile(
			m_hMapFile[i],				  //共享内存的句柄
			FILE_MAP_ALL_ACCESS,      //可读写
			0,
			0,
			BUF_SIZE
		);
	}
	m_Read[0] = CreateSemaphore(NULL, 0, 1, m_ReadSemaphoreName[0].c_str());
	m_Write[0] = CreateSemaphore(NULL, 1, 1, m_WriteSemaphoreName[0].c_str());
	m_Read[1] = CreateSemaphore(NULL, 0, 1, m_ReadSemaphoreName[1].c_str());
	m_Write[1] = CreateSemaphore(NULL, 1, 1, m_WriteSemaphoreName[1].c_str());
}
unsigned int __stdcall WriteSharedData(void *pPM)
{
	//将数据放到共享内存
	while (true)
	{
		if (read == 1) {
			//通过m_pBuf[0]向共享内存m_hMapFile[0]发送数据
			WaitForSingleObject(m_Write[0], INFINITE);

			char szInfo[BUF_SIZE] = { 0 };
			char back[1024];
			output += "\n/";
			for (int i = 0; i < pathname.size(); i++) {
				output += pathname[i];
				output += "/";
			}
			//将string output转换为 char* 后写入
			strcpy(back, output.c_str());
			memcpy(m_pBuf[0], back, BUF_SIZE - 1);
			ReleaseSemaphore(m_Read[0], 1, NULL);
			output = "";

			read = 0;
			write = 1;
		}


	}
	return true;
}
unsigned int __stdcall ReadSharedData(void *pPM)
{
	//3.从共享内存中读取数据
	while (true)
	{
		if (write == 1) {
			WaitForSingleObject(m_Read[1], INFINITE);//m_Read[1]信号量由程序B增加
			//读取后分割字符串提取命令，参数，和用户id
			string temp = (string)m_pBuf[1];
			vector<string> receive = split(temp, " ");
			if (temp == "") {
				goto out;
			}
			userid = atoi(receive[receive.size() - 1].c_str());
			changeuser();
			cmd = receive[0];
			if (cmd == "info") {
				output = output + "info:  显示整个系统信息\n"
					+ "cd … : 改变目录\n"
					+ "dir … : 显示目录\n"
					+ "md … : 创建目录\n"
					+ "rd … : 删除目录\n"
					+ "newfile … : 建立文件\n"
					+ "cat … : 读取文件\n"
					+ "copy … : 拷贝文件\n";
			}
			else if (cmd == "cd") {
				//切换目录
				int result = changed(receive[1]);
				if (result) {
					//cout << "success" << endl;
				}
				else {
					//cout << "fail" << endl;
				}
			}
			else if (cmd == "dir") {
				//打印当前路径下文件
				if (receive.size() == 3) {
					string r1 = receive[1];
					showd();
					if (r1 == "/s") {

						for (int i = 0; i < di->count; i++) {
							if (in->list[di->inode[i]].type == 'd') {
								char *name = di->name[i];
								cd(name);
								output += "ChildDirectory ";
								output += name;
								output += ': \n';
								showd();
								char back[3] = "..";
								cd(back);
							}
						}
					}
				}
				else {
					showd();
				}
			}
			else if (cmd == "md") {
				//创建目录
				char  name[16];
				string r1 = receive[1];
				vector<string>dirs = split(r1, "/");
				vector<string>save;
				for (int i = 0; i < pathname.size(); i++) {
					save.push_back(pathname[i]);
				}
				if (r1[0] == '/') {
					string di = "/";
					for (int i = 0; i < dirs.size() - 1; i++) {
						di += dirs[i];
						di += '/';
					}
					if (changed(di)) {
						strcpy(name, dirs[dirs.size() - 1].c_str());
						md(name);
						di = "/";
						for (int i = 0; i < save.size(); i++) {
							di += save[i];
							di += '/';
						}
						changed(di);
					}

				}
				else {
					string di = "";
					for (int i = 0; i < dirs.size() - 1; i++) {
						di += dirs[i];
						di += '/';
					}
					if (changed(di)) {
						strcpy(name, dirs[dirs.size() - 1].c_str());
						md(name);
						di = "/";
						for (int i = 0; i < save.size(); i++) {
							di += save[i];
							di += '/';
						}
						changed(di);
					}
				}

			}
			else if (cmd == "rd")
			{
				//删除目录
				char  name[16];
				char back[3] = "..";
				string r1 = receive[1];
				vector<string>dirs = split(r1, "/");
				vector<string>save;
				for (int i = 0; i < pathname.size(); i++) {
					save.push_back(pathname[i]);
				}
				if (r1[0] == '/') {
					string d = "/";
					for (int i = 0; i < dirs.size() - 1; i++) {
						d += dirs[i];
						d += '/';
					}
					if (changed(d)) {
						strcpy(name, dirs[dirs.size() - 1].c_str());
						cd(name);
						if (di->count != 0) {

							output += "directory contains files, remove all ? y/n \n";
							ReleaseSemaphore(m_Write[1], 1, NULL);

							write = 0;
							read = 1;
							WaitForSingleObject(m_Read[1], INFINITE);

							string tt = (string)m_pBuf[1];
							vector<string> receive = split(tt, " ");
							string ans = receive[0];
							if (ans != "y") {
								d = "/";
								for (int i = 0; i < save.size(); i++) {
									d += save[i];
									d += '/';
								}
								changed(d);
								goto out;
							}

						}
						clear();
						
						cd(back);
						rd(name);
						d = "/";
						for (int i = 0; i < save.size(); i++) {
							d += save[i];
							d += '/';
						}
						changed(d);
					}

				}
				else {
					string d = "";
					for (int i = 0; i < dirs.size() - 1; i++) {
						d += dirs[i];
						d += '/';
					}
					if (changed(d)) {
						strcpy(name, dirs[dirs.size() - 1].c_str());
						cd(name);
						if (di->count != 0) {

							output += "directory contains files, remove all ? y/n \n";
							ReleaseSemaphore(m_Write[1], 1, NULL);

							write = 0;
							read = 1;
							WaitForSingleObject(m_Read[1], INFINITE);//m_Read[1]信号量由程序B增加

							string tt = (string)m_pBuf[1];
							vector<string> receive = split(tt, " ");
							string ans = receive[0];
							if (ans != "y") {
								d = "/";
								for (int i = 0; i < save.size(); i++) {
									d += save[i];
									d += '/';
								}
								changed(d);
								goto out;
							}

						}
						clear();
						char back[3] = "..";
						cd(back);
						rd(name);
						d = "/";
						for (int i = 0; i < save.size(); i++) {
							d += save[i];
							d += '/';
						}
						changed(d);
					}
				}
			}
			else if (cmd == "newfile") {
				//创建新文件
				char  name[16];
				string r1 = receive[1];
				vector<string>dirs = split(r1, "/");
				vector<string>save;
				for (int i = 0; i < pathname.size(); i++) {
					save.push_back(pathname[i]);
				}
				if (r1[0] == '/') {
					string di = "/";
					for (int i = 0; i < dirs.size() - 1; i++) {
						di += dirs[i];
						di += '/';
					}
					if (changed(di)) {
						strcpy(name, dirs[dirs.size() - 1].c_str());
						if (newfile(name) == -1) {
							output += "has a same name file\n";
						}
						di = "/";
						for (int i = 0; i < save.size(); i++) {
							di += save[i];
							di += '/';
						}
						changed(di);
					}

				}
				else {
					string di = "";
					for (int i = 0; i < dirs.size() - 1; i++) {
						di += dirs[i];
						di += '/';
					}
					if (changed(di)) {
						strcpy(name, dirs[dirs.size() - 1].c_str());
						newfile(name);
						di = "/";
						for (int i = 0; i < save.size(); i++) {
							di += save[i];
							di += '/';
						}
						changed(di);
					}
				}
			}
			else if (cmd == "cat") {
				//打开文件
				char  name[16];
				string r1 = receive[1];
				vector<string>dirs = split(r1, "/");
				vector<string>save;
				for (int i = 0; i < pathname.size(); i++) {
					save.push_back(pathname[i]);
				}
				if (r1[0] == '/') {
					string di = "/";
					for (int i = 0; i < dirs.size() - 1; i++) {
						di += dirs[i];
						di += '/';
					}
					if (changed(di)) {
						strcpy(name, dirs[dirs.size() - 1].c_str());
						cat(name);
						di = "/";
						for (int i = 0; i < save.size(); i++) {
							di += save[i];
							di += '/';
						}
						changed(di);
					}

				}
				else {
					string di = "";
					for (int i = 0; i < dirs.size() - 1; i++) {
						di += dirs[i];
						di += '/';
					}
					if (changed(di)) {
						strcpy(name, dirs[dirs.size() - 1].c_str());
						cat(name);
						di = "/";
						for (int i = 0; i < save.size(); i++) {
							di += save[i];
							di += '/';
						}
						changed(di);
					}
				}
			}
			else if (cmd == "copy") {
				//读文件写入文件系统
				char name1[20];
				char  name2[20];
				if (receive.size() < 4) {
					goto out;
				}
				string n1 = receive[1];
				string n2 = receive[2];
				vector<string>dirs = split(n2, "/");
				string di = "/";
				if (n2[0] != '/') {
					 di = "";
				}
					for (int i = 0; i < dirs.size() - 1; i++) {
						di += dirs[i];
						di += '/';
					}
				
				strcpy(name1, receive[1].c_str());
				strcpy(name2, receive[2].c_str());
				char *ww = (char *)malloc(BlockSize);
				memset(ww, 0, BlockSize);
				FILE *input;

				if ((input = fopen(name1, "r")) != NULL) {
					fread(ww, sizeof(char), BlockSize, input);
				}
				vector<string>save;
				for (int i = 0; i < pathname.size(); i++) {
					save.push_back(pathname[i]);
				}
				if (changed(di)) {

					string temps = dirs[dirs.size() - 1];
					char name[16];
					strcpy(name, temps.c_str());

					int bc = newfile(name);
					if (bc != -1) {
						write_block(ww, fp, bc);
					}
					else {
						output += "no permission to write";
					}
					
					di = "/";
					for (int i = 0; i < save.size(); i++) {
						di += save[i];
						di += '/';
					}
					changed(di);
				}
			}
			else if (cmd == "del") {
				//删除文件
				char  name[16];
				string r1 = receive[1];
				vector<string>dirs = split(r1, "/");
				vector<string>save;
				for (int i = 0; i < pathname.size(); i++) {
					save.push_back(pathname[i]);
				}
				if (r1[0] == '/') {
					string di = "/";
					for (int i = 0; i < dirs.size() - 1; i++) {
						di += dirs[i];
						di += '/';
					}
					if (changed(di)) {
						strcpy(name, dirs[dirs.size() - 1].c_str());
						del(name,1);
						di = "/";
						for (int i = 0; i < save.size(); i++) {
							di += save[i];
							di += '/';
						}
						changed(di);
					}
				}
				else {
					string di = "";
					for (int i = 0; i < dirs.size() - 1; i++) {
						di += dirs[i];
						di += '/';
					}
					if (changed(di)) {
						strcpy(name, dirs[dirs.size() - 1].c_str());
						del(name,1);
						di = "/";
						for (int i = 0; i < save.size(); i++) {
							di += save[i];
							di += '/';
						}
						changed(di);
					}
				}
			}
		out:
			saved();
			ReleaseSemaphore(m_Write[1], 1, NULL);
			write = 0; read = 1;
		}
	}
	return true;
}
 