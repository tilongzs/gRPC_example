// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: UserRPC.proto

#include "UserRPC.pb.h"

#include <algorithm>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>

PROTOBUF_PRAGMA_INIT_SEG
namespace TestGRPC {
}  // namespace TestGRPC
static constexpr ::PROTOBUF_NAMESPACE_ID::EnumDescriptor const** file_level_enum_descriptors_UserRPC_2eproto = nullptr;
static constexpr ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor const** file_level_service_descriptors_UserRPC_2eproto = nullptr;
const uint32_t TableStruct_UserRPC_2eproto::offsets[1] = {};
static constexpr ::PROTOBUF_NAMESPACE_ID::internal::MigrationSchema* schemas = nullptr;
static constexpr ::PROTOBUF_NAMESPACE_ID::Message* const* file_default_instances = nullptr;

const char descriptor_table_protodef_UserRPC_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =
  "\n\rUserRPC.proto\022\010TestGRPC\032\nUser.proto2\374\001"
  "\n\013UserService\0226\n\007GetUser\022\031.TestGRPC.User"
  "AccountName\032\016.TestGRPC.User\"\000\0228\n\016GetUser"
  "sByRole\022\022.TestGRPC.UserRole\032\016.TestGRPC.U"
  "ser\"\0000\001\0226\n\010AddUsers\022\016.TestGRPC.User\032\026.Te"
  "stGRPC.CommonNumber\"\000(\001\022C\n\013DeleteUsers\022\031"
  ".TestGRPC.UserAccountName\032\023.TestGRPC.Com"
  "monMsg\"\000(\0010\001b\006proto3"
  ;
static const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable*const descriptor_table_UserRPC_2eproto_deps[1] = {
  &::descriptor_table_User_2eproto,
};
static ::PROTOBUF_NAMESPACE_ID::internal::once_flag descriptor_table_UserRPC_2eproto_once;
const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_UserRPC_2eproto = {
  false, false, 300, descriptor_table_protodef_UserRPC_2eproto, "UserRPC.proto", 
  &descriptor_table_UserRPC_2eproto_once, descriptor_table_UserRPC_2eproto_deps, 1, 0,
  schemas, file_default_instances, TableStruct_UserRPC_2eproto::offsets,
  nullptr, file_level_enum_descriptors_UserRPC_2eproto, file_level_service_descriptors_UserRPC_2eproto,
};
PROTOBUF_ATTRIBUTE_WEAK const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable* descriptor_table_UserRPC_2eproto_getter() {
  return &descriptor_table_UserRPC_2eproto;
}

// Force running AddDescriptors() at dynamic initialization time.
PROTOBUF_ATTRIBUTE_INIT_PRIORITY static ::PROTOBUF_NAMESPACE_ID::internal::AddDescriptorsRunner dynamic_init_dummy_UserRPC_2eproto(&descriptor_table_UserRPC_2eproto);
namespace TestGRPC {

// @@protoc_insertion_point(namespace_scope)
}  // namespace TestGRPC
PROTOBUF_NAMESPACE_OPEN
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>
