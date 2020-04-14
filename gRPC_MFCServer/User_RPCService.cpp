#include "pch.h"
#include "User_RPCService.h"
#include <random>
#include <chrono>
#include <time.h>
#include <sstream>
using namespace std::chrono;
using namespace grpc;
using namespace TestGRPC;

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

Status CUser_RPCService::GetUser(ServerContext* context, const UserAccountName* request, User* response)
{
	if (_users.find(request->accountname()) != _users.end())
	{
		response->CopyFrom(_users[request->accountname()]);
		
		return Status::OK;
	}
	else
	{
		return Status(NOT_FOUND, "accountname not found");
	}
}

Status CUser_RPCService::GetUsersByRole(ServerContext* context, const UserRole* request, ServerWriter< User>* writer)
{
	if (!_users.empty())
	{
		unsigned seed = chrono::system_clock::now().time_since_epoch().count();
		default_random_engine generator(seed);
		uniform_int_distribution<int> delay_distribution(800, 1500);

		for each (auto user in _users)
		{
			char log[60] = { 0 };
			sprintf_s(log, "%s CUser_RPCService::GetAllUser() Write\n", GetTimeStr().c_str());
			OutputDebugStringA(log);

			if (user.second.userrole() == request->role())
			{
				writer->Write(user.second);
				this_thread::sleep_for(chrono::milliseconds(delay_distribution(generator))); // ģ����ʱ����
			}
		}

		return Status::OK;
	}
	else
	{
		return Status(NOT_FOUND, "no user");
	}
}

Status CUser_RPCService::AddUsers(ServerContext* context, ServerReader< User>* reader, CommonCount* response)
{
	User user;
	while (reader->Read(&user))
	{
		ostringstream str;
		str << GetTimeStr() << "CUser_RPCService::AddUsers() Read accountName:" << user.accountname()<<endl;
		OutputDebugStringA(str.str().c_str());

		if (_users.find(user.accountname()) != _users.end()) // ����û��Ƿ��Ѵ���
		{
			_users[user.accountname()].CopyFrom(user);	// �����û�����
		}
		else
		{
			_users.insert(map<string, User>::value_type(user.accountname(), move(user))); // �������û�
		}
	}

	// �ظ�
	response->set_count(_users.size());

	return Status::OK;
}

Status CUser_RPCService::DeleteUsers(ServerContext* context, ServerReaderWriter<UserAccountName, UserAccountName>* stream)
{
	unsigned seed = chrono::system_clock::now().time_since_epoch().count();
	default_random_engine generator(seed);
	uniform_int_distribution<int> delay_distribution(800, 1500);

	UserAccountName accountName;
	while (stream->Read(&accountName))
	{
		ostringstream str;
		str << GetTimeStr() << "CUser_RPCService::DeleteUsers() Read:"<<accountName.accountname() << endl;
		OutputDebugStringA(str.str().c_str());

		if (_users.find(accountName.accountname()) != _users.end()) // ����û��Ƿ����
		{
			ostringstream str;
			str << GetTimeStr() << "CUser_RPCService::DeleteUsers() Write:"<< accountName.accountname() << endl;
			OutputDebugStringA(str.str().c_str());
			
			// ������ɾ�����û�����
			_users.erase(accountName.accountname());

			// �ظ�
			stream->Write(accountName);

			this_thread::sleep_for(chrono::milliseconds(delay_distribution(generator))); // ģ����ʱ����
		}
	}

	return Status::OK;
}
