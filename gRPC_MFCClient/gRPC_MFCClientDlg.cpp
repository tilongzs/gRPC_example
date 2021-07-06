#include "pch.h"
#include "framework.h"
#include "gRPC_MFCClient.h"
#include "gRPC_MFCClientDlg.h"
#include "afxdialogex.h"

#include <atlconv.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WMSG_FUNCTION		WM_USER + 1
#define TIMER_CheckGRPC		1


CgRPCMFCClientDlg::CgRPCMFCClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GRPC_MFCSERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CgRPCMFCClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_RECV, _editRecv);
	DDX_Control(pDX, IDC_EDIT_GetUser, _editGetUser);
	DDX_Control(pDX, IDC_BUTTON_RUN_CLIENT, _btnRunClient);
	DDX_Control(pDX, IDC_BUTTON_RUN_ASYNC_CLIENT, _btnRunAsyncClient);
}

BEGIN_MESSAGE_MAP(CgRPCMFCClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_MESSAGE(WMSG_FUNCTION, &CgRPCMFCClientDlg::OnFunction)
	ON_BN_CLICKED(IDC_BUTTON_GetUser, &CgRPCMFCClientDlg::OnBnClickedButtonGetuser)
	ON_BN_CLICKED(IDC_BUTTON_GetUsersByRole, &CgRPCMFCClientDlg::OnBnClickedButtonGetUsersByRole)
	ON_BN_CLICKED(IDC_BUTTON_AddUsers, &CgRPCMFCClientDlg::OnBnClickedButtonAddusers)
	ON_BN_CLICKED(IDC_BUTTON_DeleteUsers, &CgRPCMFCClientDlg::OnBnClickedButtonDeleteusers)
	ON_BN_CLICKED(IDC_BUTTON_RUN_CLIENT, &CgRPCMFCClientDlg::OnBnClickedButtonRunClient)
	ON_BN_CLICKED(IDC_BUTTON_RUN_ASYNC_CLIENT, &CgRPCMFCClientDlg::OnBnClickedButtonRunAsyncClient)
	ON_BN_CLICKED(IDC_BUTTON_AsyncGetUser, &CgRPCMFCClientDlg::OnBnClickedButtonAsyncgetuser)
	ON_BN_CLICKED(IDC_BUTTON_AsyncGetUsersByRole, &CgRPCMFCClientDlg::OnBnClickedButtonAsyncGetUsersByRole)
	ON_BN_CLICKED(IDC_BUTTON_AsyncAddUsers, &CgRPCMFCClientDlg::OnBnClickedButtonAsyncAddusers)
	ON_BN_CLICKED(IDC_BUTTON_AsyncDeleteUsers, &CgRPCMFCClientDlg::OnBnClickedButtonAsyncdeleteusers)
	ON_WM_CLOSE()
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CgRPCMFCClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);		
	SetIcon(m_hIcon, FALSE);

	_editGetUser.SetWindowTextW(L"accountname1");

	return TRUE; 
}

void CgRPCMFCClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

void CgRPCMFCClientDlg::OnClose()
{
	if (_user_AsyncRPCClient)
	{
		_user_AsyncRPCClient->Shutdown();
	}

	KillTimer(TIMER_CheckGRPC);
	_ctsCommon->cancel();

	CDialogEx::OnClose();
}

void CgRPCMFCClientDlg::AppendMsg(const WCHAR* msg)
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

LRESULT CgRPCMFCClientDlg::OnFunction(WPARAM wParam, LPARAM lParam)
{
	TheadFunc* pFunc = (TheadFunc*)wParam;
	pFunc->Func();
	delete pFunc;

	return TRUE;
}

void CgRPCMFCClientDlg::OnBnClickedButtonRunClient()
{
	if (_user_RPCClient)
	{
		_user_RPCClient = nullptr;
		_channel = nullptr;
		KillTimer(TIMER_CheckGRPC);
		_ctsCommon->cancel();

		_btnRunAsyncClient.EnableWindow(TRUE);
		_btnRunClient.SetWindowTextW(L"启动客户端");
		AppendMsg(L"客户端关闭 0.0.0.0:23351");
	}
	else
	{
		// 创建客户端
		_channel = grpc::CreateChannel(_serverAddr, grpc::InsecureChannelCredentials()); // 创建gRPC channel
		_user_RPCClient = make_shared<CUser_RPCClient>(_channel);

		_btnRunClient.SetWindowTextW(L"关闭客户端");
		_btnRunAsyncClient.EnableWindow(FALSE);
		AppendMsg(L"客户端启动");

		// 创建定时器监测连接状态
		SetTimer(TIMER_CheckGRPC, 2000, nullptr);
	}
}

void CgRPCMFCClientDlg::OnBnClickedButtonRunAsyncClient()
{
	if (_user_AsyncRPCClient)
	{
		_user_AsyncRPCClient->Shutdown();
		_user_AsyncRPCClient = nullptr;
		_channel = nullptr;
		KillTimer(TIMER_CheckGRPC);
		_ctsCommon->cancel();

		AppendMsg(L"异步客户端已停止");
		_btnRunClient.EnableWindow(TRUE);
		_btnRunAsyncClient.SetWindowTextW(L"启动异步客户端");
	}
	else
	{
		// 创建异步客户端
		_channel = grpc::CreateChannel(_serverAddr, grpc::InsecureChannelCredentials()); // 创建gRPC channel
		_user_AsyncRPCClient = make_unique<CAsyncRPCClient>(_channel);
		_user_AsyncRPCClient->Run(this);

		AppendMsg(L"异步客户端启动 0.0.0.0:23351");
		_btnRunAsyncClient.SetWindowTextW(L"关闭异步客户端");
		_btnRunClient.EnableWindow(FALSE);

		// 创建定时器监测连接状态
		SetTimer(TIMER_CheckGRPC, 2000, nullptr);
	}
}

void CgRPCMFCClientDlg::OnBnClickedButtonGetuser()
{
	CString accountName;
	_editGetUser.GetWindowTextW(accountName);
	if (accountName.IsEmpty())
	{
		return;
	}

	USES_CONVERSION;
	shared_ptr<User> user = _user_RPCClient->GetUser(W2A(accountName));
	if (user)
	{
		CStringA strTmp;
		strTmp.Format("找到用户(%s) 昵称:%s Age:%d", user->accountname().c_str(), user->nickname().c_str(), user->age());
		AppendMsg(A2W(strTmp));
	}
	else
	{
		AppendMsg(L"没有找到该用户");
	}
}

void CgRPCMFCClientDlg::OnBnClickedButtonAsyncgetuser()
{
	CString accountName;
	_editGetUser.GetWindowTextW(accountName);
	if (accountName.IsEmpty())
	{
		return;
	}

	USES_CONVERSION;
	_user_AsyncRPCClient->GetUser(W2A(accountName));
}


void CgRPCMFCClientDlg::OnBnClickedButtonGetUsersByRole()
{
	shared_ptr<vector<User>> users = _user_RPCClient->GetUsersByRole(Role::USER);
	if (users)
	{
		USES_CONVERSION;
		CStringA strTmp;
		strTmp.Format("用户数为:%d", users->size());
		AppendMsg(A2W(strTmp));
	}
	else
	{
		AppendMsg(L"用户数为0");
	}
}

void CgRPCMFCClientDlg::OnBnClickedButtonAsyncGetUsersByRole()
{
	_user_AsyncRPCClient->GetUsersByRole(Role::USER);
}

void CgRPCMFCClientDlg::OnBnClickedButtonAddusers()
{
	static int num = 1;

	// 模拟批量创建用户
	shared_ptr<vector<User>> users = make_shared<vector<User>>();
	int count = 3;
	while (count)
	{
		int currentNum = num++;
		CStringA strAccountName, strNickname;
		strAccountName.Format("accountname%d", currentNum);
		strNickname.Format("nickname%d", currentNum);

		User user;
		user.set_accountname(strAccountName);
		user.set_accountpwd("pwd");
		user.set_age(currentNum);
		user.set_nickname(strNickname);
		user.set_userrole(Role::USER);
		users->push_back(move(user));

		count--;
	}

	shared_ptr<unsigned> userCount = _user_RPCClient->AddUsers(users);
	if (userCount)
	{
		USES_CONVERSION;
		CStringA strTmp;
		strTmp.Format("批量增加用户成功，用户总数为:%d", *userCount);
		AppendMsg(A2W(strTmp));
	}
	else
	{
		AppendMsg(L"创建用户失败");
	}
}

void CgRPCMFCClientDlg::OnBnClickedButtonAsyncAddusers()
{
	static int num = 1;

	// 模拟批量创建用户
	shared_ptr<vector<User>> users = make_shared<vector<User>>();
	int count = 3;
	while (count)
	{
		int currentNum = num++;
		CStringA strAccountName, strNickname;
		strAccountName.Format("accountname%d", currentNum);
		strNickname.Format("nickname%d", currentNum);

		User user;
		user.set_accountname(strAccountName);
		user.set_accountpwd("pwd");
		user.set_age(currentNum);
		user.set_nickname(strNickname);
		user.set_userrole(Role::USER);
		users->push_back(move(user));

		count--;
	}

	_user_AsyncRPCClient->AddUsers(users);
}

void CgRPCMFCClientDlg::OnBnClickedButtonDeleteusers()
{
	static int num = 1;

	// 模拟批量删除用户
	shared_ptr<vector<UserAccountName>> names = make_shared<vector<UserAccountName>>();
	int count = 2;
	while (count)
	{
		CStringA strAccountName;
		strAccountName.Format("accountname%d", num++);

		UserAccountName userAccountName;
		userAccountName.set_accountname(strAccountName);
		names->push_back(userAccountName);

		count--;
	}

	_user_RPCClient->DeleteUsers(names);
	AppendMsg(L"删除用户完成");
}

void CgRPCMFCClientDlg::OnBnClickedButtonAsyncdeleteusers()
{
	static int num = 1;

	// 模拟批量删除用户
	shared_ptr<vector<UserAccountName>> names = make_shared<vector<UserAccountName>>();
	int count = 2;
	while (count)
	{
		CStringA strAccountName;
		strAccountName.Format("accountname%d", num++);

		UserAccountName userAccountName;
		userAccountName.set_accountname(strAccountName);
		names->push_back(userAccountName);

		count--;
	}
	_user_AsyncRPCClient->DeleteUsers(names);
}

void CgRPCMFCClientDlg::OnGetUser(shared_ptr<User> user)
{
	if (user)
	{
		USES_CONVERSION;
		CStringA strTmp;
		strTmp.Format("找到用户(%s) 昵称:%s Age:%d", user->accountname().c_str(), user->nickname().c_str(), user->age());
		AppendMsg(A2W(strTmp));
	}
	else
	{
		AppendMsg(L"没有找到该用户");
	}
}

void CgRPCMFCClientDlg::OnGetUsersByRole(shared_ptr<User> user)
{
	USES_CONVERSION;
	CStringA strTmp;
	strTmp.Format("获取到指定角色的用户:%s", user->accountname().c_str());
	AppendMsg(A2W(strTmp));
}

void CgRPCMFCClientDlg::OnGetUsersByRoleComplete(bool isSucceed)
{
	if (isSucceed)
	{
		AppendMsg(L"获取到指定角色的用户 完成");
	}
	else
	{
		AppendMsg(L"获取到指定角色的用户 失败");
	}
}

void CgRPCMFCClientDlg::OnAddUsers(shared_ptr<string> userAccountName)
{
	USES_CONVERSION;
	CStringA strTmp;
	strTmp.Format("增加用户:%s 成功", userAccountName->c_str());
	AppendMsg(A2W(strTmp));
}

void CgRPCMFCClientDlg::OnAddUsersComplete(bool isSucceed, int count)
{
	if (isSucceed)
	{
		CString strTmp;
		strTmp.Format(L"批量增加用户完成，当前共%d个用户", count);
		AppendMsg(strTmp);
	}
	else
	{
		AppendMsg(L"批量增加用户 失败");
	}
}

void CgRPCMFCClientDlg::OnDeleteUsers(bool isSucceed, shared_ptr<string> msg)
{
	USES_CONVERSION;
	AppendMsg(A2W(msg->c_str()));
}

void CgRPCMFCClientDlg::OnDeleteUsersComplete()
{
	AppendMsg(L"批量删除用户 完成");
}

void CgRPCMFCClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_CheckGRPC)
	{
		task<void>([&] {
			auto token = _ctsCommon->get_token();
			if (!token.is_canceled() && _channel)
			{
				gpr_timespec tm_out{ 1, 0, GPR_TIMESPAN };
				bool isConnected = _channel->WaitForConnected<gpr_timespec>(tm_out);
				if (!isConnected)
				{
					AppendMsg(L"gRPC未连接");
				}
			}			
		});		
	}

	CDialogEx::OnTimer(nIDEvent);
}
