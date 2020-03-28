
#include<iostream>
#include <windows.h>
#include <process.h>
#include <string>
#include <mutex>
using namespace std;

#define BUF_SIZE 2048
wstring   m_cstrSzName[2];//共享内存名
wstring   m_ReadSemaphoreName[2];//读数据的信号量名
wstring   m_WriteSemaphoreName[2];//写数据的信号量名
HANDLE  m_hMapFile[2];//共享内存句柄
char    *m_pBuf[2];//读取数据的缓存
HANDLE	m_Read[2];//读数据的信号量句柄
HANDLE  m_Write[2];//写数据的信号量句柄
int uid;//用户id
int w = 0;//控制输入输出
int ex = 0;
void CloseFileMap()
{
	for (int i = 0; i < 2; i++)
	{
		//撤销文件视图UnMapViewOfFile()
		UnmapViewOfFile(m_pBuf[i]);
		//关闭映射文件句柄CloseHandle()
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
		//获取指向文件视图的指针 pBuf
		m_pBuf[i] = (char*)MapViewOfFile(
			m_hMapFile[i],	
			FILE_MAP_ALL_ACCESS, 
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

	while (true)
	{

		//通过m_pBuf[0]向共享内存m_hMapFile[0]发送数据
		char szInfo[BUF_SIZE] = { 0 };
		gets_s(szInfo, BUF_SIZE);
		//mt.lock();
		string a= szInfo;
		if (a == "exit") {
			ex = 1;
			break;
		}
		a += "  ";
		a += to_string(uid);
		strcpy_s(szInfo, a.c_str());
		WaitForSingleObject(m_Write[1], INFINITE);
		w = 1;
		memcpy(m_pBuf[1], szInfo, BUF_SIZE - 1);
		ReleaseSemaphore(m_Read[1], 1, NULL); 
	}
	return true;
}
unsigned int __stdcall ReadSharedData(void *pPM)
{

	while (true)
	{
		if (ex == 1) {
			break;
		}
		if (w != 1) {
			continue;
		}
		WaitForSingleObject(m_Read[0], INFINITE);
		cout << m_pBuf[0] ;
		ReleaseSemaphore(m_Write[0], 1, NULL);
		w = 0;
	}
	return true;
}
int main()
{
	cout << "input userid:";
	
	cin >> uid;
	CreateFileMap();//创建共享内存
	UINT threadId;
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, WriteSharedData, 0, 0, &threadId);//创建写数据的线程
	if (hThread == NULL)
	{
		cout << "Starting WriteSharedData Thread  Failed!" << endl;
	}
	HANDLE hThread2 = (HANDLE)_beginthreadex(NULL, 0, ReadSharedData, 0, 0, &threadId);//创建读数据的线程
	if (hThread2 == NULL)
	{
		cout << "Starting ReadSharedData Thread  Failed!" << endl;
	}
	Sleep(1000000);
	CloseFileMap();
	return 0;
}