#pragma once
#include "User_RPCAsyncService.h"

#include <functional>
#include <string>
#include <memory>
#include <ppltasks.h>

#include <grpcpp/server_builder.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

using namespace Concurrency;
using namespace std;
using namespace grpc;

class CgRPCMFCServerDlg : public CDialogEx
{
public:
	CgRPCMFCServerDlg(CWnd* pParent = nullptr);

	struct TheadFunc
	{
		function<void()> Func;
	};

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GRPC_MFCSERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);


protected:
	HICON m_hIcon;
	CEdit _editRecv;
	CButton _btnRunServer;
	CButton _btnRunAsyncServer;

	string _serverAddr = "0.0.0.0:23351";
	unique_ptr<Server> _rpcServer = nullptr;
	unique_ptr<CAsyncRPCService> _asyncRPCServer = nullptr;
	unique_ptr<cancellation_token_source> _ctsCommon = make_unique<cancellation_token_source>();

	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()


	void AppendMsg(const WCHAR* msg);
	LRESULT OnFunction(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnBnClickedButtonRunServer();
	afx_msg void OnBnClickedButtonRunAsyncServer();
};
