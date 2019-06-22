#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>

/*
��һ��λͼ�����
ֻ����ɫ��λͼ ֻ����ɫ��λͼ ֻ����ɫ��λͼ
*/

//����λͼ
//@szPath: λͼ��·��
VOID HandleBitmap(CHAR* szPath)
{
	//·����Ϊ�� �ַ�����ַ��Ϊ��
	if (!strlen(szPath) || !szPath)
		return;
	//�ж��Ƿ���λͼ�ļ�
	INT nPathLen = strlen(szPath);
	_strupr(szPath);
	if (strncmp(&szPath[nPathLen - 4], ".BMP", 4))
		return;

	HANDLE hFile = 0;			//λͼ�ľ��
	INT nFileSize;				//λͼ���ܴ�С
	LPBYTE pBuffer = 0;			//����λͼ�Ļ�����
	DWORD dwByte;				//��ȡ���ݵĴ�С
	BITMAPFILEHEADER stFile;	//λͼ�ļ�ͷ
	BITMAPINFOHEADER stInfo;	//λͼ��Ϣͷ
	LPBYTE pData;				//λͼRGB����
	LPBYTE pDataRGB[3] = { 0 };	//ֻ��һ����ɫ��RGB����
	BOOL bTip;					//������־
	INT nLineLen;				//λͼһ��RGB���ݵĴ�С
	INT nBit;					//��ɫλ
	HANDLE hWrite = 0;			//д����ļ����
	CHAR szNewPath[MAX_PATH];	//��λͼ·��
	do
	{
		//��λͼ
		hFile = CreateFileA(szPath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if (!hFile)
			break;
		//��ȡλͼ��С
		nFileSize = GetFileSize(hFile, 0);
		if(!nFileSize)
			break;
		//����ռ�
		pBuffer = (LPBYTE)VirtualAlloc(0, nFileSize, MEM_COMMIT, PAGE_READWRITE);
		if(!pBuffer)
			break;
		//��ȡλͼ����
		ReadFile(hFile, pBuffer, nFileSize, &dwByte, 0);
		if(!dwByte)
			break;
		//�ر�λͼ���
		CloseHandle(hFile);
		hFile = 0;
		//����λͼ�ļ�ͷ λͼ��Ϣͷ ��λͼRGB����
		CopyMemory(&stFile, pBuffer, sizeof(BITMAPFILEHEADER));
		CopyMemory(&stInfo, (pBuffer + sizeof(BITMAPFILEHEADER)), sizeof(BITMAPINFOHEADER));
		pData = (pBuffer + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
		if (stInfo.biBitCount != 24)
			break;

		//�����ڴ�
		bTip = FALSE;
		for (int nIndex = 0; nIndex < 3; nIndex++)
		{
			pDataRGB[nIndex] = (LPBYTE)VirtualAlloc(0, nFileSize - sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER), MEM_COMMIT, PAGE_READWRITE);
			if (!pDataRGB[nIndex])
			{
				bTip = TRUE;
				break;
			}
		}
		//�ж������Ƿ�ʧ��
		if(bTip)
			break;
		//��ȡλͼһ����ɫ�ĳ��Ⱥ���ɫλ
		nLineLen = (stInfo.biWidth*stInfo.biBitCount / 8 + 3) / 4 * 4;
		nBit = stInfo.biBitCount / 8;
		//��ȡָ��RGB��ɫ��ָ���ڴ�
		for (int nHeight = 0; nHeight < stInfo.biHeight; nHeight++)
		{
			for (int nWidth = 0; nWidth < stInfo.biWidth; nWidth++)
			{
				for (int nRGB = 0; nRGB < nBit; nRGB++)
				{
					pDataRGB[nRGB][nLineLen*nHeight + nWidth * nBit + nRGB] = pData[nLineLen*nHeight + nWidth * nBit + nRGB];
				}
			}
		}
		//�ֱ�д�뵽λͼ�ļ�
		for (int nIndex = 0; nIndex < 3; nIndex++)
		{
			//ƴװ�µ�λͼ�ļ�ͷ
			wsprintf(szNewPath,"%s_%d_%s",szPath,nIndex,".BMP");
			//�����µĵ�ɫλͼ�ļ�
			hWrite = CreateFileA(szNewPath, GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_ALWAYS, 0, 0);
			if (!hWrite)
				break;
			//д��λͼ�ļ�ͷ
			WriteFile(hWrite, &stFile, sizeof(BITMAPFILEHEADER), &dwByte, 0);
			if(!dwByte)
				break;
			//д��λͼ��Ϣͷ
			WriteFile(hWrite, &stInfo, sizeof(BITMAPINFOHEADER), &dwByte, 0);
			if(!dwByte)
				break;
			//д�뵥ɫλͼ����
			WriteFile(hWrite, pDataRGB[nIndex], nFileSize - sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER), &dwByte, 0);
			if(!dwByte)
				break;
			//�ر��ļ����
			CloseHandle(hWrite);
			hWrite = 0;
		}

	} while (0);
	//�ͷš�����

	if (hFile)
		CloseHandle(hFile);
	if (pBuffer)
		VirtualFree(pBuffer, 0, MEM_RELEASE);
	for (int nIndex = 0; nIndex < 3; nIndex++)
		if (pDataRGB[nIndex])
			VirtualFree(pDataRGB[nIndex], 0, MEM_RELEASE);
	if (hWrite)
		CloseHandle(hWrite);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HDC hdc = 0;
	static PAINTSTRUCT stPs;
	static HFONT hFont;
	static CHAR szText[] = "��λͼ����...";
	static HDROP hDrop;
	static CHAR szPath[MAX_PATH];
	static INT nNums;
	switch (uMsg)
	{
	case WM_CREATE:
		DragAcceptFiles(hWnd, TRUE);
		break;
	case WM_DROPFILES:
		hDrop = (HDROP)wParam;
		nNums = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);//��ȡ�Ϸ��ļ�����
		for (int i = 0; i < nNums; i++)//�ֱ��ȡ�Ϸ��ļ���(��Զ���ļ�����)
		{
			DragQueryFile(hDrop, i, szPath, MAX_PATH);
			HandleBitmap(szPath);
		}
		DragFinish(hDrop);//�ͷ��ļ���������    
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &stPs);
		hFont = CreateFont(30, 0, 0, 0, FW_HEAVY,
			0, 0, 0, GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE, (TCHAR*)"����");
		SelectObject(hdc, hFont);
		SetBkMode(hdc, 0);
		TextOutA(hdc, 100, 100, szText, strlen(szText));
		DeleteObject(hFont);
		EndPaint(hWnd, &stPs);
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProcA(hWnd, uMsg, wParam, lParam);
		break;
	}
	return 0;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{
	WNDCLASSEXA stWndClass;
	ZeroMemory(&stWndClass, sizeof(WNDCLASSEXA));
	stWndClass.cbSize = sizeof(WNDCLASSEXA);
	stWndClass.hbrBackground = (HBRUSH)(GetStockObject(WHITE_BRUSH));
	stWndClass.hCursor = LoadCursorA(0, IDC_ARROW);
	stWndClass.hInstance = hInstance;
	stWndClass.lpfnWndProc = WindowProc;
	stWndClass.lpszClassName = "FRGB";
	stWndClass.style = CS_VREDRAW | CS_HREDRAW;
	if (!RegisterClassExA(&stWndClass))
		return -1;

	HWND hWnd = CreateWindowExA(0, "FRGB", "FRGB", WS_OVERLAPPEDWINDOW,
		300, 300, 600, 300, 0, 0, hInstance, 0);
	if (!hWnd)
		return -1;

	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);

	MSG stMsg;
	while (GetMessageA(&stMsg, 0, 0, 0))
	{
		TranslateMessage(&stMsg);
		DispatchMessageA(&stMsg);
	}
	return stMsg.wParam;
}
