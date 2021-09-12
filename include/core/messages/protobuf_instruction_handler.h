#ifndef __CORE__MESSAGES_PROTOBUF_INSTRUCTION_HANDLER__H
#define __CORE__MESSAGES_PROTOBUF_INSTRUCTION_HANDLER__H

#include <functional>
#include <vector>
#include <core/messages/instructions_handler.h>
#include <google/protobuf/message.h>
namespace core::messages {
using google::protobuf::MessageLite;
class ProtobufInstructionHandler : public InstructionHandler
{
public:
  ProtobufInstructionHandler();
  ~ProtobufInstructionHandler();
  void onInstructionIssued(OutgoingInstructionCallback outgoingCallback) override;
  void sendIdentification(const Identification &identification) override;
  void send(RequestType requestType, std::vector<uint8_t> &payload) override;
  void disconnect() override;

private:
  void writeToBuffer(RequestType requestType, const MessageLite &message, std::vector<uint8_t> &buffer);
  void writeToBuffer(RequestType requestType, std::vector<uint8_t> &buffer);
  OutgoingInstructionCallback outgoingCallback;
};
};// namespace core::messages

#endif// __CORE__MESSAGES_PROTOBUF_INSTRUCTION_HANDLER__H