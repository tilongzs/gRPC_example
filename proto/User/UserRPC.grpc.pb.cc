// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: proto/User/UserRPC.proto

#include "UserRPC.pb.h"
#include "UserRPC.grpc.pb.h"

#include <functional>
#include <grpcpp/impl/codegen/async_stream.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/channel_interface.h>
#include <grpcpp/impl/codegen/client_unary_call.h>
#include <grpcpp/impl/codegen/client_callback.h>
#include <grpcpp/impl/codegen/message_allocator.h>
#include <grpcpp/impl/codegen/method_handler.h>
#include <grpcpp/impl/codegen/rpc_service_method.h>
#include <grpcpp/impl/codegen/server_callback.h>
#include <grpcpp/impl/codegen/server_callback_handlers.h>
#include <grpcpp/impl/codegen/server_context.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/sync_stream.h>
namespace TestGRPC {

static const char* UserService_method_names[] = {
  "/TestGRPC.UserService/GetUser",
  "/TestGRPC.UserService/GetUsersByRole",
  "/TestGRPC.UserService/AddUsers",
  "/TestGRPC.UserService/DeleteUsers",
};

std::unique_ptr< UserService::Stub> UserService::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< UserService::Stub> stub(new UserService::Stub(channel));
  return stub;
}

UserService::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel)
  : channel_(channel), rpcmethod_GetUser_(UserService_method_names[0], ::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_GetUsersByRole_(UserService_method_names[1], ::grpc::internal::RpcMethod::SERVER_STREAMING, channel)
  , rpcmethod_AddUsers_(UserService_method_names[2], ::grpc::internal::RpcMethod::CLIENT_STREAMING, channel)
  , rpcmethod_DeleteUsers_(UserService_method_names[3], ::grpc::internal::RpcMethod::BIDI_STREAMING, channel)
  {}

::grpc::Status UserService::Stub::GetUser(::grpc::ClientContext* context, const ::TestGRPC::UserAccountName& request, ::TestGRPC::User* response) {
  return ::grpc::internal::BlockingUnaryCall< ::TestGRPC::UserAccountName, ::TestGRPC::User, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetUser_, context, request, response);
}

void UserService::Stub::experimental_async::GetUser(::grpc::ClientContext* context, const ::TestGRPC::UserAccountName* request, ::TestGRPC::User* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::TestGRPC::UserAccountName, ::TestGRPC::User, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetUser_, context, request, response, std::move(f));
}

void UserService::Stub::experimental_async::GetUser(::grpc::ClientContext* context, const ::TestGRPC::UserAccountName* request, ::TestGRPC::User* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetUser_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::TestGRPC::User>* UserService::Stub::PrepareAsyncGetUserRaw(::grpc::ClientContext* context, const ::TestGRPC::UserAccountName& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::TestGRPC::User, ::TestGRPC::UserAccountName, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetUser_, context, request);
}

::grpc::ClientAsyncResponseReader< ::TestGRPC::User>* UserService::Stub::AsyncGetUserRaw(::grpc::ClientContext* context, const ::TestGRPC::UserAccountName& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetUserRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::ClientReader< ::TestGRPC::User>* UserService::Stub::GetUsersByRoleRaw(::grpc::ClientContext* context, const ::TestGRPC::UserRole& request) {
  return ::grpc::internal::ClientReaderFactory< ::TestGRPC::User>::Create(channel_.get(), rpcmethod_GetUsersByRole_, context, request);
}

void UserService::Stub::experimental_async::GetUsersByRole(::grpc::ClientContext* context, const ::TestGRPC::UserRole* request, ::grpc::experimental::ClientReadReactor< ::TestGRPC::User>* reactor) {
  ::grpc::internal::ClientCallbackReaderFactory< ::TestGRPC::User>::Create(stub_->channel_.get(), stub_->rpcmethod_GetUsersByRole_, context, request, reactor);
}

::grpc::ClientAsyncReader< ::TestGRPC::User>* UserService::Stub::AsyncGetUsersByRoleRaw(::grpc::ClientContext* context, const ::TestGRPC::UserRole& request, ::grpc::CompletionQueue* cq, void* tag) {
  return ::grpc::internal::ClientAsyncReaderFactory< ::TestGRPC::User>::Create(channel_.get(), cq, rpcmethod_GetUsersByRole_, context, request, true, tag);
}

::grpc::ClientAsyncReader< ::TestGRPC::User>* UserService::Stub::PrepareAsyncGetUsersByRoleRaw(::grpc::ClientContext* context, const ::TestGRPC::UserRole& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncReaderFactory< ::TestGRPC::User>::Create(channel_.get(), cq, rpcmethod_GetUsersByRole_, context, request, false, nullptr);
}

::grpc::ClientWriter< ::TestGRPC::User>* UserService::Stub::AddUsersRaw(::grpc::ClientContext* context, ::TestGRPC::CommonNumber* response) {
  return ::grpc::internal::ClientWriterFactory< ::TestGRPC::User>::Create(channel_.get(), rpcmethod_AddUsers_, context, response);
}

void UserService::Stub::experimental_async::AddUsers(::grpc::ClientContext* context, ::TestGRPC::CommonNumber* response, ::grpc::experimental::ClientWriteReactor< ::TestGRPC::User>* reactor) {
  ::grpc::internal::ClientCallbackWriterFactory< ::TestGRPC::User>::Create(stub_->channel_.get(), stub_->rpcmethod_AddUsers_, context, response, reactor);
}

::grpc::ClientAsyncWriter< ::TestGRPC::User>* UserService::Stub::AsyncAddUsersRaw(::grpc::ClientContext* context, ::TestGRPC::CommonNumber* response, ::grpc::CompletionQueue* cq, void* tag) {
  return ::grpc::internal::ClientAsyncWriterFactory< ::TestGRPC::User>::Create(channel_.get(), cq, rpcmethod_AddUsers_, context, response, true, tag);
}

::grpc::ClientAsyncWriter< ::TestGRPC::User>* UserService::Stub::PrepareAsyncAddUsersRaw(::grpc::ClientContext* context, ::TestGRPC::CommonNumber* response, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncWriterFactory< ::TestGRPC::User>::Create(channel_.get(), cq, rpcmethod_AddUsers_, context, response, false, nullptr);
}

::grpc::ClientReaderWriter< ::TestGRPC::UserAccountName, ::TestGRPC::CommonMsg>* UserService::Stub::DeleteUsersRaw(::grpc::ClientContext* context) {
  return ::grpc::internal::ClientReaderWriterFactory< ::TestGRPC::UserAccountName, ::TestGRPC::CommonMsg>::Create(channel_.get(), rpcmethod_DeleteUsers_, context);
}

void UserService::Stub::experimental_async::DeleteUsers(::grpc::ClientContext* context, ::grpc::experimental::ClientBidiReactor< ::TestGRPC::UserAccountName,::TestGRPC::CommonMsg>* reactor) {
  ::grpc::internal::ClientCallbackReaderWriterFactory< ::TestGRPC::UserAccountName,::TestGRPC::CommonMsg>::Create(stub_->channel_.get(), stub_->rpcmethod_DeleteUsers_, context, reactor);
}

::grpc::ClientAsyncReaderWriter< ::TestGRPC::UserAccountName, ::TestGRPC::CommonMsg>* UserService::Stub::AsyncDeleteUsersRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) {
  return ::grpc::internal::ClientAsyncReaderWriterFactory< ::TestGRPC::UserAccountName, ::TestGRPC::CommonMsg>::Create(channel_.get(), cq, rpcmethod_DeleteUsers_, context, true, tag);
}

::grpc::ClientAsyncReaderWriter< ::TestGRPC::UserAccountName, ::TestGRPC::CommonMsg>* UserService::Stub::PrepareAsyncDeleteUsersRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncReaderWriterFactory< ::TestGRPC::UserAccountName, ::TestGRPC::CommonMsg>::Create(channel_.get(), cq, rpcmethod_DeleteUsers_, context, false, nullptr);
}

UserService::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      UserService_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< UserService::Service, ::TestGRPC::UserAccountName, ::TestGRPC::User, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](UserService::Service* service,
             ::grpc::ServerContext* ctx,
             const ::TestGRPC::UserAccountName* req,
             ::TestGRPC::User* resp) {
               return service->GetUser(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      UserService_method_names[1],
      ::grpc::internal::RpcMethod::SERVER_STREAMING,
      new ::grpc::internal::ServerStreamingHandler< UserService::Service, ::TestGRPC::UserRole, ::TestGRPC::User>(
          [](UserService::Service* service,
             ::grpc::ServerContext* ctx,
             const ::TestGRPC::UserRole* req,
             ::grpc::ServerWriter<::TestGRPC::User>* writer) {
               return service->GetUsersByRole(ctx, req, writer);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      UserService_method_names[2],
      ::grpc::internal::RpcMethod::CLIENT_STREAMING,
      new ::grpc::internal::ClientStreamingHandler< UserService::Service, ::TestGRPC::User, ::TestGRPC::CommonNumber>(
          [](UserService::Service* service,
             ::grpc::ServerContext* ctx,
             ::grpc::ServerReader<::TestGRPC::User>* reader,
             ::TestGRPC::CommonNumber* resp) {
               return service->AddUsers(ctx, reader, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      UserService_method_names[3],
      ::grpc::internal::RpcMethod::BIDI_STREAMING,
      new ::grpc::internal::BidiStreamingHandler< UserService::Service, ::TestGRPC::UserAccountName, ::TestGRPC::CommonMsg>(
          [](UserService::Service* service,
             ::grpc::ServerContext* ctx,
             ::grpc::ServerReaderWriter<::TestGRPC::CommonMsg,
             ::TestGRPC::UserAccountName>* stream) {
               return service->DeleteUsers(ctx, stream);
             }, this)));
}

UserService::Service::~Service() {
}

::grpc::Status UserService::Service::GetUser(::grpc::ServerContext* context, const ::TestGRPC::UserAccountName* request, ::TestGRPC::User* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status UserService::Service::GetUsersByRole(::grpc::ServerContext* context, const ::TestGRPC::UserRole* request, ::grpc::ServerWriter< ::TestGRPC::User>* writer) {
  (void) context;
  (void) request;
  (void) writer;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status UserService::Service::AddUsers(::grpc::ServerContext* context, ::grpc::ServerReader< ::TestGRPC::User>* reader, ::TestGRPC::CommonNumber* response) {
  (void) context;
  (void) reader;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status UserService::Service::DeleteUsers(::grpc::ServerContext* context, ::grpc::ServerReaderWriter< ::TestGRPC::CommonMsg, ::TestGRPC::UserAccountName>* stream) {
  (void) context;
  (void) stream;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace TestGRPC
