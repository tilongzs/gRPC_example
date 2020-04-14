#include "pch.h"
#include "User_RPCClient.h"
#include <random>
#include <chrono>
#include <time.h>
#include <sstream>
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

shared_ptr<User> CUser_RPCClient::GetUser(const string& accountName)
{
	// 创建请求数据
	UserAccountName userAccountName;
	userAccountName.set_accountname(accountName);

	// 创建接收服务器返回的回复数据
	shared_ptr<User> reply = make_shared<User>();
	ClientContext context;// 一次性使用，不能作为成员变量重复使用，否则会导致异常！

	// 发送请求
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
	// 创建请求数据
	UserRole userRole;
	userRole.set_role(role);

	// 创建接收服务器返回的回复数据
	shared_ptr<vector<User>> users = make_shared<vector<User>>();
	unique_ptr<User> reply = make_unique<User>();
	ClientContext context;// 一次性使用，不能作为成员变量重复使用，否则会导致异常！

	// 发送请求
	unsigned seed = chrono::system_clock::now().time_since_epoch().count();
	default_random_engine generator(seed);
	uniform_int_distribution<int> delay_distribution(800, 1500);
	unique_ptr<ClientReader<User>> reader(_stub->GetUsersByRole(&context, userRole));
	while (reader->Read(reply.get())) // 连续读取服务器返回的回复数据
	{
		char log[60] = { 0 };
		sprintf_s(log, "%s CUser_RPCClient::GetUsersByRole() Read\n", GetTimeStr().c_str());
		OutputDebugStringA(log);

		User cloneUser;
		cloneUser.CopyFrom(*reply); // 将读取到的一次数据保存
		users->push_back(move(cloneUser));
	}

	Status status = reader->Finish(); // 所有数据读取完毕
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
	// 接收服务器返回的回复数据
	CommonCount userCount;
	ClientContext context;// 一次性使用，不能作为成员变量重复使用，否则会导致异常！

	// 创建请求数据
	unique_ptr<ClientWriter<User>> writer(_stub->AddUsers(&context, &userCount));

	// 发送请求
	bool hasError = false;
	unsigned seed = chrono::system_clock::now().time_since_epoch().count();
	default_random_engine generator(seed);
	uniform_int_distribution<int> delay_distribution(800, 1500);
	for each (auto& user in *users)
	{
		char log[60] = { 0 };
		sprintf_s(log, "%s CUser_RPCClient::AddUsers() Write\n", GetTimeStr().c_str());
		OutputDebugStringA(log);

		if (!writer->Write(user))// 连续发送
		{
			hasError = true;
			break;
		}

		this_thread::sleep_for(chrono::milliseconds(delay_distribution(generator))); // 模拟延时发送
	}
	writer->WritesDone();

	Status status = writer->Finish(); // 所有数据发送完毕
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
	// 创建读写器
	ClientContext context;
	shared_ptr<ClientReaderWriter<UserAccountName, UserAccountName>> stream(_stub->DeleteUsers(&context));

	// 发送请求
	bool hasError = false;
	unsigned seed = chrono::system_clock::now().time_since_epoch().count();
	default_random_engine generator(seed);
	uniform_int_distribution<int> delay_distribution(800, 1500);
	for each (auto & accountName in *accountNames)
	{
		ostringstream str;
		str << GetTimeStr() << "CUser_RPCClient::DeleteUsers() Write:" << accountName.accountname() << endl;
		OutputDebugStringA(str.str().c_str());

		if (!stream->Write(accountName))// 连续发送
		{
			hasError = true;
			break;
		}

		this_thread::sleep_for(chrono::milliseconds(delay_distribution(generator))); // 测试延时发送
	}
	stream->WritesDone(); // 所有数据发送完毕

	// 读取回复
	shared_ptr<vector<UserAccountName>> deletedUserAccountName = make_shared<vector<UserAccountName>>();
	UserAccountName reply;
	while (stream->Read(&reply))
	{
		ostringstream str;
		str << GetTimeStr() << "CUser_RPCClient::DeleteUsers() Read:" << reply.accountname() << endl;
		OutputDebugStringA(str.str().c_str());

		deletedUserAccountName->push_back(move(reply));
	}

	Status status = stream->Finish(); // 所有数据读取完毕
	if (status.ok() && !hasError)
	{
		return deletedUserAccountName;
	}
	else
	{
		return nullptr;
	}
}
