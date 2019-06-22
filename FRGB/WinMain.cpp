#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>

/*
将一个位图分离成
只带红色的位图 只带绿色的位图 只带蓝色的位图
*/

//处理位图
//@szPath: 位图的路径
VOID HandleBitmap(CHAR* szPath)
{
	//路径不为空 字符串地址不为空
	if (!strlen(szPath) || !szPath)
		return;
	//判断是否是位图文件
	INT nPathLen = strlen(szPath);
	_strupr(szPath);
	if (strncmp(&szPath[nPathLen - 4], ".BMP", 4))
		return;

	HANDLE hFile = 0;			//位图的句柄
	INT nFileSize;				//位图的总大小
	LPBYTE pBuffer = 0;			//整个位图的缓冲区
	DWORD dwByte;				//读取数据的大小
	BITMAPFILEHEADER stFile;	//位图文件头
	BITMAPINFOHEADER stInfo;	//位图信息头
	LPBYTE pData;				//位图RGB数据
	LPBYTE pDataRGB[3] = { 0 };	//只带一个颜色的RGB数据
	BOOL bTip;					//辅助标志
	INT nLineLen;				//位图一行RGB数据的大小
	INT nBit;					//颜色位
	HANDLE hWrite = 0;			//写入的文件句柄
	CHAR szNewPath[MAX_PATH];	//新位图路径
	do
	{
		//打开位图
		hFile = CreateFileA(szPath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if (!hFile)
			break;
		//获取位图大小
		nFileSize = GetFileSize(hFile, 0);
		if(!nFileSize)
			break;
		//申请空间
		pBuffer = (LPBYTE)VirtualAlloc(0, nFileSize, MEM_COMMIT, PAGE_READWRITE);
		if(!pBuffer)
			break;
		//读取位图数据
		ReadFile(hFile, pBuffer, nFileSize, &dwByte, 0);
		if(!dwByte)
			break;
		//关闭位图句柄
		CloseHandle(hFile);
		hFile = 0;
		//复制位图文件头 位图信息头 和位图RGB数据
		CopyMemory(&stFile, pBuffer, sizeof(BITMAPFILEHEADER));
		CopyMemory(&stInfo, (pBuffer + sizeof(BITMAPFILEHEADER)), sizeof(BITMAPINFOHEADER));
		pData = (pBuffer + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
		if (stInfo.biBitCount != 24)
			break;

		//申请内存
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
		//判断申请是否失败
		if(bTip)
			break;
		//获取位图一行颜色的长度和颜色位
		nLineLen = (stInfo.biWidth*stInfo.biBitCount / 8 + 3) / 4 * 4;
		nBit = stInfo.biBitCount / 8;
		//提取指定RGB颜色到指定内存
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
		//分别写入到位图文件
		for (int nIndex = 0; nIndex < 3; nIndex++)
		{
			//拼装新的位图文件头
			wsprintf(szNewPath,"%s_%d_%s",szPath,nIndex,".BMP");
			//创建新的单色位图文件
			hWrite = CreateFileA(szNewPath, GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_ALWAYS, 0, 0);
			if (!hWrite)
				break;
			//写入位图文件头
			WriteFile(hWrite, &stFile, sizeof(BITMAPFILEHEADER), &dwByte, 0);
			if(!dwByte)
				break;
			//写入位图信息头
			WriteFile(hWrite, &stInfo, sizeof(BITMAPINFOHEADER), &dwByte, 0);
			if(!dwByte)
				break;
			//写入单色位图数据
			WriteFile(hWrite, pDataRGB[nIndex], nFileSize - sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER), &dwByte, 0);
			if(!dwByte)
				break;
			//关闭文件句柄
			CloseHandle(hWrite);
			hWrite = 0;
		}

	} while (0);
	//释放。。。

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
	static CHAR szText[] = "将位图拖入...";
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
		nNums = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);//获取拖放文件个数
		for (int i = 0; i < nNums; i++)//分别获取拖放文件名(针对多个文件操作)
		{
			DragQueryFile(hDrop, i, szPath, MAX_PATH);
			HandleBitmap(szPath);
		}
		DragFinish(hDrop);//释放文件名缓冲区    
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &stPs);
		hFont = CreateFont(30, 0, 0, 0, FW_HEAVY,
			0, 0, 0, GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE, (TCHAR*)"宋体");
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
