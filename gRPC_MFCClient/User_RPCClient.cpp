#include "pch.h"
#include "User_RPCClient.h"
#include <random>
#include <chrono>
#include <time.h>
#include <sstream>
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

shared_ptr<User> CUser_RPCClient::GetUser(const string& accountName)
{
	// ������������
	UserAccountName userAccountName;
	userAccountName.set_accountname(accountName);

	// �������շ��������صĻظ�����
	shared_ptr<User> reply = make_shared<User>();
	ClientContext context;// һ����ʹ�ã�������Ϊ��Ա�����ظ�ʹ�ã�����ᵼ���쳣��

	// ��������
	Status status = _stub->GetUser(&context, userAccountName, reply.get());
	if (status.ok())
	{
		return reply;
	}
	else
	{
		return nullptr;
	}
}

shared_ptr<vector<User>> CUser_RPCClient::GetUsersByRole(const Role& role)
{
	// ������������
	UserRole userRole;
	userRole.set_role(role);

	// �������շ��������صĻظ�����
	shared_ptr<vector<User>> users = make_shared<vector<User>>();
	unique_ptr<User> reply = make_unique<User>();
	ClientContext context;// һ����ʹ�ã�������Ϊ��Ա�����ظ�ʹ�ã�����ᵼ���쳣��

	// ��������
	unsigned seed = chrono::system_clock::now().time_since_epoch().count();
	default_random_engine generator(seed);
	uniform_int_distribution<int> delay_distribution(800, 1500);
	unique_ptr<ClientReader<User>> reader(_stub->GetUsersByRole(&context, userRole));
	while (reader->Read(reply.get())) // ������ȡ���������صĻظ�����
	{
		char log[60] = { 0 };
		sprintf_s(log, "%s CUser_RPCClient::GetUsersByRole() Read\n", GetTimeStr().c_str());
		OutputDebugStringA(log);

		User cloneUser;
		cloneUser.CopyFrom(*reply); // ����ȡ����һ�����ݱ���
		users->push_back(move(cloneUser));
	}

	Status status = reader->Finish(); // �������ݶ�ȡ���
	if (status.ok())
	{
		return users;
	}
	else
	{
		return nullptr;
	}
}

shared_ptr<unsigned> CUser_RPCClient::AddUsers(const shared_ptr<vector<User>>& users)
{
	// ���շ��������صĻظ�����
	CommonCount userCount;
	ClientContext context;// һ����ʹ�ã�������Ϊ��Ա�����ظ�ʹ�ã�����ᵼ���쳣��

	// ������������
	unique_ptr<ClientWriter<User>> writer(_stub->AddUsers(&context, &userCount));

	// ��������
	bool hasError = false;
	unsigned seed = chrono::system_clock::now().time_since_epoch().count();
	default_random_engine generator(seed);
	uniform_int_distribution<int> delay_distribution(800, 1500);
	for each (auto& user in *users)
	{
		char log[60] = { 0 };
		sprintf_s(log, "%s CUser_RPCClient::AddUsers() Write\n", GetTimeStr().c_str());
		OutputDebugStringA(log);

		if (!writer->Write(user))// ��������
		{
			hasError = true;
			break;
		}

		this_thread::sleep_for(chrono::milliseconds(delay_distribution(generator))); // ģ����ʱ����
	}
	writer->WritesDone();

	Status status = writer->Finish(); // �������ݷ������
	if (status.ok() && !hasError)
	{
		return make_shared<unsigned>(userCount.count());
	}
	else
	{
		return nullptr;
	}
}

shared_ptr<vector<UserAccountName>> CUser_RPCClient::DeleteUsers(shared_ptr<vector<UserAccountName>> accountNames)
{
	// ������д��
	ClientContext context;
	shared_ptr<ClientReaderWriter<UserAccountName, UserAccountName>> stream(_stub->DeleteUsers(&context));

	// ��������
	bool hasError = false;
	unsigned seed = chrono::system_clock::now().time_since_epoch().count();
	default_random_engine generator(seed);
	uniform_int_distribution<int> delay_distribution(800, 1500);
	for each (auto & accountName in *accountNames)
	{
		ostringstream str;
		str << GetTimeStr() << "CUser_RPCClient::DeleteUsers() Write:" << accountName.accountname() << endl;
		OutputDebugStringA(str.str().c_str());

		if (!stream->Write(accountName))// ��������
		{
			hasError = true;
			break;
		}

		this_thread::sleep_for(chrono::milliseconds(delay_distribution(generator))); // ������ʱ����
	}
	stream->WritesDone(); // �������ݷ������

	// ��ȡ�ظ�
	shared_ptr<vector<UserAccountName>> deletedUserAccountName = make_shared<vector<UserAccountName>>();
	UserAccountName reply;
	while (stream->Read(&reply))
	{
		ostringstream str;
		str << GetTimeStr() << "CUser_RPCClient::DeleteUsers() Read:" << reply.accountname() << endl;
		OutputDebugStringA(str.str().c_str());

		deletedUserAccountName->push_back(move(reply));
	}

	Status status = stream->Finish(); // �������ݶ�ȡ���
	if (status.ok() && !hasError)
	{
		return deletedUserAccountName;
	}
	else
	{
		return nullptr;
	}
}
