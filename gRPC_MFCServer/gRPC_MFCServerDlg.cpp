#include "pch.h"
#include "framework.h"
#include "gRPC_MFCServer.h"
#include "gRPC_MFCServerDlg.h"
#include "afxdialogex.h"
#include "User_RPCService.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WMSG_FUNCTION		WM_USER + 1

CgRPCMFCServerDlg::CgRPCMFCServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GRPC_MFCSERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CgRPCMFCServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_RECV, _editRecv);
	DDX_Control(pDX, IDC_BUTTON_RUN_SERVER, _btnRunServer);
	DDX_Control(pDX, IDC_BUTTON_RUN_ASYNC_SERVER, _btnRunAsyncServer);
}

BEGIN_MESSAGE_MAP(CgRPCMFCServerDlg, CDialogEx)
	ON_MESSAGE(WMSG_FUNCTION, &CgRPCMFCServerDlg::OnFunction)
	ON_BN_CLICKED(IDC_BUTTON_RUN_SERVER, &CgRPCMFCServerDlg::OnBnClickedButtonRunServer)
	ON_BN_CLICKED(IDC_BUTTON_RUN_ASYNC_SERVER, &CgRPCMFCServerDlg::OnBnClickedButtonRunAsyncServer)
END_MESSAGE_MAP()

BOOL CgRPCMFCServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);		
	SetIcon(m_hIcon, FALSE);	

	return TRUE; 
}

void CgRPCMFCServerDlg::AppendMsg(const WCHAR* msg)
{
	WCHAR* tmpMsg = new WCHAR[wcslen(msg) + 1];
	memset(tmpMsg, 0, sizeof(WCHAR) * (wcslen(msg) + 1));
	wsprintf(tmpMsg, msg);

	TheadFunc* pFunc = new TheadFunc;
	pFunc->Func = ([=]()
	{
		if (_editRecv.GetLineCount() > 100)
		{
			_editRecv.Clear();
		}

		CString curMsg;
		_editRecv.GetWindowTextW(curMsg);
		curMsg += "\r\n";

		CString strTime;
		SYSTEMTIME sysTime;
		GetLocalTime(&sysTime);
		strTime.Format(L"%02ld:%02ld:%02ld.%03ld ",
			sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);

		curMsg += strTime;
		curMsg += tmpMsg;
		_editRecv.SetWindowTextW(curMsg);
		_editRecv.LineScroll(_editRecv.GetLineCount());

		delete[] tmpMsg;
	});

	PostMessage(WMSG_FUNCTION, (WPARAM)pFunc);
}

LRESULT CgRPCMFCServerDlg::OnFunction(WPARAM wParam, LPARAM lParam)
{
	TheadFunc* pFunc = (TheadFunc*)wParam;
	pFunc->Func();
	delete pFunc;

	return TRUE;
}

void CgRPCMFCServerDlg::OnBnClickedButtonRunServer()
{
	if (_rpcServer)
	{
		// 停止RPC服务
		AppendMsg(L"手动停止服务");
		_rpcServer->Shutdown();

		_btnRunAsyncServer.EnableWindow(TRUE);
		_btnRunServer.SetWindowTextW(L"启动服务");
	}
	else
	{
		std::thread([&]() {
			grpc::EnableDefaultHealthCheckService(true);
			grpc::reflection::InitProtoReflectionServerBuilderPlugin();

			ServerBuilder builder;

			// 监听不需要认证的连接
			builder.AddListeningPort(_serverAddr, grpc::InsecureServerCredentials());
			// 注册一个服务
			CUser_RPCService userService;
			builder.RegisterService(&userService);
			// 启动服务器
			_rpcServer = builder.BuildAndStart();
			AppendMsg(L"服务启动 0.0.0.0:23351");

			// 等待服务停止（阻塞）
			_rpcServer->Wait();

			_rpcServer = nullptr;
			AppendMsg(L"服务已停止");
		}).detach();

		_btnRunServer.SetWindowTextW(L"关闭服务");
		_btnRunAsyncServer.EnableWindow(FALSE);
	}
}

void CgRPCMFCServerDlg::OnBnClickedButtonRunAsyncServer()
{
	if (_asyncRPCServer)
	{
		// 停止异步服务
		AppendMsg(L"手动停止异步服务");
		_asyncRPCServer->Shutdown();
		_asyncRPCServer = nullptr;

		_btnRunServer.EnableWindow(TRUE);
		_btnRunAsyncServer.SetWindowTextW(L"启动异步服务");
	}
	else
	{
		// 启动异步服务
		_asyncRPCServer = make_unique<CAsyncRPCService>();
		_asyncRPCServer->Run(_serverAddr, 4);

		AppendMsg(L"异步服务启动 0.0.0.0:23351");
		_btnRunAsyncServer.SetWindowTextW(L"关闭异步服务");
		_btnRunServer.EnableWindow(FALSE);
	}
}
