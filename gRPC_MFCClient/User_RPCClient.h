#pragma once
#include <string>

#include <grpcpp/grpcpp.h>
#include "../proto/User/User.grpc.pb.h"

using namespace grpc;
using namespace TestGRPC;
using namespace std;

class CUser_RPCClient
{
public:
	CUser_RPCClient(shared_ptr<Channel>& channel) 
		: _stub(UserService::NewStub(channel)) {}
private:
	std::unique_ptr<UserService::Stub> _stub;

public:
	shared_ptr<User> GetUser(const string& accountName);
	shared_ptr<vector<User>> GetUsersByRole(const Role& role);
	shared_ptr<unsigned> AddUsers(const shared_ptr<vector<User>>& users);
	void DeleteUsers(shared_ptr<vector<UserAccountName>> accountNames);
};

