
#include "share.h"
#include "main.h"


int main() {


	init();
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