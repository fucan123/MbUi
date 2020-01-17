
// MbUiDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "MbUi.h"
#include "MbUiDlg.h"
#include "afxdialogex.h"

#include <My/Common/func.h>
#include <My/Common/Explode.h>
#include <My/Common/MachineID.h>
#include <My/Driver/KbdMou.h>
#include <My/Db/Sqlite.h>
#include <My/Win32/PE.h>
#include <My/Win32/Peb.h>
#include <My/Common/C.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MSG_CALLJS       (WM_USER+100)

#define GetGameProc(type, name) (type)g_dlg->GetGameProcAddress(name)

typedef void(*WINAPI Func_Game_NoArg)();
typedef void (*WINAPI Func_Game_Init)(HWND, const char*);

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMbUiDlg 对话框


CMbUiDlg* g_dlg = nullptr;
CMbUiDlg::CMbUiDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MBUI_DIALOG, pParent)
{
	g_dlg = this;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMbUiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMbUiDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_MESSAGE(MSG_CALLJS, &CMbUiDlg::OnCallJs)
	ON_WM_HOTKEY()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CMbUiDlg 消息处理程序

BOOL CMbUiDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	RegisterHotKey(m_hWnd, 1001, NULL, VK_F1);
	RegisterHotKey(m_hWnd, 1002, NULL, VK_F2);

	bool adr = AdjustPrivileges();
	::printf("提权状态结果:%d\n", adr);

	DWORD adbs[10];
	DWORD adbs_l = SGetProcessIds(L"adb.exe", adbs, sizeof(adbs) / sizeof(DWORD));
	for (int i = 0; i < adbs_l; i++) {
		char cmd[64];
		sprintf_s(cmd, "taskkill /f /t /pid %d", adbs[i]);
		system(cmd);
	}
	adbs_l = SGetProcessIds(L"conhost.exe", adbs, sizeof(adbs) / sizeof(DWORD));
	for (int i = 0; i < adbs_l; i++) {
		char cmd[64];
		sprintf_s(cmd, "taskkill /f /t /pid %d", adbs[i]);
		//system(cmd);
	}

#ifdef _DEBUG
	AllocConsole();
	freopen("CON", "w", stdout);
#endif
#ifdef  _DEBUG
	//AllocConsole();
	//freopen("CON", "w", stdout);

	SHGetSpecialFolderPathA(0, m_ConfPath, CSIDL_DESKTOPDIRECTORY, 0);
	strcat(m_ConfPath, "\\MNQ-9Star");
#else
	//pfnNtQuerySetInformationThread f = (pfnNtQuerySetInformationThread)GetNtdllProcAddress("ZwSetInformationThread");
	//NTSTATUS sta = f(GetCurrentThread(), ThreadHideFromDebugger, NULL, 0);
	//::printf("sta:%d\n", sta);

	GetCurrentDirectoryA(MAX_PATH, m_ConfPath);
#endif //  _DEBUG

#ifndef x64
#else
#ifdef _DEBUG
	m_hGameModule = LoadLibrary(L"C:\\Users\\fucan\\Desktop\\MNQ-9Star\\vs\\x64\\Game.dll");
#else
	CString dll;
	dll += m_ConfPath;
	dll += L"\\files\\Game.dll";
	m_hGameModule = LoadLibrary(dll);

	CString tip;
	tip.Format(L"(%d)\n", GetLastError());

	//AfxMessageBox(dll);
	//AfxMessageBox(tip);
	if (m_hGameModule) {
		//AfxMessageBox(L"加载成功Game");
	}
	else{
		//AfxMessageBox(L"加载失败Game");
	}
	
#endif
	
#endif

	CRect rect(0, 0, 800, 750);
	SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOMOVE);
	GetClientRect(rect);

	jsBindFunction("CallCpp", js_Func, 1);
	m_web = wkeCreateWebWindow(WKE_WINDOW_TYPE_CONTROL, *this, 0, 0, rect.Width(), rect.Height());
	//wkeLoadURL(m_web, "https://www.baidu.com");
	wchar_t html[MAX_PATH];
#ifdef _DEBUG
	wsprintfW(html, L"E:\\CPP\\MbUi\\html\\static\\index.html");
#else
	wsprintfW(html, L"%hs\\html\\static\\index.html", m_ConfPath);
#endif
	//AfxMessageBox(html);
	wkeLoadFileW(m_web, html);
	wkeShowWindow(m_web, TRUE);

	m_es = wkeGlobalExec(m_web);
	
	wkeOnDocumentReady(m_web, DocumentReadyCallback, this);

	//mouse_event(MOUSEEVENTF_MOVE| MOUSEEVENTF_ABSOLUTE, 1000 * 65536 / GetSystemMetrics(SM_CXSCREEN), 50 * 65536 / GetSystemMetrics(SM_CYSCREEN), 0, 0);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMbUiDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMbUiDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMbUiDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 调用JS
LRESULT CMbUiDlg::OnCallJs(WPARAM w, LPARAM l)
{
	my_msg* msg = (my_msg*)w;
	//::printf("OnAddLog:%08X\n", msg);

	//printf("msg:%d %s\n", msg->op, msg->text);
	//AfxMessageBox(msg->text_w);
	switch (msg->op)
	{
	case MSG_ADDLOG:
		AddLog(msg);
		break;
	case MSG_SETTEXT:
		SetText(msg);
		break;
	case MSG_ADDTABLEROW:
		AddTableRow(msg);
		break;
	case MSG_FILLTABLE:
		FillTable(msg);
		break;
	case MSG_SETSETTING:
		SetSetting(msg);
		break;
	case MSG_ALERT:
		Alert(msg);
		break;
	default:
		break;
	}

	if (l) {
		::printf("delete msg %p\n", msg);
		//delete msg;
	}

	return 0;
}

// 写入日记信息
void CMbUiDlg::AddLog(my_msg* pMsg)
{
	jsExecState es = wkeGlobalExec(m_web);
	jsValue f = jsGetGlobal(es, "AddLog");
	jsValue vs[3];
	vs[0] = jsNull();
	if (pMsg->text[0]) {
		vs[1] = jsString(es, pMsg->text);
	}
	else {
		vs[1] = jsStringW(es, pMsg->text_w);
	}

	if (pMsg->cla)
		vs[2] = jsString(es, pMsg->cla);
	else
		vs[2] = jsNull();

	jsCallGlobal(es, f, vs, sizeof(vs)/sizeof(jsValue));
}

// 设置html文字
void CMbUiDlg::SetText(my_msg* pMsg)
{
	jsExecState es = wkeGlobalExec(m_web);
	jsValue f, vs[5];
	int argv = 0;

	if (pMsg->table_text) {
		f = jsGetGlobal(es, "UpdateTableText");
		vs[0] = jsString(es, pMsg->id);
		vs[1] = jsInt(pMsg->value[0]);
		vs[2] = jsInt(pMsg->value[1]);
		if (pMsg->text[0]) vs[3] = jsString(es, pMsg->text);
		else vs[3] = jsStringW(es, pMsg->text_w);
		argv = 4;
	}
	else if (pMsg->status_text) {
		f = jsGetGlobal(es, "UpdateStatusText");
		if (pMsg->text[0]) vs[0] = jsString(es, pMsg->text);
		else vs[0] = jsStringW(es, pMsg->text_w);
		vs[1] = jsInt(pMsg->value[0]);
		argv = 2;
	}
	else {
		f = jsGetGlobal(es, "SetText");
		vs[0] = jsString(es, pMsg->id);
		if (pMsg->text[0]) vs[1] = jsString(es, pMsg->text);
		else vs[1] = jsStringW(es, pMsg->text_w);
		argv = 2;
	}
	
	jsCallGlobal(es, f, vs, argv);
}

// 更新html文字
void CMbUiDlg::AddTableRow(my_msg * pMsg)
{
	jsExecState es = wkeGlobalExec(m_web);
	jsValue f = jsGetGlobal(es, "AddTableRow");
	jsValue vs[4];
	vs[0] = jsString(es, pMsg->id);
	vs[1] = jsInt(0);
	vs[2] = jsInt(pMsg->value[0]);
	if (pMsg->text[0]) vs[3] = jsString(es, pMsg->text);
	else vs[3] = jsStringW(es, pMsg->text_w);

	jsCallGlobal(es, f, vs, sizeof(vs) / sizeof(jsValue));
}

void  CMbUiDlg::FillTable(my_msg* pMsg)
{
	jsExecState es = wkeGlobalExec(m_web);
	jsValue f = jsGetGlobal(es, "FillTableRow");
	jsValue vs[3];
	vs[0] = jsNull();
	vs[1] = jsInt(0);
	vs[2] = jsInt(0);

	jsCallGlobal(es, f, vs, sizeof(vs) / sizeof(jsValue));
}

// 设置
void CMbUiDlg::SetSetting(my_msg * pMsg)
{
	jsExecState es = wkeGlobalExec(m_web);
	jsValue f = jsGetGlobal(es, "SetSetting");
	jsValue vs[3];
	vs[0] = jsString(es, pMsg->id);
	vs[1] = jsInt(pMsg->value[0]);
	vs[2] = jsInt(pMsg->value[1]);

	jsCallGlobal(es, f, vs, sizeof(vs) / sizeof(jsValue));
}

// Alert
void CMbUiDlg::Alert(my_msg * pMsg)
{
	jsExecState es = wkeGlobalExec(m_web);
	jsValue f = jsGetGlobal(es, "ShowMsg");
	jsValue vs[3];
	if (pMsg->text[0]) vs[0] = jsString(es, pMsg->text);
	else vs[0] = jsStringW(es, pMsg->text_w);
	vs[1] = jsStringW(es, L"提示");
	vs[2] = jsInt(pMsg->value[0]);

	jsCallGlobal(es, f, vs, sizeof(vs) / sizeof(jsValue));
}

// 获取游戏模块函数
FARPROC CMbUiDlg::GetGameProcAddress(LPCSTR lpProcName)
{
	return GetProcAddress(m_hGameModule, lpProcName);
}

// 文档加载完成
void WKE_CALL_TYPE CMbUiDlg::DocumentReadyCallback(wkeWebView webView, void* param)
{
	//AfxMessageBox(L"DocumentReadyCallback");

#if 1
	CMbUiDlg* p = (CMbUiDlg*)param;
	jsExecState es = wkeGlobalExec(webView);
	jsValue f = jsGetGlobal(es, "test");
	if (f) {
		//AfxMessageBox(L"jsGetGlobal");
	}
	jsValue vs[2];
	vs[0] = jsInt(123);
	vs[1] = jsStringW(es, L"法克油");
	jsCallGlobal(es, f, vs, 2);
#endif
	if (!g_dlg->m_hGameModule) {
		AfxMessageBox(L"无法加载游戏模块！！！");
		return;
	}

	Func_Game_Init Game_Init = GetGameProc(Func_Game_Init, "Game_Init");
	if (Game_Init) {
		printf("%p\n", Game_Init);
		Game_Init(g_dlg->m_hWnd, g_dlg->m_ConfPath);
	}

	CreateThread(NULL, NULL, Thread, param, NULL, NULL);
}

// js调用函数
jsValue JS_CALL CMbUiDlg::js_Func(jsExecState es)
{
	printf("参数数量:%d\n", jsArgCount(es));
	if (jsArgType(es, 0) != JSTYPE_STRING)
		return 0;

	const utf8* func_name = jsToString(es, jsArg(es, 0));
	printf("%s\n", jsToString(es, jsArg(es, 0)));
	if (strcmp("set_title", func_name) == 0)
		return g_dlg->SetTitle(es);
	if (strcmp("open_menu", func_name) == 0)
		return g_dlg->OpenMenu(es);
	if (strcmp("start", func_name) == 0)
		return g_dlg->InstallDll(es);
	if (strcmp("open_game", func_name) == 0)
		return g_dlg->OpenGame(es);
	if (strcmp("close_game", func_name) == 0)
		return g_dlg->CloseGame(es);
	if (strcmp("in_team", func_name) == 0)
		return g_dlg->InTeam(es);
	if (strcmp("put_setting", func_name) == 0)
		return g_dlg->PutSetting(es);
	if (strcmp("verify_card", func_name) == 0)
		return g_dlg->VerifyCard(es);
	if (strcmp("fb_record", func_name) == 0)
		return g_dlg->FBRecord(es);

	CString str;
	str.Format(L"参数数量:%d", jsArgCount(es));
	return jsStringW(es, str);
}

// 设置程序标题
jsValue CMbUiDlg::SetTitle(jsExecState es)
{
	SetWindowText(jsToStringW(es, jsArg(es, 1)));
	return jsInt(0);
}

// 打开帐号菜单 返回是否登录
jsValue CMbUiDlg::OpenMenu(jsExecState es)
{
	//printf("OpenMenu:%d\n", jsToInt(es, jsArg(es, 1)));
	typedef int (*WINAPI Func_Game_IsLogin)(int index);
	return jsInt((GetGameProc(Func_Game_IsLogin, "Game_IsLogin"))(jsToInt(es, jsArg(es, 1))));
}

// 安装dll驱动
jsValue CMbUiDlg::InstallDll(jsExecState es)
{
	typedef int (*WINAPI Func_Game_InstallDll)();
	return jsInt((GetGameProc(Func_Game_InstallDll, "Game_InstallDll"))());
}

// 打开游戏
jsValue CMbUiDlg::OpenGame(jsExecState es)
{
	typedef int(*WINAPI Func_Game_OpenGame)(int, int);
	return jsInt((GetGameProc(Func_Game_OpenGame, "Game_OpenGame"))(jsToInt(es, jsArg(es, 1)), jsToInt(es, jsArg(es, 2))));
}

// 关闭游戏
jsValue CMbUiDlg::CloseGame(jsExecState es)
{
	typedef int(*WINAPI Func_Game_CloseGame)(int index);
	return jsInt((GetGameProc(Func_Game_CloseGame, "Game_CloseGame"))(jsToInt(es, jsArg(es, 1))));
}

// 设置入队
jsValue CMbUiDlg::InTeam(jsExecState es)
{
	typedef int(*WINAPI Func_Game_InTeam)(int index);
	return jsInt((GetGameProc(Func_Game_InTeam, "Game_InTeam"))(jsToInt(es, jsArg(es, 1))));
}

// 设置
jsValue CMbUiDlg::PutSetting(jsExecState es)
{
	typedef int(*WINAPI Func_Game_PutSetting)(const wchar_t*, int);
	return jsInt((GetGameProc(Func_Game_PutSetting, "Game_PutSetting"))(
		jsToStringW(es, jsArg(es, 1)),
		jsToInt(es, jsArg(es, 2)))
	);
}

// 验证卡号
jsValue CMbUiDlg::VerifyCard(jsExecState es)
{
	typedef int(*WINAPI Func_Game_VerifyCard)(const wchar_t*);
	return jsInt((GetGameProc(Func_Game_VerifyCard, "Game_VerifyCard"))(jsToStringW(es, jsArg(es, 1))));
}

// 查询副本记录
jsValue CMbUiDlg::FBRecord(jsExecState es)
{
	typedef int(*WINAPI Func_Game_SelectFBRecord)(char***, int*);
	int col = 0;
	char** result = nullptr;
	int row = (GetGameProc(Func_Game_SelectFBRecord, "Game_SelectFBRecord"))(&result, &col);

	jsValue v = jsEmptyArray(es);
	for (int i = 1; i <= row; i++) {
		int index = i * col;

		jsValue object = jsEmptyObject(es);
		jsSet(es, object, "id", jsInt(atoi(result[index])));
		jsSet(es, object, "start_time", jsInt(atoi(result[index + 1])));
		jsSet(es, object, "end_time", jsInt(atoi(result[index + 2])));
		jsSet(es, object, "time_long", jsInt(atoi(result[index + 3])));
		jsSet(es, object, "status", jsInt(atoi(result[index + 4])));
		jsSetAt(es, v, i - 1, object);
	}
	return v;
}

// 线程
DWORD WINAPI CMbUiDlg::Thread(LPVOID param)
{
	while (true) {
		my_msg msg;
		COPY_MSG_W(msg, 1, nullptr, L"text w ", "cb", 0);
		//::PostMessage(g_dlg->m_hWnd, MSG_CALLJS, (WPARAM)&msg, 0);
		Sleep(800);
	}
	
	return 0;
}


void CMbUiDlg::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	typedef int(*WINAPI Func_Game_Pause)(bool);
	switch (nHotKeyId)
	{
	case 1001:
		(GetGameProc(Func_Game_Pause, "Game_Pause"))(true);
		break;
	case 1002:
		(GetGameProc(Func_Game_Pause, "Game_Pause"))(false);
		break;
	}

	CDialogEx::OnHotKey(nHotKeyId, nKey1, nKey2);
}


void CMbUiDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	typedef int(*WINAPI Func_Game_Relase)();
	(GetGameProc(Func_Game_Relase, "Game_Relase"))();

	UnregisterHotKey(m_hWnd, 1001);
	UnregisterHotKey(m_hWnd, 1002);

	CDialogEx::OnClose();
}
