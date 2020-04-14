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
	uint64_t timestamp(duration_cast<milliseconds>(chrono::system_clock::now().time_since_epoch()).count()); // ��ȡʱ��������룩

	uint64_t milli = timestamp + 8 * 60 * 60 * 1000; // תΪ����������ʱ��
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
		// ��ֹ�ظ�����
		return;
	}

	// ��������Ҫ��֤������
	_builder = make_unique<ServerBuilder>();
	_builder->AddListeningPort(serverAddr, grpc::InsecureServerCredentials());
	// ע�����
	_asyncUserService = make_unique<UserService::AsyncService>();
	_builder->RegisterService(_asyncUserService.get());

	while (threadCount)
	{
		// ��Ӷ��У�������BuildAndStart֮ǰ��
		ServerCompletionQueue* cqNotification = _builder->AddCompletionQueue().release();
		_vecCQ.push_back(cqNotification);

		threadCount--;
	}

	// ��������
	_rpcServer = _builder->BuildAndStart();

	// ��������
	auto token = _ctsCommon->get_token();
	for each (auto cq in _vecCQ)
	{
		CompletionQueue* cqNewCall = new CompletionQueue;
		_vecCQNewCall.push_back(cqNewCall);

		// ����Ӧ��������
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
				if (!cq->Next(&tag, &ok)) // ������ֱ�����µ��¼�
				{
					// ���ִ��󣬷���ֹͣ
					str.clear();
					str << GetTimeStr() << "CUser_RPCAsyncService::RunServer()taskProceed:: Next() failed" << endl;
					OutputDebugStringA(str.str().c_str());

					break;
				}

				// �����¼�
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
				if (!cqNewCall->Next(&tag, &ok)) // ������ֱ�����µ��¼�
				{
					// ���ִ��󣬷���ֹͣ
					str.clear();
					str << GetTimeStr() << "CUser_RPCAsyncService::RunServer() taskOnNewCall::Next() failed" << endl;
					OutputDebugStringA(str.str().c_str());

					break;
				}

				// �����¼�
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
			// �����µ�Ӧ��������
			if (!_isNewResponderCreated)
			{
				_isNewResponderCreated = true;
				new CAsyncRPCResponder_GetUser(_asyncService, _cqNewCall, _cqNotification, _dataService);
			}

			// ��������
			_callStatus = CallStatus::FINISH;

			auto& users = _dataService->GetUsers();
			User user;
			if (users.find(_rqUserAccountName.accountname()) != users.end())
			{
				user.CopyFrom(users[_rqUserAccountName.accountname()]);

				// �������
				_responder.Finish(user, Status::OK, this);
			}
			else
			{
				// �������
				_responder.Finish(user, Status(NOT_FOUND, "accountname not found"), this);
			}
		}
		else
		{
			_callStatus = CallStatus::DESTROY;
			delete this; // Ӧ�����
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
		delete this; // Ӧ�����
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
			// �����µ�Ӧ��������
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
			delete this; // Ӧ�����
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
			// �������
			_callStatus = CallStatus::FINISH;
			_responder.Finish(Status(CANCELLED, "error"), this);
		}
	}
	break;
	case CallStatus::FINISH:
	{
		_callStatus = CallStatus::DESTROY;
		delete this; // Ӧ�����
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
			++iter; // ���ݱ�ǵ�������ָ��
		}

		while (true)
		{
			if (iter != users.end())
			{
				_tag++; // ��ǵ�ǰ���ͽ���
				if (iter->second.userrole() == _rqUserRole.role())
				{
					// ����һ�λظ�
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
				// �������
				_callStatus = CallStatus::FINISH;
				_responder.Finish(Status::OK, this);
				break;
			}
		}
	}
	else
	{
		// �������
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
			// �����µ�Ӧ��������
			if (!_isNewResponderCreated)
			{
				_isNewResponderCreated = true;
				new CAsyncRPCResponder_AddUsers(_asyncService, _cqNewCall, _cqNotification, _dataService);
			}

			// ��һ�εȴ�����
			_requester.Read(&_tmpUser, this); // ���ͻ��˵��������ݴ����Ա����_tmpUser�У�����ʹ����ʱ������
		}
		else
		{
			_callStatus = CallStatus::DESTROY;
			delete this; // Ӧ�����
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
			// ������յ���һ��stream����
			auto& users = _dataService->GetUsers();

			string accountName = _tmpUser.accountname();
			if (users.find(accountName) != users.end()) // ����û��Ƿ��Ѵ���
			{
				users[accountName].CopyFrom(_tmpUser);	// �����û�����
			}
			else
			{
				users.insert(map<string, User>::value_type(accountName, move(_tmpUser))); // �������û�
			}

			// �����ȴ�����
			_requester.Read(&_tmpUser, this); // ���ͻ��˵��������ݴ����Ա����_tmpUser�У�����ʹ����ʱ������
		}
		else
		{
			_callStatus = CallStatus::FINISH;

			// �ظ�
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
		delete this; // Ӧ�����
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
		// ��ʼ��ȡ
		if (isOK)
		{
			// �����µ�Ӧ��������
			if (!_isNewResponderCreated)
			{
				_isNewResponderCreated = true;
				new CAsyncRPCResponder_DeleteUsers(_asyncService, _cqNewCall, _cqNotification, _dataService);
			}

			// ��һ�εȴ�����
			_requester.Read(&_tmpAccountName, this); // ������ʱ��Ա����tmpAccountName�У�����ʹ����ʱ������
		}
		else
		{
			_callStatus = CallStatus::DESTROY;
			delete this; // Ӧ�����
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
		// ��ʼ��ȡ
		if (!_isReadComplete)
		{
			if (isOK)
			{
				// ������յ���һ��stream����
				auto& users = _dataService->GetUsers();
				if (users.find(_tmpAccountName.accountname()) != users.end()) // ����û��Ƿ��Ѵ���
				{
					ostringstream str;
					str << GetTimeStr() << "CAsyncRPCResponder_DeleteUsers:: read:" << _tmpAccountName.accountname() << endl;
					OutputDebugStringA(str.str().c_str());

					// ������ɾ�����û�����
					users.erase(_tmpAccountName.accountname());

					// ����ɾ�����û���
					_rp_accountNames.push_back(move(_tmpAccountName));
				}

				// �����ȴ�����
				_requester.Read(&_tmpAccountName, this); // ������ʱ��Ա����tmpAccountName�У�����ʹ����ʱ������
			}
			else
			{
				// ��ȡ����
				_isReadComplete = true;
				isOK = true;
			}
		}

		// ��ȡ��������ʼ�ظ�
		if (_isReadComplete)
		{
			if (isOK)
			{
				auto iter = _rp_accountNames.begin();
				for (int i = 0; i != _tag; ++i)
				{
					++iter; // ���ݱ�ǵ�������ָ��
				}

				if (iter != _rp_accountNames.end())
				{
					_tag++; // ��ǵ�ǰ���ͽ���

					_requester.Write(*iter, this);
				}
				else
				{
					// �������
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
		delete this; // Ӧ�����
		break;
	default:
		assert(false);
		break;
	}
}
