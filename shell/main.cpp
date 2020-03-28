
#include<iostream>
#include <windows.h>
#include <process.h>
#include <string>
#include <mutex>
using namespace std;

#define BUF_SIZE 2048
wstring   m_cstrSzName[2];//�����ڴ���
wstring   m_ReadSemaphoreName[2];//�����ݵ��ź�����
wstring   m_WriteSemaphoreName[2];//д���ݵ��ź�����
HANDLE  m_hMapFile[2];//�����ڴ���
char    *m_pBuf[2];//��ȡ���ݵĻ���
HANDLE	m_Read[2];//�����ݵ��ź������
HANDLE  m_Write[2];//д���ݵ��ź������
int uid;//�û�id
int w = 0;//�����������
int ex = 0;
void CloseFileMap()
{
	for (int i = 0; i < 2; i++)
	{
		//�����ļ���ͼUnMapViewOfFile()
		UnmapViewOfFile(m_pBuf[i]);
		//�ر�ӳ���ļ����CloseHandle()
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
		//��ȡָ���ļ���ͼ��ָ�� pBuf
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

		//ͨ��m_pBuf[0]�����ڴ�m_hMapFile[0]��������
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
	CreateFileMap();//���������ڴ�
	UINT threadId;
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, WriteSharedData, 0, 0, &threadId);//����д���ݵ��߳�
	if (hThread == NULL)
	{
		cout << "Starting WriteSharedData Thread  Failed!" << endl;
	}
	HANDLE hThread2 = (HANDLE)_beginthreadex(NULL, 0, ReadSharedData, 0, 0, &threadId);//���������ݵ��߳�
	if (hThread2 == NULL)
	{
		cout << "Starting ReadSharedData Thread  Failed!" << endl;
	}
	Sleep(1000000);
	CloseFileMap();
	return 0;
}