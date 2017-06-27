#include<vector>
using namespace std;


vector<RECT> vecMonitor;


int dspNum = 0;


BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	MONITORINFO monitorInfo;
	monitorInfo.cbSize  = sizeof(MONITORINFO);

	if (GetMonitorInfo(hMonitor, &monitorInfo))
	{
		vecMonitor.push_back(monitorInfo.rcWork);
	}
	//MONITORINFOF_PRIMARY
	dspNum += 1;
	return TRUE;
}


HBITMAP CopyScreenToBitmap(LPRECT lpRect)
{
	HDC hScrDC,hMemDC;
	HBITMAP hBitmap,hOldBitmap;
	int nX1,nX2,nY1,nY2;
	int nWidth,nHeight;
	if (IsRectEmpty(lpRect))
	{
		return FALSE;
	}
	//Ϊ��Ļ�����豸������
	hScrDC = CreateDC("DISPLAY",NULL,NULL,NULL);
	//Ϊ��Ļ�豸�����������ݵ��ڴ��豸������
	hMemDC = CreateCompatibleDC(hScrDC);
	// ���ѡ����������
	nX1 = lpRect->left;
	nY1 = lpRect->top;
	nX2 = lpRect->right;
	nY2 = lpRect->bottom;

	nWidth = nX2 - nX1;
	nHeight = nY2 - nY1;
	// ����һ������Ļ�豸��������ݵ�λͼ
	hBitmap = CreateCompatibleBitmap(hScrDC,nWidth,nHeight);
	// ����λͼѡ���ڴ��豸��������
	hOldBitmap = (HBITMAP)SelectObject(hMemDC,hBitmap);
	// ����Ļ�豸�����������ڴ��豸��������
	BitBlt(hMemDC,0,0,nWidth,nHeight,hScrDC,nX1,nY1,SRCCOPY);
	//�õ���Ļλͼ�ľ��
	hBitmap = (HBITMAP)SelectObject(hMemDC,hOldBitmap);
	//���
	DeleteDC(hScrDC);
	DeleteDC(hMemDC);
	return hBitmap;
}


int SaveBitmapToFile(HBITMAP hBitmap, LPCTSTR lpFileName) //hBitmap Ϊ�ղŵ���Ļλͼ���
{
	//lpFileNameΪλͼ�ļ���
	HDC hDC; 
	//�豸������
	int iBits; 
	//��ǰ��ʾ�ֱ�����ÿ��������ռ�ֽ���
	WORD wBitCount; 
	//λͼ��ÿ��������ռ�ֽ���
	//�����ɫ���С�� λͼ�������ֽڴ�С �� λͼ�ļ���С �� д���ļ��ֽ���
	DWORD dwPaletteSize=0,dwBmBitsSize,dwDIBSize, dwWritten;
	BITMAP Bitmap; 
	//λͼ���Խṹ
	BITMAPFILEHEADER bmfHdr; 
	//λͼ�ļ�ͷ�ṹ
	BITMAPINFOHEADER bi; 
	//λͼ��Ϣͷ�ṹ 
	LPBITMAPINFOHEADER lpbi; 
	//ָ��λͼ��Ϣͷ�ṹ
	HANDLE fh, hDib, hPal;
	HPALETTE hOldPal=NULL;
	//�����ļ��������ڴ�������ɫ����

	//����λͼ�ļ�ÿ��������ռ�ֽ���
	hDC = CreateDC("DISPLAY",NULL,NULL,NULL);
	iBits = GetDeviceCaps(hDC, BITSPIXEL) * 
		GetDeviceCaps(hDC, PLANES);
	DeleteDC(hDC);
	if (iBits <= 1)
		wBitCount = 1;
	else if (iBits <= 4)
		wBitCount = 4;
	else if (iBits <= 8)
		wBitCount = 8;
	else if (iBits <= 24)
		wBitCount = 24;
	else
		wBitCount = 32;
	//�����ɫ���С
	if (wBitCount <= 8)
		dwPaletteSize=(1<<wBitCount)*sizeof(RGBQUAD);

	//����λͼ��Ϣͷ�ṹ
	GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = Bitmap.bmWidth;
	bi.biHeight = Bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = wBitCount;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	dwBmBitsSize = ((Bitmap.bmWidth*wBitCount+31)/32)*4*Bitmap.bmHeight;
	//Ϊλͼ���ݷ����ڴ�

	/*xxxxxxxx����λͼ��С�ֽ�һ��(����һ����������)xxxxxxxxxxxxxxxxxxxx 
	//ÿ��ɨ������ռ���ֽ���Ӧ��Ϊ4���������������㷨Ϊ:
	int biWidth = (Bitmap.bmWidth*wBitCount) / 32;
	if((Bitmap.bmWidth*wBitCount) % 32)
	biWidth++; //�����������ļ�1
	biWidth *= 4;//���������õ���Ϊÿ��ɨ���е��ֽ�����
	dwBmBitsSize = biWidth * Bitmap.bmHeight;//�õ���С
	xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx*/


	hDib = GlobalAlloc(GHND,dwBmBitsSize+dwPaletteSize+sizeof(BITMAPINFOHEADER));
	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
	*lpbi = bi;
	// �����ɫ�� 
	hPal = GetStockObject(DEFAULT_PALETTE);
	if (hPal)
	{
		hDC = ::GetDC(NULL);
		hOldPal=SelectPalette(hDC,(HPALETTE)hPal,FALSE);
		RealizePalette(hDC);
	}
	// ��ȡ�õ�ɫ�����µ�����ֵ
	GetDIBits(hDC,hBitmap,0,(UINT)Bitmap.bmHeight,(LPSTR)lpbi+sizeof(BITMAPINFOHEADER)+dwPaletteSize, (BITMAPINFO *)lpbi,DIB_RGB_COLORS);
	//�ָ���ɫ�� 
	if (hOldPal)
	{
		SelectPalette(hDC, hOldPal, TRUE);
		RealizePalette(hDC);
		::ReleaseDC(NULL, hDC);
	}
	//����λͼ�ļ� 
	fh=CreateFile(lpFileName, GENERIC_WRITE,0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (fh==INVALID_HANDLE_VALUE)
		return FALSE;
	// ����λͼ�ļ�ͷ
	bmfHdr.bfType = 0x4D42; // "BM"
	dwDIBSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+dwPaletteSize+dwBmBitsSize; 
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER)+(DWORD)sizeof(BITMAPINFOHEADER)+dwPaletteSize;
	// д��λͼ�ļ�ͷ
	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
	// д��λͼ�ļ���������
	WriteFile(fh, (LPSTR)lpbi, sizeof(BITMAPINFOHEADER)+dwPaletteSize+dwBmBitsSize , &dwWritten, NULL); 
	//��� 
	GlobalUnlock(hDib);
	GlobalFree(hDib);
	CloseHandle(fh);
	return TRUE;
}


void SceenShoot()
{
	
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, NULL);
	//int dspNum = ::GetSystemMetrics(SM_CMONITORS);

	
	int            dspNum           = 0;
	DISPLAY_DEVICE ddDisplay;
	DEVMODE        dmDevMode;

	ZeroMemory(&ddDisplay, sizeof(ddDisplay));
	ddDisplay.cb = sizeof(ddDisplay);
	ZeroMemory(&dmDevMode, sizeof(dmDevMode));
	dmDevMode.dmSize = sizeof(dmDevMode);
	BOOL bFlag = true;
	while (bFlag)
	{
		bFlag = EnumDisplayDevices(NULL, dspNum, &ddDisplay, 0);
		bFlag = bFlag & EnumDisplaySettings(ddDisplay.DeviceName, ENUM_CURRENT_SETTINGS, &dmDevMode);
		if (bFlag)
		{
			dspNum += 1;
		}
	}

	HBITMAP g_screenBmp=NULL;

	vector<RECT>::iterator it = vecMonitor.begin();
	
	RECT rc;
	rc.left = (*it).left;
	rc.top = (*it).top;
	rc.right = (*it).right;
	rc.bottom = (*it).bottom;
	g_screenBmp = CopyScreenToBitmap(&rc);
		
	SaveBitmapToFile(g_screenBmp,"d:\\hideme\\screen.bmp");
	
}