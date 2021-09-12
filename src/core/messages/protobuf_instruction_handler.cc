#include <asio/basic_socket.hpp>
#include <core/messages/protobuf_instruction_handler.h>
namespace core::messages {
  using namespace google::protobuf::util;
ProtobufInstructionHandler::ProtobufInstructionHandler()
{
}
void ProtobufInstructionHandler::onInstructionIssued(OutgoingInstructionCallback outgoingCallback) {
  this->outgoingCallback = outgoingCallback;
}

void ProtobufInstructionHandler::sendIdentification(const Identification &identification)
{
  std::vector<uint8_t> request_payload;
  this->writeToBuffer(RequestType::IDENTITY, identification, request_payload);
  this->send(RequestType::IDENTITY, request_payload);
}
void ProtobufInstructionHandler::send(RequestType requestType, std::vector<uint8_t> &payload)
{
  this->outgoingCallback(requestType, payload);
}
void ProtobufInstructionHandler::disconnect()
{
  std::vector<uint8_t> request_payload;
  writeToBuffer(RequestType::DISCONNECT, request_payload);
  this->send(RequestType::DISCONNECT, request_payload);
}
void ProtobufInstructionHandler::writeToBuffer(RequestType requestType, std::vector<uint8_t> &buffer)
{
  const uint16_t type_network = htons(static_cast<uint16_t>(requestType));
  const size_t length = sizeof(type_network);
  buffer.resize(length);
  std::memcpy(buffer.data(), &type_network, sizeof(type_network));
}
void ProtobufInstructionHandler::writeToBuffer(RequestType requestType, const MessageLite &message, std::vector<uint8_t> &buffer)
{
  const uint16_t type_network = htons(static_cast<uint16_t>(requestType));
  const size_t size = message.ByteSizeLong();
  const uint32_t size_network = htonl((u_long)size);
  const size_t length = sizeof(type_network) + sizeof(size_network) + size;
  buffer.resize(length);
  std::memcpy(buffer.data(), &type_network, sizeof(type_network));
  memcpy(buffer.data() + sizeof(type_network), &size_network, sizeof(size_network));
  message.SerializeToArray(buffer.data() + sizeof(type_network) + sizeof(size_network), int(size));
}
ProtobufInstructionHandler::~ProtobufInstructionHandler()
{
}
}// namespace core::messages