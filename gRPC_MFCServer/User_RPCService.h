#pragma once
#include <map>
using namespace std;

#include "../proto/User/UserRPC.grpc.pb.h"

class CUser_RPCService : public ::TestGRPC::UserService::Service
{
protected:
	// 根据用户名获取用户信息
	virtual ::grpc::Status GetUser(::grpc::ServerContext* context, const ::TestGRPC::UserAccountName* request, ::TestGRPC::User* response);
	// 获取所有用户信息
	virtual ::grpc::Status GetUsersByRole(::grpc::ServerContext* context, const ::TestGRPC::UserRole* request, ::grpc::ServerWriter< ::TestGRPC::User>* writer);
	// 批量增加新用户，返回服务器生成的新用户身份信息
	virtual ::grpc::Status AddUsers(::grpc::ServerContext* context, ::grpc::ServerReader< ::TestGRPC::User>* reader, ::TestGRPC::CommonNumber* response);
	// 批量删除用户，返回删除成功的用户名
	virtual ::grpc::Status DeleteUsers(::grpc::ServerContext* context, ::grpc::ServerReaderWriter< ::TestGRPC::CommonMsg, ::TestGRPC::UserAccountName>* stream);

private:
	map<string/*username*/, ::TestGRPC::User> _users;	// 所有用户信息
};

