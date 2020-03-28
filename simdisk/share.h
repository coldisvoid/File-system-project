#pragma once
#include "main.h"
#include <windows.h>
#include <process.h>
#include <mutex>
#define BUF_SIZE 2048
wstring   m_cstrSzName[2];//�����ڴ���
wstring   m_ReadSemaphoreName[2];//�����ݵ��ź�����
wstring   m_WriteSemaphoreName[2];//д���ݵ��ź�����
HANDLE  m_hMapFile[2];//�����ڴ���
char    *m_pBuf[2];//��ȡ���ݵĻ���
HANDLE	m_Read[2];//�����ݵ��ź������
HANDLE  m_Write[2];//д���ݵ��ź������
//���ƶ�д
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
	m_cstrSzName[0] = L"NameOfMappingObject0";//�����ڴ�0������
	m_cstrSzName[1] = L"NameOfMappingObject1";//�����ڴ�1������
	m_ReadSemaphoreName[0] = L"ReadSemaphoreName0";//���ź���0������
	m_ReadSemaphoreName[1] = L"ReadSemaphoreName1";//���ź���1������
	m_WriteSemaphoreName[0] = L"WriteSemaphoreName0";//д�ź���0������
	m_WriteSemaphoreName[1] = L"WriteSemaphoreName1";//д�ź���1������
	for (int i = 0; i < 2; i++)
	{
		//���������ļ���� hMapFile
		m_hMapFile[i] = CreateFileMapping(
			INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE,
			0,
			BUF_SIZE,
			m_cstrSzName[i].c_str()
		);
		m_pBuf[i] = (char*)MapViewOfFile(
			m_hMapFile[i],				  //�����ڴ�ľ��
			FILE_MAP_ALL_ACCESS,      //�ɶ�д
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
	//�����ݷŵ������ڴ�
	while (true)
	{
		if (read == 1) {
			//ͨ��m_pBuf[0]�����ڴ�m_hMapFile[0]��������
			WaitForSingleObject(m_Write[0], INFINITE);

			char szInfo[BUF_SIZE] = { 0 };
			char back[1024];
			output += "\n/";
			for (int i = 0; i < pathname.size(); i++) {
				output += pathname[i];
				output += "/";
			}
			//��string outputת��Ϊ char* ��д��
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
	//3.�ӹ����ڴ��ж�ȡ����
	while (true)
	{
		if (write == 1) {
			WaitForSingleObject(m_Read[1], INFINITE);//m_Read[1]�ź����ɳ���B����
			//��ȡ��ָ��ַ�����ȡ������������û�id
			string temp = (string)m_pBuf[1];
			vector<string> receive = split(temp, " ");
			if (temp == "") {
				goto out;
			}
			userid = atoi(receive[receive.size() - 1].c_str());
			changeuser();
			cmd = receive[0];
			if (cmd == "info") {
				output = output + "info:  ��ʾ����ϵͳ��Ϣ\n"
					+ "cd �� : �ı�Ŀ¼\n"
					+ "dir �� : ��ʾĿ¼\n"
					+ "md �� : ����Ŀ¼\n"
					+ "rd �� : ɾ��Ŀ¼\n"
					+ "newfile �� : �����ļ�\n"
					+ "cat �� : ��ȡ�ļ�\n"
					+ "copy �� : �����ļ�\n";
			}
			else if (cmd == "cd") {
				//�л�Ŀ¼
				int result = changed(receive[1]);
				if (result) {
					//cout << "success" << endl;
				}
				else {
					//cout << "fail" << endl;
				}
			}
			else if (cmd == "dir") {
				//��ӡ��ǰ·�����ļ�
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
				//����Ŀ¼
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
				//ɾ��Ŀ¼
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
							WaitForSingleObject(m_Read[1], INFINITE);//m_Read[1]�ź����ɳ���B����

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
				//�������ļ�
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
				//���ļ�
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
				//���ļ�д���ļ�ϵͳ
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
				//ɾ���ļ�
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
 