#include "pch.h"
#include "User_RPCAsyncClient.h"
#include "gRPC_MFCClientDlg.h"
#include <pplx/pplxtasks.h>
#include <chrono>
#include <sstream>
#include <fstream>
using namespace Concurrency;
using namespace std::chrono;

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

void CAsyncRPCClient::Run(CgRPCMFCClientDlg* mainDlg, string serverAddr)
{
	_mainDlg = mainDlg;

	auto GetFileString = [&](string extraFilePath)
	{
		char buf[MAX_PATH] = { 0 };
		GetModuleFileNameA(NULL, buf, MAX_PATH * sizeof(WCHAR));
		GetLongPathNameA(buf, buf, MAX_PATH * sizeof(WCHAR));
		PathRemoveFileSpecA(buf);
		PathAppendA(buf, extraFilePath.c_str());

		ifstream in(buf);
		istreambuf_iterator<char> beg(in), end;
		string str(beg, end);
		return str;
	};

	// ����gRPC channel
	_channel = grpc::CreateChannel(serverAddr, grpc::InsecureChannelCredentials()); // ��������Ҫ��֤������
	// ����SSL��֤
// 	grpc::SslCredentialsOptions sslOpts;
// 	sslOpts.pem_root_certs = GetFileString("ssl/server.crt");
// 	sslOpts.pem_private_key = GetFileString("ssl/client.key");
// 	sslOpts.pem_cert_chain = GetFileString("ssl/client.crt");
// 	auto creds = grpc::SslCredentials(sslOpts);
// 	_channel = grpc::CreateChannel(serverAddr, creds);

	_stub = UserService::NewStub(_channel);

	task<void> taskProceed([&]
	{
		ostringstream str;
		str << GetTimeStr() << "CommonAsyncClientCall::RunClient() taskBegin" << endl;
		OutputDebugStringA(str.str().c_str());

		void* tag = nullptr;
		bool ok = false;
		while (true)
		{
			if (!_cq.Next(&tag, &ok)) // ������ֱ�����µ��¼�
			{
				str.clear();
				str << GetTimeStr() << "CAsyncRPCClient::Run() Next() failed" << endl;
				OutputDebugStringA(str.str().c_str());
				break;
			}

			// �����¼�
			static_cast<CAsyncRPCRequester*>(tag)->Proceed(ok);
		}

		str.clear();
		str << GetTimeStr() << "CommonAsyncClientCall::RunClient() taskEnd" << endl;
		OutputDebugStringA(str.str().c_str());
	});
}

void CAsyncRPCClient::GetUser(const string& accountName)
{
	UserAccountName userAccountName;
	userAccountName.set_accountname(accountName);
	new CAsyncRPC_GetUser(_mainDlg, _stub.get(), _cq, userAccountName);
}

void CAsyncRPCClient::GetUsersByRole(const Role& role)
{
	UserRole userRole;
	userRole.set_role(role);
	new CAsyncRPC_GetUsersByRole(_mainDlg, _stub.get(), _cq, userRole);
}

void CAsyncRPCClient::AddUsers(shared_ptr<vector<User>> users)
{
	new CAsyncRPC_AddUsers(_mainDlg, _stub.get(), _cq, users);
}

void CAsyncRPCClient::DeleteUsers(shared_ptr<vector<UserAccountName>> accountNames)
{
	new CAsyncRPC_DeleteUsers(_mainDlg, _stub.get(), _cq, accountNames);
}
/*CAsyncRPCClient***************************************************************************************************/

// CAsyncRPC_GetUser
CAsyncRPC_GetUser::CAsyncRPC_GetUser(CgRPCMFCClientDlg* mainDlg, UserService::Stub* stub, CompletionQueue& cq, UserAccountName& request)
{
	_mainDlg = mainDlg;
	_responsder = stub->AsyncGetUser(&_ctx, request, &cq);
	_responsder->Finish(&_reply, &_status, this);
}

void CAsyncRPC_GetUser::Proceed(bool isOK /*= true*/)
{
	if (isOK)
	{
		if (_status.ok())
		{
			// �������
			_mainDlg->OnGetUser(make_shared<User>(move(_reply)));
		}
		else
		{
			// ������ɣ���������������ִ��ʧ����Ϣ
			_mainDlg->OnGetUser(nullptr);
		}
	}
	else
	{
		// ���չ����з��ʹ���
		_mainDlg->OnGetUser(nullptr);
	}

	delete this;
}

// CAsyncRPC_GetUsersByRole
CAsyncRPC_GetUsersByRole::CAsyncRPC_GetUsersByRole(CgRPCMFCClientDlg* mainDlg, UserService::Stub* stub, CompletionQueue& cq, UserRole& request)
{
	_mainDlg = mainDlg;
	_responsder = stub->AsyncGetUsersByRole(&_ctx, request, &cq, this);
}

void CAsyncRPC_GetUsersByRole::Proceed(bool isOK)
{
	switch (_callStatus)
	{
	case CAsyncRPCRequester::CallStatus::PROCESS:
	{
		if (isOK)
		{
			if (_status.ok())
			{
				// ��һ�εȴ�����
				if (_isFirstRead)
				{
					_isFirstRead = false;
					_responsder->Read(&_reply, this);
					return;
				}

				// ������յ���һ��stream����
				_mainDlg->OnGetUsersByRole(make_shared<User>(move(_reply)));

				// �����ȴ�����
				_responsder->Read(&_reply, this);
			}
			else
			{
				// ������ɣ���������������ִ��ʧ����Ϣ
				ostringstream str;
				str << GetTimeStr() << "CAsyncRPC_GetUsersByRole::Proceed() failed:" << _status.error_message() << endl;
				OutputDebugStringA(str.str().c_str());

				_mainDlg->OnGetUsersByRoleComplete(false);

				_callStatus = CAsyncRPCRequester::CallStatus::FINISH;
				_responsder->Finish(&_status, this);
			}
		}
		else
		{
			// �������
			_callStatus = CAsyncRPCRequester::CallStatus::FINISH;
			_responsder->Finish(&_status, this);

			_mainDlg->OnGetUsersByRoleComplete(true);
		}
	}
	break;
	case CAsyncRPCRequester::CallStatus::FINISH:
	{
		delete this;
	}
	break;
	default:
		assert(false);
		break;
	}
}

// CAsyncRPC_AddUsers
CAsyncRPC_AddUsers::CAsyncRPC_AddUsers(CgRPCMFCClientDlg* mainDlg, UserService::Stub* stub, CompletionQueue& cq, shared_ptr<vector<User>>& request)
{
	_mainDlg = mainDlg;
	_request = request;
	_responsder = stub->AsyncAddUsers(&_ctx, &_reply, &cq, this);
}

void CAsyncRPC_AddUsers::Proceed(bool isOK /*= true*/)
{
	switch (_callStatus)
	{
	case CAsyncRPCRequester::CallStatus::PROCESS:
	{
		if (isOK)
		{
			if (!_isSendComplete)
			{
				auto iter = _request->begin();
				for (int i = 0; i != _tag; ++i)
				{
					++iter; // ���ݱ�ǵ�������ָ��
				}

				if (iter != _request->end())
				{
					_tag++; // ��ǵ�ǰ���ͽ���
					_responsder->Write(*iter, this);
				}
				else
				{
					_isSendComplete = true;
					_responsder->WritesDone(this);
				}

				if (iter != _request->begin())
				{
					auto lastIter = iter - 1;
					_mainDlg->OnAddUsers(make_shared<string>(lastIter->accountname()));
				}
			}
			else
			{
				_mainDlg->OnAddUsersComplete(true);

				// ��������
				_callStatus = CallStatus::FINISH;
				_responsder->Finish(&_status, this);
			}
		}
		else
		{
			_mainDlg->OnAddUsersComplete(false);

			// ��������
			_callStatus = CallStatus::FINISH;
			_responsder->Finish(&_status, this);
		}		
	}
		break;
	case CAsyncRPCRequester::CallStatus::FINISH:
	{
		delete this;
	}
		break;
	default:
		assert(false);
		break;
	}
}

// CAsyncRPC_DeleteUsers
CAsyncRPC_DeleteUsers::CAsyncRPC_DeleteUsers(CgRPCMFCClientDlg* mainDlg, UserService::Stub* stub, CompletionQueue& cq, shared_ptr<vector<UserAccountName>>& request)
{
	_mainDlg = mainDlg;
	_request = request;
	_responsder = stub->AsyncDeleteUsers(&_ctx, &cq, this);
}

void CAsyncRPC_DeleteUsers::Proceed(bool isOK)
{
	switch (_callStatus)
	{
	case CAsyncRPCRequester::CallStatus::PROCESS:
	{
		if (!_isSendComplete)
		{
			if (isOK)
			{
				auto iter = _request->begin();
				for (int i = 0; i != _tag; ++i)
				{
					++iter; // ���ݱ�ǵ�������ָ��
				}

				if (iter != _request->end())
				{
					ostringstream str;
					str << GetTimeStr() << "CAsyncRPC_DeleteUsers::Proceed() Write:" << iter->accountname() << endl;
					OutputDebugStringA(str.str().c_str());

					_tag++; // ��ǵ�ǰ���ͽ���
					_responsder->Write(*iter, this);
				}
				else
				{
					_isSendComplete = true;
					_responsder->WritesDone(this);
				}
			}
			else
			{
				_mainDlg->OnDeleteUsersComplete(false);

				// ��������
				_callStatus = CallStatus::FINISH;
				_responsder->Finish(&_status, this);
			}
		}
		else
		{
			if (isOK)
			{
				// ��һ�εȴ�����
				if (_isFirstRead)
				{
					_isFirstRead = false;
					_responsder->Read(&_reply, this);
					return;
				}
				
				// ������յ���һ��stream����
				_mainDlg->OnDeleteUsers(make_shared<string>(_reply.accountname()));
				_deletedUserAccountName.push_back(move(_reply));

				// �����ȴ�����
				_responsder->Read(&_reply, this);
			}
			else
			{
				_mainDlg->OnDeleteUsersComplete(true);

				// ��������
				_callStatus = CallStatus::FINISH;
				_responsder->Finish(&_status, this);
			}
		}
	}
		break;
	case CAsyncRPCRequester::CallStatus::FINISH:
	{
		delete this;
	}
		break;
	default:
		break;
	}
}
