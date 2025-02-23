#include "pch.h"
#include "User_RPCClient.h"
#include <random>
#include <chrono>
#include <time.h>
#include <sstream>
using namespace std::chrono;

static string GetTimeStr()
{
	COleDateTime oleTime(time(nullptr));
	return CStringA(oleTime.Format(L"%H:%M:%S "));
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
	auto seed = chrono::system_clock::now().time_since_epoch().count();;
	default_random_engine generator(seed);
	uniform_int_distribution<int> delay_distribution(800, 1500);
	unique_ptr<ClientReader<User>> reader(_stub->GetUsersByRole(&context, userRole));
	while (reader->Read(reply.get())) // 连续读取服务器返回的回复数据
	{
		ostringstream str;
		str << GetTimeStr() << "CUser_RPCClient::GetUsersByRole() Read\n";
		OutputDebugStringA(str.str().c_str());

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
	CommonNumber userCount;
	ClientContext context;// 一次性使用，不能作为成员变量重复使用，否则会导致异常！

	// 创建请求数据
	unique_ptr<ClientWriter<User>> writer(_stub->AddUsers(&context, &userCount));

	// 发送请求
	bool hasError = false;
	auto seed = chrono::system_clock::now().time_since_epoch().count();
	default_random_engine generator(seed);
	uniform_int_distribution<int> delay_distribution(800, 1500);
	for each (auto& user in *users)
	{
		ostringstream str;
		str << GetTimeStr() << "CUser_RPCClient::AddUsers() Write\n";
		OutputDebugStringA(str.str().c_str());

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
		return make_shared<unsigned>(userCount.num());
	}
	else
	{
		return nullptr;
	}
}

void CUser_RPCClient::DeleteUsers(shared_ptr<vector<UserAccountName>> accountNames)
{
	// 创建读写器
	ClientContext context;
	auto stream = _stub->DeleteUsers(&context);

	// 发送请求
	bool hasError = false;
	auto seed = chrono::system_clock::now().time_since_epoch().count();
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
	CommonMsg reply;
	while (stream->Read(&reply))
	{
		ostringstream str;
		if (reply.issucess())
		{
			str << GetTimeStr() << "CUser_RPCClient::DeleteUsers() success"  << endl;
		}
		else
		{
			str << GetTimeStr() << "CUser_RPCClient::DeleteUsers() failed:" << reply.msg() << endl;
		}
	
		OutputDebugStringA(str.str().c_str());
	}
}
