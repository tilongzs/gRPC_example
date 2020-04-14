#pragma once

#include <grpcpp/grpcpp.h>
#include <grpcpp/client_context.h>

#include "../proto/User/User.grpc.pb.h"

using namespace grpc;
using namespace TestGRPC;
using namespace std;

class CgRPCMFCClientDlg;
class CAsyncRPCClient
{
private:
	CgRPCMFCClientDlg* _mainDlg = nullptr;
	CompletionQueue		_cq;
	std::unique_ptr<UserService::Stub> _stub;
	shared_ptr<Channel> _channel = nullptr;

public:
	void Run(CgRPCMFCClientDlg* mainDlg, string serverAddr);
	void GetUser(const string& accountName);
	void GetUsersByRole(const Role& role);
	void AddUsers(shared_ptr<vector<User>> users);
	void DeleteUsers(shared_ptr<vector<UserAccountName>> accountNames);
};

class CAsyncRPCRequester
{
public:
	// ��ǰ����״̬
	enum class CallStatus {PROCESS, FINISH };

protected:
	CallStatus	_callStatus = CallStatus::PROCESS;	// ��ǰ����״̬
	int				_tag;	// ���н��ȱ�ǩ

	ClientContext					_ctx;
	Status							_status;
public:
	virtual void Proceed(bool isOK = true) = 0;
};

// GetUser һ��һ
class CAsyncRPC_GetUser : public CAsyncRPCRequester
{
public:
	CAsyncRPC_GetUser(CgRPCMFCClientDlg* mainDlg, UserService::Stub* stub, CompletionQueue& cq,  UserAccountName& request);

private:
	CgRPCMFCClientDlg* _mainDlg = nullptr;
	unique_ptr<ClientAsyncResponseReader<User>> _responsder;
	User	_reply; // �洢�յ�������
public:
	virtual void Proceed(bool isOK = true);
};

// GetAllUsers һ�Զ�
class CAsyncRPC_GetUsersByRole : public CAsyncRPCRequester
{
public:
	CAsyncRPC_GetUsersByRole(CgRPCMFCClientDlg* mainDlg, UserService::Stub* stub, CompletionQueue& cq, UserRole& request);

private:
	CgRPCMFCClientDlg* _mainDlg = nullptr;
	User	_reply;
	unique_ptr<ClientAsyncReader<User>> _responsder;

	bool _isFirstRead = true;
public:
	virtual void Proceed(bool isOK = true);
};

// AddUsers ���һ
class CAsyncRPC_AddUsers : public CAsyncRPCRequester
{
public:
	CAsyncRPC_AddUsers(CgRPCMFCClientDlg* mainDlg, UserService::Stub* stub, CompletionQueue& cq, shared_ptr<vector<User>>& request);

private:
	CgRPCMFCClientDlg* _mainDlg = nullptr;
	shared_ptr<vector<User>>	_request; // �洢��������ָ��

	bool _isSendComplete = false;
	unique_ptr<ClientAsyncWriter<User>> _responsder;
	CommonCount _reply;	// �洢�յ�������
public:
	virtual void Proceed(bool isOK = true);
};

// DeleteUsers ��Զ�
class CAsyncRPC_DeleteUsers : public CAsyncRPCRequester
{
public:
	CAsyncRPC_DeleteUsers(CgRPCMFCClientDlg* mainDlg, UserService::Stub* stub, CompletionQueue& cq, shared_ptr<vector<UserAccountName>>& request);

private:
	CgRPCMFCClientDlg* _mainDlg = nullptr;
	shared_ptr<vector<UserAccountName>>	_request; // �洢��������ָ��
	vector<UserAccountName> _deletedUserAccountName;	// �洢���յ�������

	bool _isFirstRead = true;
	bool _isSendComplete = false;
	unique_ptr<ClientAsyncReaderWriter<UserAccountName, UserAccountName>> _responsder;
	UserAccountName _reply;	// �洢�յ�������
public:
	virtual void Proceed(bool isOK = true);
};