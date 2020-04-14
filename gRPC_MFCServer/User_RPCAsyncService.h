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

/* CAsyncRPCService 异步RPC服务
	包含启动、停止服务，以及演示数据
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
	vector<ServerCompletionQueue*> _vecCQ;
	vector<CompletionQueue*> _vecCQNewCall;

	// 数据
	map<string/*username*/, ::TestGRPC::User> _users;	// 所有用户信息
public:
	void Run(string serverAddr, int threadCount = 1);
	void Shutdown();

	map<string/*username*/, ::TestGRPC::User>& GetUsers() { return _users; }
};

/* CAsyncRPCResponder 异步RPC应答器基类
	包含启动、停止服务，以及演示数据
*/
class CAsyncRPCResponder 
{
public:
	// 当前运行状态
	enum class CallStatus { PROCESS, FINISH, DESTROY };

	CAsyncRPCResponder(UserService::AsyncService* service, CompletionQueue* cqNewCall, ServerCompletionQueue* cqNotification, CAsyncRPCService* dataService)
		: _asyncService(service), _cqNewCall(cqNewCall), _cqNotification(cqNotification), _dataService(dataService)
	{}

protected:

	CallStatus _callStatus = CallStatus::PROCESS;	// 当前运行状态

	int						_tag =  0;	// 数据进度标签
	CAsyncRPCService*		_dataService;	// 数据源
	bool		_isNewResponderCreated = false;	// 是否创建了新的应答器

	UserService::AsyncService*	_asyncService;
	CompletionQueue*			_cqNewCall;
	ServerCompletionQueue*		_cqNotification;
	ServerContext				_ctx;

public:
	virtual void OnNotification(bool isOK = true) = 0;
	virtual void OnNewCall(bool isOK = true) = 0;
};

// UserService::GetUser 一对一
class CAsyncRPCResponder_GetUser : public CAsyncRPCResponder
{
public:
	CAsyncRPCResponder_GetUser(UserService::AsyncService* service, CompletionQueue* cqNewCall, ServerCompletionQueue* cqNotification, CAsyncRPCService* dataService);

private:
	UserAccountName					_rqUserAccountName;
	ServerAsyncResponseWriter<User> _responder;

public:
	virtual void OnNotification(bool isOK = true);
	virtual void OnNewCall(bool isOK = true);
};

// UserService::GetUsersByRole  一对多
class CAsyncRPCResponder_GetUsersByRole : public CAsyncRPCResponder
{
public:
	CAsyncRPCResponder_GetUsersByRole(UserService::AsyncService* service, CompletionQueue* cqNewCall, ServerCompletionQueue* cqNotification, CAsyncRPCService* dataService);

private:
	UserRole						_rqUserRole;
	ServerAsyncWriter<User>			_responder;

	void Process();
public:
	virtual void OnNotification(bool isOK = true);
	virtual void OnNewCall(bool isOK = true);
};

// UserService::AddUsers 多对一
class CAsyncRPCResponder_AddUsers : public CAsyncRPCResponder
{
public:
	CAsyncRPCResponder_AddUsers(UserService::AsyncService* service, CompletionQueue* cqNewCall, ServerCompletionQueue* cqNotification, CAsyncRPCService* dataService);

private:
	User _tmpUser;
	ServerAsyncReader<CommonCount, User> _requester;

public:
	virtual void OnNotification(bool isOK = true);
	virtual void OnNewCall(bool isOK = true);
};

// UserService::DeleteUsers 多对多
class CAsyncRPCResponder_DeleteUsers : public CAsyncRPCResponder
{
public:
	CAsyncRPCResponder_DeleteUsers(UserService::AsyncService* service, CompletionQueue* cqNewCall, ServerCompletionQueue* cqNotification, CAsyncRPCService* dataService);

private:
	bool _isReadComplete = false;
	UserAccountName _tmpAccountName;
	vector<UserAccountName>	_rp_accountNames;	// 待返回的数据
	ServerAsyncReaderWriter<UserAccountName, UserAccountName> _requester;

public:
	virtual void OnNotification(bool isOK = true);
	virtual void OnNewCall(bool isOK = true);
};