#include "pch.h"
#include "User_RPCAsyncService.h"
#include <grpcpp/server_builder.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <pplx/pplxtasks.h>
#include <chrono>
#include <sstream>
using namespace Concurrency;
using namespace chrono;

static string GetTimeStr()
{
	uint64_t timestamp(duration_cast<milliseconds>(chrono::system_clock::now().time_since_epoch()).count()); // 获取时间戳（毫秒）

	uint64_t milli = timestamp + 8 * 60 * 60 * 1000; // 转为东八区北京时间
	auto mTime = milliseconds(milli);
	auto tp = time_point<system_clock, milliseconds>(mTime);
	auto tt = system_clock::to_time_t(tp);
	tm now;
	gmtime_s(&now, &tt);
	char str[60] = { 0 };
	sprintf_s(str, "%02d:%02d:%02d.%03d ", now.tm_hour, now.tm_min, now.tm_sec, int(timestamp % 1000));
	return str;
}

CAsyncRPCService::CAsyncRPCService()
{
	grpc::EnableDefaultHealthCheckService(true);
	grpc::reflection::InitProtoReflectionServerBuilderPlugin();
}

void CAsyncRPCService::Run(string serverAddr, int threadCount /*= 1*/)
{
	if (_builder)
	{
		// 防止重复运行
		return;
	}

	// 监听不需要认证的连接
	_builder = make_unique<ServerBuilder>();
	_builder->AddListeningPort(serverAddr, grpc::InsecureServerCredentials());
	// 注册服务
	_asyncUserService = make_unique<UserService::AsyncService>();
	_builder->RegisterService(_asyncUserService.get());

	while (threadCount)
	{
		// 添加队列（必须在BuildAndStart之前）
		ServerCompletionQueue* cqNotification = _builder->AddCompletionQueue().release();
		_vecCQ.push_back(cqNotification);

		threadCount--;
	}

	// 启动服务
	_rpcServer = _builder->BuildAndStart();

	// 服务运行
	auto token = _ctsCommon->get_token();
	for each (auto cq in _vecCQ)
	{
		CompletionQueue* cqNewCall = new CompletionQueue;
		_vecCQNewCall.push_back(cqNewCall);

		// 创建应答器对象
		new CAsyncRPCResponder_GetUser(_asyncUserService.get(), cqNewCall, cq, this);
		new CAsyncRPCResponder_GetUsersByRole(_asyncUserService.get(), cqNewCall, cq, this);
		new CAsyncRPCResponder_AddUsers(_asyncUserService.get(), cqNewCall, cq, this);
		new CAsyncRPCResponder_DeleteUsers(_asyncUserService.get(), cqNewCall, cq, this);

		task<void> taskProceed([&, cq, token]
		{
			ostringstream str;
			void* tag = nullptr;
			bool ok = false;
			while (!token.is_canceled())
			{
				if (!cq->Next(&tag, &ok)) // 阻塞，直至有新的事件
				{
					// 出现错误，服务停止
					str.clear();
					str << GetTimeStr() << "CUser_RPCAsyncService::RunServer()taskProceed:: Next() failed" << endl;
					OutputDebugStringA(str.str().c_str());

					break;
				}

				// 处理事件
				static_cast<CAsyncRPCResponder*>(tag)->OnNotification(ok);
			}

			cq->Shutdown();
			delete cq;

			str.clear();
			str << GetTimeStr() << "CUser_RPCAsyncService::RunServer() taskEnd" << endl;
			OutputDebugStringA(str.str().c_str());
		});

		task<void> taskOnNewCall([&, cqNewCall, token]
		{
			ostringstream str;
			void* tag = nullptr;
			bool ok = false;
			while (!token.is_canceled())
			{
				if (!cqNewCall->Next(&tag, &ok)) // 阻塞，直至有新的事件
				{
					// 出现错误，服务停止
					str.clear();
					str << GetTimeStr() << "CUser_RPCAsyncService::RunServer() taskOnNewCall::Next() failed" << endl;
					OutputDebugStringA(str.str().c_str());

					break;
				}

				// 处理事件
				static_cast<CAsyncRPCResponder*>(tag)->OnNewCall(ok);
			}

			cqNewCall->Shutdown();
			delete cqNewCall;

			str.clear();
			str << GetTimeStr() << "CUser_RPCAsyncService::RunServer() taskEnd" << endl;
			OutputDebugStringA(str.str().c_str());
		});
	}
}

void CAsyncRPCService::Shutdown()
{
	if (!_builder)
	{
		return;
	}

	_rpcServer->Shutdown();

	for each (auto iter in _vecCQ)
	{
		iter->Shutdown();
	}

	for each (auto iter in _vecCQNewCall)
	{
		iter->Shutdown();
	}

	_ctsCommon->cancel();
	_asyncUserService = nullptr;
	_builder = nullptr;
}

CAsyncRPCResponder_GetUser::CAsyncRPCResponder_GetUser(UserService::AsyncService* service, CompletionQueue* cqNewCall, ServerCompletionQueue* cqNotification, CAsyncRPCService* dataService)
	: CAsyncRPCResponder(service, cqNewCall, cqNotification, dataService), _responder(&_ctx)
{
	_asyncService->RequestGetUser(&_ctx, &_rqUserAccountName, &_responder, _cqNewCall, _cqNotification, this);
}

void CAsyncRPCResponder_GetUser::OnNotification(bool isOK)
{
	switch (_callStatus)
	{
	case CallStatus::PROCESS:
	{
		if (isOK)
		{
			// 创建新的应答器对象
			if (!_isNewResponderCreated)
			{
				_isNewResponderCreated = true;
				new CAsyncRPCResponder_GetUser(_asyncService, _cqNewCall, _cqNotification, _dataService);
			}

			// 处理数据
			_callStatus = CallStatus::FINISH;

			auto& users = _dataService->GetUsers();
			User user;
			if (users.find(_rqUserAccountName.accountname()) != users.end())
			{
				user.CopyFrom(users[_rqUserAccountName.accountname()]);

				// 发送完成
				_responder.Finish(user, Status::OK, this);
			}
			else
			{
				// 发送完成
				_responder.Finish(user, Status(NOT_FOUND, "accountname not found"), this);
			}
		}
		else
		{
			_callStatus = CallStatus::DESTROY;
			delete this; // 应答结束
		}
	}
	break;
	default:
		assert(false);
		break;
	}
}

void CAsyncRPCResponder_GetUser::OnNewCall(bool isOK)
{
	switch (_callStatus)
	{
	case CallStatus::FINISH:
	{
		_callStatus = CallStatus::DESTROY;
		delete this; // 应答结束
	}
	break;
	default:
		assert(false);
		break;
	}
}

CAsyncRPCResponder_GetUsersByRole::CAsyncRPCResponder_GetUsersByRole(UserService::AsyncService* service, CompletionQueue* cqNewCall, ServerCompletionQueue* cqNotification, CAsyncRPCService* dataService)
	: CAsyncRPCResponder(service, cqNewCall, cqNotification, dataService), _responder(&_ctx)
{
	_asyncService->RequestGetUsersByRole(&_ctx, &_rqUserRole, &_responder, _cqNewCall, _cqNotification, this);
}

void CAsyncRPCResponder_GetUsersByRole::OnNotification(bool isOK)
{
	switch (_callStatus)
	{
	case CallStatus::PROCESS:
	{
		if (isOK)
		{
			// 创建新的应答器对象
			if (!_isNewResponderCreated)
			{
				_isNewResponderCreated = true;
				new CAsyncRPCResponder_GetUsersByRole(_asyncService, _cqNewCall, _cqNotification, _dataService);
			}

			Process();
		}
		else
		{
			_callStatus = CallStatus::DESTROY;
			delete this; // 应答结束
		}
	}
		break;
	default:
		assert(false);
		break;
	}
}

void CAsyncRPCResponder_GetUsersByRole::OnNewCall(bool isOK /*= true*/)
{
	switch (_callStatus)
	{
	case CallStatus::PROCESS:
	{
		if (isOK)
		{
			Process();
		}
		else
		{
			// 发送完成
			_callStatus = CallStatus::FINISH;
			_responder.Finish(Status(CANCELLED, "error"), this);
		}
	}
	break;
	case CallStatus::FINISH:
	{
		_callStatus = CallStatus::DESTROY;
		delete this; // 应答结束
	}
	break;
	default:
		assert(false);
		break;
	}
}

void CAsyncRPCResponder_GetUsersByRole::Process()
{
	auto& users = _dataService->GetUsers();
	if (!users.empty())
	{
		auto iter = users.begin();
		for (int i = 0; i != _tag; ++i)
		{
			++iter; // 根据标记调整数据指针
		}

		while (true)
		{
			if (iter != users.end())
			{
				_tag++; // 标记当前发送进度
				if (iter->second.userrole() == _rqUserRole.role())
				{
					// 发送一次回复
					_responder.Write(iter->second, this);
					break;
				}
				else
				{
					++iter;
				}
			}
			else
			{
				// 发送完成
				_callStatus = CallStatus::FINISH;
				_responder.Finish(Status::OK, this);
				break;
			}
		}
	}
	else
	{
		// 发送完成
		_callStatus = CallStatus::FINISH;
		_responder.Finish(Status(NOT_FOUND, "no user"), this);
	}
}

CAsyncRPCResponder_AddUsers::CAsyncRPCResponder_AddUsers(UserService::AsyncService* service, CompletionQueue* cqNewCall, ServerCompletionQueue* cqNotification, CAsyncRPCService* dataService)
	: CAsyncRPCResponder(service, cqNewCall, cqNotification, dataService), _requester(&_ctx)
{
	_asyncService->RequestAddUsers(&_ctx, &_requester, _cqNewCall, _cqNotification, this);
}

void CAsyncRPCResponder_AddUsers::OnNotification(bool isOK)
{
	switch (_callStatus)
	{
	case CallStatus::PROCESS:
	{
		if (isOK)
		{
			// 创建新的应答器对象
			if (!_isNewResponderCreated)
			{
				_isNewResponderCreated = true;
				new CAsyncRPCResponder_AddUsers(_asyncService, _cqNewCall, _cqNotification, _dataService);
			}

			// 第一次等待数据
			_requester.Read(&_tmpUser, this); // 将客户端的请求数据存入成员变量_tmpUser中，不能使用临时变量！
		}
		else
		{
			_callStatus = CallStatus::DESTROY;
			delete this; // 应答结束
		}
	}
		break;
	default:
		assert(false);
		break;
	}
}

void CAsyncRPCResponder_AddUsers::OnNewCall(bool isOK)
{
	switch (_callStatus)
	{
	case CallStatus::PROCESS:
	{
		if (isOK)
		{
			// 处理接收到的一次stream数据
			auto& users = _dataService->GetUsers();

			string accountName = _tmpUser.accountname();
			if (users.find(accountName) != users.end()) // 检查用户是否已存在
			{
				users[accountName].CopyFrom(_tmpUser);	// 更新用户数据
			}
			else
			{
				users.insert(map<string, User>::value_type(accountName, move(_tmpUser))); // 插入新用户
			}

			// 继续等待数据
			_requester.Read(&_tmpUser, this); // 将客户端的请求数据存入成员变量_tmpUser中，不能使用临时变量！
		}
		else
		{
			_callStatus = CallStatus::FINISH;

			// 回复
			auto& users = _dataService->GetUsers();
			CommonCount userCount;
			userCount.set_count(users.size());
			_requester.Finish(userCount, Status::OK, this);
		}
	}
	break;
	case CallStatus::FINISH:
	{
		_callStatus = CallStatus::DESTROY;
		delete this; // 应答结束
	}
	break;
	default:
		assert(false);
		break;
	}
}

CAsyncRPCResponder_DeleteUsers::CAsyncRPCResponder_DeleteUsers(UserService::AsyncService* service, CompletionQueue* cqNewCall, ServerCompletionQueue* cqNotification, CAsyncRPCService* dataService)
	: CAsyncRPCResponder(service, cqNewCall, cqNotification, dataService), _requester(&_ctx)
{
	_asyncService->RequestDeleteUsers(&_ctx, &_requester, _cqNewCall, _cqNotification, this);
}

void CAsyncRPCResponder_DeleteUsers::OnNotification(bool isOK /*= true*/)
{
	switch (_callStatus)
	{
	case CallStatus::PROCESS:
	{
		// 开始读取
		if (isOK)
		{
			// 创建新的应答器对象
			if (!_isNewResponderCreated)
			{
				_isNewResponderCreated = true;
				new CAsyncRPCResponder_DeleteUsers(_asyncService, _cqNewCall, _cqNotification, _dataService);
			}

			// 第一次等待数据
			_requester.Read(&_tmpAccountName, this); // 存入临时成员变量tmpAccountName中，不能使用临时变量！
		}
		else
		{
			_callStatus = CallStatus::DESTROY;
			delete this; // 应答结束
		}
	}
		break;
	default:
		assert(false);
		break;
	}
}

void CAsyncRPCResponder_DeleteUsers::OnNewCall(bool isOK)
{
	switch (_callStatus)
	{
	case CallStatus::PROCESS:
	{
		// 开始读取
		if (!_isReadComplete)
		{
			if (isOK)
			{
				// 处理接收到的一次stream数据
				auto& users = _dataService->GetUsers();
				if (users.find(_tmpAccountName.accountname()) != users.end()) // 检查用户是否已存在
				{
					ostringstream str;
					str << GetTimeStr() << "CAsyncRPCResponder_DeleteUsers:: read:" << _tmpAccountName.accountname() << endl;
					OutputDebugStringA(str.str().c_str());

					// 服务器删除该用户数据
					users.erase(_tmpAccountName.accountname());

					// 保存删除的用户名
					_rp_accountNames.push_back(move(_tmpAccountName));
				}

				// 继续等待数据
				_requester.Read(&_tmpAccountName, this); // 存入临时成员变量tmpAccountName中，不能使用临时变量！
			}
			else
			{
				// 读取结束
				_isReadComplete = true;
				isOK = true;
			}
		}

		// 读取结束，开始回复
		if (_isReadComplete)
		{
			if (isOK)
			{
				auto iter = _rp_accountNames.begin();
				for (int i = 0; i != _tag; ++i)
				{
					++iter; // 根据标记调整数据指针
				}

				if (iter != _rp_accountNames.end())
				{
					_tag++; // 标记当前发送进度

					_requester.Write(*iter, this);
				}
				else
				{
					// 发送完成
					_callStatus = CallStatus::FINISH;
					_requester.Finish(Status::OK, this);
				}
			}
			else
			{
				_callStatus = CallStatus::FINISH;
				_requester.Finish(Status(), (void*)this);
			}
		}
	}
	break;
	case CallStatus::FINISH:
		delete this; // 应答结束
		break;
	default:
		assert(false);
		break;
	}
}
