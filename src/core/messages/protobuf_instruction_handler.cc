#include <asio/basic_socket.hpp>
#include <core/messages/protobuf_instruction_handler.h>
#include <spdlog/spdlog.h>
#include <google/protobuf/util/time_util.h>
namespace core::messages {
  using namespace google::protobuf::util;
ProtobufInstructionHandler::ProtobufInstructionHandler()
{
}
void ProtobufInstructionHandler::onInstructionIssued(OutgoingInstructionCallback outgoingCallback) {
  this->outgoingCallback = outgoingCallback;
}
void ProtobufInstructionHandler::onReceived(ResponseType responseType, int length, const uint8_t *buffer)
{
  // FOR THE TIME BEING WE CAN ASSUME THE TYPE OF INSTRUCTIONS TO BE HANDLED
  spdlog::get("chat-client")->info("RECEIVED INSTRUCTION");
  if (responseType == ResponseType::REQUEST_IDENTITY) {
    spdlog::get("chat-client")->info("SERVER IS REQUESTING CLIENT IDENTITY");
    Identification identification;
    Device device;
    device.set_name("WINDOWS DEV");
    device.set_operating_system("WINDOWS 10 HOME EDITION");
    device.set_version("20H10");
    device.set_ip_address("127.0.0.1");
    identification.set_token("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    identification.set_allocated_device(&device);
    spdlog::get("chat-client")->info("SENDING IDENTIFICATION TO SERVER");
    sendIdentification(identification);
  } else if (responseType == ResponseType::IDENTITY_REJECTION) {
    spdlog::get("chat-client")->info("YOUR IDENTITY WAS NOT ACCEPTED");
    Failure failure;
    failure.ParseFromArray(buffer, length);
    spdlog::get("chat-client")->error("ERROR: {}", failure.error());
    spdlog::get("chat-client")->error("DETAILS: {}", failure.details());
    spdlog::get("chat-client")->error("OCCURRED AT: {}", TimeUtil::ToString(failure.occurred_at()));
    this->disconnect();
  } else if (responseType == ResponseType::IDENTITY_ACCEPTED) {
    Info success;
    spdlog::get("chat-client")->info("MESSAGE: {}", success.message());
    spdlog::get("chat-client")->info("DETAILS: {}", success.details());
    spdlog::get("chat-client")->info("OCCURRED AT: {}", TimeUtil::ToString(success.occurred_at()));
  } else if (responseType == ResponseType::DISCONNECTION_ACCEPTED) {
    Info success;
    spdlog::get("chat-client")->info("MESSAGE: {}", success.message());
    spdlog::get("chat-client")->info("DETAILS: {}", success.details());
    spdlog::get("chat-client")->info("OCCURRED AT: {}", TimeUtil::ToString(success.occurred_at()));
  }
}
void ProtobufInstructionHandler::sendIdentification(const Identification &identification)
{
  std::vector<uint8_t> request_payload;
  this->writeToBuffer(RequestType::IDENTITY, identification, request_payload);
  this->send(RequestType::IDENTITY, request_payload);
}
void ProtobufInstructionHandler::send(RequestType requestType, const std::vector<uint8_t> &payload)
{
  this->outgoingCallback(requestType, payload);
}
void ProtobufInstructionHandler::disconnect()
{
  spdlog::get("chat-client")->info("PREPARING TO DISCONNECT");
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
  const uint32_t size_network = htonl(size);
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