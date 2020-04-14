#pragma once

#include <functional>
#include <string>

#include "User_RPCClient.h"
#include "User_RPCAsyncClient.h"

using namespace std;

class CgRPCMFCClientDlg : public CDialogEx
{
public:
	CgRPCMFCClientDlg(CWnd* pParent = nullptr);

	struct TheadFunc
	{
		function<void()> Func;
	};

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GRPC_MFCCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	HICON m_hIcon;
	CEdit _editRecv;
	CEdit _editGetUser;
	CButton _btnRunClient;
	CButton _btnRunAsyncClient;

	string _serverAddr = "localhost:23351";
	shared_ptr<Channel> _channel = nullptr;
	shared_ptr<CUser_RPCClient> _user_RPCClient = nullptr;
	unique_ptr<CAsyncRPCClient>	_user_AsyncRPCClient = nullptr;

	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

	void AppendMsg(const WCHAR* msg);
	LRESULT OnFunction(WPARAM wParam, LPARAM lParam);

	afx_msg void OnBnClickedButtonRunClient();
	afx_msg void OnBnClickedButtonGetuser();
	afx_msg void OnBnClickedButtonGetUsersByRole();
	afx_msg void OnBnClickedButtonAddusers();
	afx_msg void OnBnClickedButtonDeleteusers();
	afx_msg void OnBnClickedButtonRunAsyncClient();
	afx_msg void OnBnClickedButtonAsyncgetuser();
	afx_msg void OnBnClickedButtonAsyncGetUsersByRole();
	afx_msg void OnBnClickedButtonAsyncAddusers();
	afx_msg void OnBnClickedButtonAsyncdeleteusers();

public:
	void OnGetUser(shared_ptr<User> user);
	void OnGetUsersByRole(shared_ptr<User> user);
	void OnGetUsersByRoleComplete(bool isSucceed);
	void OnAddUsers(shared_ptr<string> userAccountName);
	void OnAddUsersComplete(bool isSucceed);
	void OnDeleteUsers(shared_ptr<string> userAccountName);
	void OnDeleteUsersComplete(bool isSucceed);
};
