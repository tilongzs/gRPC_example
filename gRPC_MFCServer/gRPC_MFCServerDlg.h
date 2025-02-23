#pragma once
#include "User_RPCAsyncService.h"

#include <functional>
#include <string>
#include <memory>

#include <grpcpp/server_builder.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

using std::string;
using std::unique_ptr;
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
	unique_ptr<Server> _rpcServer = nullptr;	// 同步模式服务
	unique_ptr<CAsyncRPCService> _asyncRPCServer = nullptr;

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

	// 打印日志
	void AppendMsg(const WCHAR* msg);
	LRESULT OnFunction(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnBnClickedButtonRunServer();
	afx_msg void OnBnClickedButtonRunAsyncServer();
};
