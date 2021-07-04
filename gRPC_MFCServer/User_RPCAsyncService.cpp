#include "pch.h"
#include "User_RPCAsyncService.h"
#include <grpcpp/server_builder.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <chrono>
#include <sstream>
#include <fstream>
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

	_builder = make_unique<ServerBuilder>();

	/*
	auto GetFileString = [&](string extraFilePath)
	{
		char buf[MAX_PATH] = { 0 };
		GetModuleFileNameA(NULL, buf, MAX_PATH * sizeof(char));
		GetLongPathNameA(buf, buf, MAX_PATH * sizeof(char));
		PathRemoveFileSpecA(buf);
		PathAppendA(buf, extraFilePath.c_str());

		ifstream in(buf);
		istreambuf_iterator<char> beg(in), end;
		string str(beg, end);
		return str;
	};

	// ����SSL��֤������
	grpc::SslServerCredentialsOptions sslOpts{};
	sslOpts.client_certificate_request = GRPC_SSL_REQUEST_AND_REQUIRE_CLIENT_CERTIFICATE_AND_VERIFY;
	sslOpts.pem_key_cert_pairs.push_back(grpc::SslServerCredentialsOptions::PemKeyCertPair{ GetFileString("ssl/server.key"), GetFileString("ssl/server.crt") });
	sslOpts.pem_root_certs = GetFileString("ssl/client.crt");
	auto creds = grpc::SslServerCredentials(sslOpts);
	_builder->AddListeningPort(serverAddr, creds);
	*/

	// ��������Ҫ��֤������
	_builder->AddListeningPort(serverAddr, grpc::InsecureServerCredentials());

	// ע�����
	_asyncUserService = make_unique<UserService::AsyncService>();
	_builder->RegisterService(_asyncUserService.get());

	while (threadCount)
	{
		// ��Ӷ��У�������BuildAndStart֮ǰ��
		ServerCompletionQueue* cqNotification = _builder->AddCompletionQueue().release();
		_vecSCQ.push_back(cqNotification);

		threadCount--;
	}

	// ��������
	_rpcServer = _builder->BuildAndStart();

	// ��������
	auto token = _ctsCommon->get_token();
	for each (auto scq in _vecSCQ)
	{
// 		CompletionQueue* cqNewCall = new CompletionQueue;
// 		_vecCQNewCall.push_back(cqNewCall);

		// ����Ӧ��������
		new CAsyncRPCResponder_GetUser(_asyncUserService.get(), scq, scq, this);
		new CAsyncRPCResponder_GetUsersByRole(_asyncUserService.get(), scq, scq, this);
		new CAsyncRPCResponder_AddUsers(_asyncUserService.get(), scq, scq, this);
		new CAsyncRPCResponder_DeleteUsers(_asyncUserService.get(), scq, scq, this);

		task<void> taskProceed([&, scq, token]
		{
			ostringstream str;
			void* tag = nullptr;
			bool ok = false;
			while (!token.is_canceled())
			{
				if (!scq->Next(&tag, &ok)) // ������ֱ�����µ��¼�
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

			scq->Shutdown();
			delete scq;

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

	for each (auto iter in _vecSCQ)
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
	_svcUser->RequestGetUser(&_ctx, &_rqUserAccountName, &_responder, _cqNewCall, _scqNotification, this);
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
			if (_isFirstCalled)
			{
				_isFirstCalled = false;
				new CAsyncRPCResponder_GetUser(_svcUser, _cqNewCall, _scqNotification, _dataService);
			}

			// ��������
			_callStatus = CallStatus::FINISH;

			auto& users = _dataService->GetUsers();
			User user;
			if (users.find(_rqUserAccountName.accountname()) != users.end())
			{
				user.CopyFrom(users[_rqUserAccountName.accountname()]);

				// �ظ�
				_responder.Finish(user, Status::OK, this);
			}
			else
			{
				// �ظ�
				_responder.Finish(user, Status(NOT_FOUND, "accountname not found"), this);
			}
		}
		else if (_isFirstCalled)
		{
			delete this;
		}
		else
		{
			_callStatus = CallStatus::FINISH;
		}
	}
	break;
	case CallStatus::FINISH:
	{
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
	_svcUser->RequestGetUsersByRole(&_ctx, &_rqUserRole, &_responder, _cqNewCall, _scqNotification, this);
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
			if (_isFirstCalled)
			{
				_isFirstCalled = false;
				new CAsyncRPCResponder_GetUsersByRole(_svcUser, _cqNewCall, _scqNotification, _dataService);
			}

			// ��������
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
							// �ظ�һ��stream����
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
						// ȫ���ظ����
						_callStatus = CallStatus::FINISH;
						_responder.Finish(Status::OK, this);
						break;
					}
				}
			}
			else
			{
				// ȫ���ظ����
				_callStatus = CallStatus::FINISH;
				_responder.Finish(Status(NOT_FOUND, "not found"), this);
			}
		}
		else if (_isFirstCalled)
		{
			delete this;
		}
		else
		{
			_callStatus = CallStatus::FINISH;
			_responder.Finish(Status(UNKNOWN, "UNKNOWN error"), this);
		}
	}
		break;
	case CallStatus::FINISH:
	{
		delete this; // Ӧ�����
	}
	break;
	default:
		assert(false);
		break;
	}
}

CAsyncRPCResponder_AddUsers::CAsyncRPCResponder_AddUsers(UserService::AsyncService* service, CompletionQueue* cqNewCall, ServerCompletionQueue* cqNotification, CAsyncRPCService* dataService)
	: CAsyncRPCResponder(service, cqNewCall, cqNotification, dataService), _requester(&_ctx)
{
	_svcUser->RequestAddUsers(&_ctx, &_requester, _cqNewCall, _scqNotification, this);
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
			if (_isFirstCalled)
			{
				_isFirstCalled = false;
				new CAsyncRPCResponder_AddUsers(_svcUser, _cqNewCall, _scqNotification, _dataService);

				// ��һ�εȴ�����
				_requester.Read(&_tmpUser, this); // ���ͻ��˵��������ݴ����Ա����_tmpUser�У�����ʹ����ʱ������
			}
			else
			{
				// ������յ���һ��stream����
				auto& users = _dataService->GetUsers();

				string accountName = _tmpUser.accountname();
				if (users.find(accountName) != users.end()) // ����û��Ƿ��Ѵ���
				{
					users[accountName].CopyFrom(_tmpUser);	// �����û�����

					ostringstream str;
					str << GetTimeStr() << "CAsyncRPCResponder_AddUsers::�����û��ɹ���" << accountName.c_str() << endl;
					OutputDebugStringA(str.str().c_str());
				}
				else
				{
					users.insert(map<string, User>::value_type(accountName, move(_tmpUser))); // �������û�

					ostringstream str;
					str << GetTimeStr() << "CAsyncRPCResponder_AddUsers::�������û��ɹ���" << accountName.c_str() << endl;
					OutputDebugStringA(str.str().c_str());
				}

				// �����ȴ�����
				_requester.Read(&_tmpUser, this); // ���ͻ��˵��������ݴ����Ա����_tmpUser�У�����ʹ����ʱ������
			}
		}
		else if (_isFirstCalled)
		{
			delete this; // Ӧ�����
		}
		else
		{
			_callStatus = CallStatus::FINISH;

			// �ظ�
			auto& users = _dataService->GetUsers();
			CommonNumber userCount;
			userCount.set_num(users.size());
			_requester.Finish(userCount, Status::OK, this);
		}
	}
		break;
	case CallStatus::FINISH:
	{
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
	_svcUser->RequestDeleteUsers(&_ctx, &_requester, _cqNewCall, _scqNotification, this);
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
			if (_isFirstCalled)
			{
				_isFirstCalled = false;
				new CAsyncRPCResponder_DeleteUsers(_svcUser, _cqNewCall, _scqNotification, _dataService);

				// ��һ�εȴ�����
				_requester.Read(&_rqAccountName, this); // ������ʱ��Ա����tmpAccountName�У�����ʹ����ʱ������
			}
			else
			{
				if (_isWriteDone)
				{
					ostringstream str;
					str << GetTimeStr() << "CAsyncRPCResponder_DeleteUsers:: read:" << _rqAccountName.accountname() << endl;
					OutputDebugStringA(str.str().c_str());

					// ������յ���һ��stream���ݡ�����ظ�������ͻ��˽��������������ݣ�
					auto& users = _dataService->GetUsers();
					if (users.find(_rqAccountName.accountname()) != users.end()) // ����û��Ƿ����
					{
						users.erase(_rqAccountName.accountname());

						// �����ظ�����
						CommonMsg rp;
						rp.set_issucess(true);
						ostringstream str;
						str << "DeleteUsers:" << _rqAccountName.accountname() << endl;
						rp.set_msg(str.str());

						// �ظ�һ��stream����
						_isWriteDone = false;
						_requester.Write(rp, this);						
					}
					else
					{
						// �����ظ�����
						CommonMsg rp;
						rp.set_issucess(false);
						ostringstream str;
						str << "DeleteUsers:not found " << _rqAccountName.accountname() << endl;
						rp.set_msg(str.str());

						// �ظ�һ��stream����
						_isWriteDone = false;
						_requester.Write(rp, this);
					}
				}
				else
				{
					// �����ȴ�����
					_isWriteDone = true;
					_requester.Read(&_rqAccountName, this); // ������ʱ��Ա����tmpAccountName�У�����ʹ����ʱ������
				}
			}
		}
		else if (_isFirstCalled)
		{
			delete this; // Ӧ�����
		}
		else
		{
			_callStatus = CallStatus::FINISH;
			_requester.Finish(Status::OK, this);
		}
	}
		break;
	case CallStatus::FINISH:
	{
		delete this; // Ӧ�����
	}
	break;
	default:
		assert(false);
		break;
	}
}

