#pragma once
#include <map>
#include <grpcpp/grpcpp.h>
#include <pplx/pplxtasks.h>
#include "../proto/User/User.grpc.pb.h"

using namespace std;
using namespace Concurrency;
using namespace TestGRPC;
using namespace grpc;
using grpc::Server;
using grpc::ServerCompletionQueue;

/* CAsyncRPCService �첽RPC����
	����������ֹͣ�����Լ���ʾ����
*/
class CAsyncRPCService
{
public:
	CAsyncRPCService();

private:
	unique_ptr<Server> _rpcServer = nullptr;
	unique_ptr<ServerBuilder> _builder = nullptr;
	unique_ptr<UserService::AsyncService> _asyncUserService = nullptr;
	unique_ptr<cancellation_token_source> _ctsCommon = make_unique<cancellation_token_source>();
	vector<ServerCompletionQueue*> _vecSCQ;
	vector<CompletionQueue*> _vecCQNewCall;

	// ����
	map<string/*username*/, ::TestGRPC::User> _users;	// �����û���Ϣ
public:
	void Run(string serverAddr, int threadCount = 1);
	void Shutdown();

	map<string/*username*/, ::TestGRPC::User>& GetUsers() { return _users; }
};

/* CAsyncRPCResponder �첽RPCӦ��������
*/
class CAsyncRPCResponder 
{
public:
	// ��ǰ����״̬
	enum class CallStatus { PROCESS, FINISH };

	CAsyncRPCResponder(UserService::AsyncService* service, CompletionQueue* cqNewCall, ServerCompletionQueue* scqNotification, CAsyncRPCService* dataService)
		: _svcUser(service), _cqNewCall(cqNewCall), _scqNotification(scqNotification), _dataService(dataService)
	{}

protected:

	CallStatus _callStatus = CallStatus::PROCESS;	// ��ǰ����״̬
	CAsyncRPCService*		_dataService;	// ����Դ
	bool		_isFirstCalled = true;	// �Ƿ��һ�α�����

	UserService::AsyncService*	_svcUser;

	CompletionQueue*			_cqNewCall;
	ServerCompletionQueue*		_scqNotification;
	ServerContext				_ctx;

public:
	virtual void OnNotification(bool isOK = true) = 0;
};

// UserService::GetUser һ��һ
class CAsyncRPCResponder_GetUser : public CAsyncRPCResponder
{
public:
	CAsyncRPCResponder_GetUser(UserService::AsyncService* service, CompletionQueue* cqNewCall, ServerCompletionQueue* cqNotification, CAsyncRPCService* dataService);

private:
	UserAccountName					_rqUserAccountName;
	ServerAsyncResponseWriter<User> _responder;

public:
	virtual void OnNotification(bool isOK = true);
};

// UserService::GetUsersByRole  һ�Զ�
class CAsyncRPCResponder_GetUsersByRole : public CAsyncRPCResponder
{
public:
	CAsyncRPCResponder_GetUsersByRole(UserService::AsyncService* service, CompletionQueue* cqNewCall, ServerCompletionQueue* cqNotification, CAsyncRPCService* dataService);

private:
	UserRole						_rqUserRole;
	ServerAsyncWriter<User>			_responder;
	int								_tag = 0;	// ���ݽ��ȱ�ǩ

public:
	virtual void OnNotification(bool isOK = true);
};

// UserService::AddUsers ���һ
class CAsyncRPCResponder_AddUsers : public CAsyncRPCResponder
{
public:
	CAsyncRPCResponder_AddUsers(UserService::AsyncService* service, CompletionQueue* cqNewCall, ServerCompletionQueue* cqNotification, CAsyncRPCService* dataService);

private:
	User _tmpUser;
	ServerAsyncReader<CommonNumber, User> _requester;

public:
	virtual void OnNotification(bool isOK = true);
};

// UserService::DeleteUsers ��Զ�
class CAsyncRPCResponder_DeleteUsers : public CAsyncRPCResponder
{
public:
	CAsyncRPCResponder_DeleteUsers(UserService::AsyncService* service, CompletionQueue* cqNewCall, ServerCompletionQueue* cqNotification, CAsyncRPCService* dataService);

private:
	bool	_isWriteDone = true;	// ��ǰΪ��ģʽ
	int		_tag = 0;	// ���ݽ��ȱ�ǩ
	UserAccountName _rqAccountName;
	ServerAsyncReaderWriter<CommonMsg, UserAccountName> _requester;

public:
	virtual void OnNotification(bool isOK = true);
};