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

void CAsyncRPCClient::Run(CgRPCMFCClientDlg* mainDlg, string serverAddr)
{
	_mainDlg = mainDlg;

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

	// 创建gRPC channel
	_channel = grpc::CreateChannel(serverAddr, grpc::InsecureChannelCredentials()); // 监听不需要认证的连接
	// 增加SSL验证
// 	grpc::SslCredentialsOptions sslOpts;
// 	sslOpts.pem_root_certs = GetFileString("ssl/server.crt");
// 	sslOpts.pem_private_key = GetFileString("ssl/client.key");
// 	sslOpts.pem_cert_chain = GetFileString("ssl/client.crt");
// 	auto creds = grpc::SslCredentials(sslOpts);
// 	_channel = grpc::CreateChannel(serverAddr, creds);

	_stub = UserService::NewStub(_channel);

	_ctsCommon = make_unique<cancellation_token_source>();
	auto token = _ctsCommon->get_token();
	task<void> taskProceed([&, token]
	{
		ostringstream str;
		str << GetTimeStr() << "CommonAsyncClientCall::RunClient() taskBegin" << endl;
		OutputDebugStringA(str.str().c_str());

		void* tag = nullptr;
		bool ok = false;
		while (!token.is_canceled())
		{
			if (!_cq.Next(&tag, &ok)) // 阻塞，直至有新的事件
			{
				str.clear();
				str << GetTimeStr() << "CAsyncRPCClient::Run() Next() failed" << endl;
				OutputDebugStringA(str.str().c_str());
				break;
			}

			// 处理事件
			static_cast<CAsyncRPCRequester*>(tag)->Proceed(ok);
		}

		_cq.Shutdown();

		str.clear();
		str << GetTimeStr() << "CommonAsyncClientCall::RunClient() taskEnd" << endl;
		OutputDebugStringA(str.str().c_str());
	});
}

void CAsyncRPCClient::Shutdown()
{
	if (_ctsCommon)
	{
		_cq.Shutdown();
		_ctsCommon->cancel();
		_ctsCommon = nullptr;
	}
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
			// 接收完成
			_mainDlg->OnGetUser(make_shared<User>(move(_reply)));
		}
		else
		{
			// 接收完成，但服务器返回了执行失败消息
			_mainDlg->OnGetUser(nullptr);
		}
	}
	else if (_isFirstCalled)
	{
		delete this; // 应答结束
	}
	else
	{
		// 接收过程中发送错误
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
	case CallStatus::PROCESS:
	{
		if (isOK)
		{
			if (_status.ok())
			{
				// 第一次等待数据
				if (_isFirstCalled)
				{
					_isFirstCalled = false;
					_responsder->Read(&_reply, this);
				}
				else
				{
					// 处理接收到的一次stream数据
					_mainDlg->OnGetUsersByRole(make_shared<User>(move(_reply)));

					// 继续等待数据
					_responsder->Read(&_reply, this);
				}
			}
			else
			{
				// 接收完成，但服务器返回了执行失败消息
				ostringstream str;
				str << GetTimeStr() << "CAsyncRPC_GetUsersByRole::Proceed() failed:" << _status.error_message() << endl;
				OutputDebugStringA(str.str().c_str());

				_mainDlg->OnGetUsersByRoleComplete(false);

				_callStatus = CallStatus::FINISH;
				_responsder->Finish(&_status, this);
			}
		}
		else if (_isFirstCalled)
		{
			delete this; // 应答结束
		}
		else
		{
			// 接收完成
			_callStatus = CallStatus::FINISH;
			_responsder->Finish(&_status, this);

			_mainDlg->OnGetUsersByRoleComplete(true);
		}
	}
	break;
	case CallStatus::FINISH:
	{
		delete this; // 应答结束
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
	case CallStatus::PROCESS:
	{
		if (isOK)
		{
			if (!_isSendComplete)
			{
				auto iter = _request->begin();
				for (int i = 0; i != _tag; ++i)
				{
					++iter; // 根据标记调整数据指针
				}

				if (iter != _request->end())
				{
					_tag++; // 标记当前发送进度
					_responsder->Write(*iter, this);
				}
				else
				{
					// 发送完成
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
				// 结束发送
				_callStatus = CallStatus::FINISH;
				_responsder->Finish(&_status, this);
			}
		}
		else if (_isFirstCalled)
		{
			delete this; // 应答结束
		}
		else
		{
			_mainDlg->OnAddUsersComplete(false);

			// 结束发送
			_callStatus = CallStatus::FINISH;
			_responsder->Finish(&_status, this);
		}		
	}
		break;
	case CallStatus::FINISH:
	{
		// 处理收到的回复数据
		_mainDlg->OnAddUsersComplete(true, _reply.num());

		// 应答结束
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
	case CallStatus::PROCESS:
	{
		if (isOK)
		{
			// 第一次发送数据
			if (_isFirstCalled)
			{
				_isFirstCalled = false;

				// 发送一次stream数据
				auto iter = _request->begin();
				_tag++; // 标记当前发送进度
				_isWriteDone = false;
				_responsder->Write(*iter, this);
			}
			else
			{
				if (_isWriteDone)
				{
					// 处理接收到的一次stream数据
					_mainDlg->OnDeleteUsers(_reply.issucess(), make_shared<string>(_reply.msg()));

					// 发送请求数据
					auto iter = _request->begin();
					for (int i = 0; i != _tag; ++i)
					{
						++iter; // 根据标记调整数据指针
					}

					if (iter != _request->end())
					{
						_tag++; // 标记当前发送进度
						_isWriteDone = false;
						_responsder->Write(*iter, this);// 发送一次stream数据
					}
					else
					{
						// 全部请求发送完成
						_isSendComplete = true;
						_isWriteDone = false;
						_responsder->WritesDone(this); // 结束发送
					}
				}
				else
				{
					// 继续等待数据
					_isWriteDone = true;
					_responsder->Read(&_reply, this);
				}
			}			
		}
		else if (_isFirstCalled)
		{
			delete this; // 应答结束
		}
		else
		{
			_callStatus = CallStatus::FINISH;
			_responsder->Finish(&_status, this);
		}
	}
		break;
	case CallStatus::FINISH:
	{
		_mainDlg->OnDeleteUsersComplete();
		delete this;
	}
		break;
	default:
		break;
	}
}
