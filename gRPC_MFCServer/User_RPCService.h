#pragma once
#include <map>
using namespace std;

#include "../proto/User/UserRPC.grpc.pb.h"

class CUser_RPCService : public ::TestGRPC::UserService::Service
{
protected:
	// �����û�����ȡ�û���Ϣ
	virtual ::grpc::Status GetUser(::grpc::ServerContext* context, const ::TestGRPC::UserAccountName* request, ::TestGRPC::User* response);
	// ��ȡ�����û���Ϣ
	virtual ::grpc::Status GetUsersByRole(::grpc::ServerContext* context, const ::TestGRPC::UserRole* request, ::grpc::ServerWriter< ::TestGRPC::User>* writer);
	// �����������û������ط��������ɵ����û������Ϣ
	virtual ::grpc::Status AddUsers(::grpc::ServerContext* context, ::grpc::ServerReader< ::TestGRPC::User>* reader, ::TestGRPC::CommonNumber* response);
	// ����ɾ���û�������ɾ���ɹ����û���
	virtual ::grpc::Status DeleteUsers(::grpc::ServerContext* context, ::grpc::ServerReaderWriter< ::TestGRPC::CommonMsg, ::TestGRPC::UserAccountName>* stream);

private:
	map<string/*username*/, ::TestGRPC::User> _users;	// �����û���Ϣ
};

