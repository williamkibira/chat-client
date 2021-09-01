#ifndef __CORE__MESSAGES_INSTRUCTION_HANDLER__H
#define __CORE__MESSAGES_INSTRUCTION_HANDLER__H

#include <functional>
#include <vector>
#include <core/definitions/message_types.h>
#include <protos/identification.pb.h>
#include <protos/responses.pb.h>

namespace core::messages {
using namespace core::definitions;
class InstructionHandler
{
public:
  typedef std::function<void(RequestType requestType, std::vector<uint8_t> &payload)> OutgoingInstructionCallback;
  virtual void onInstructionIssued(OutgoingInstructionCallback outgoingCallback) = 0;
  virtual void onReceived(ResponseType responseType, const std::vector<uint8_t>& buffer) = 0;
  virtual void sendIdentification(const Identification &identification) = 0;
  virtual void send(RequestType requestType, std::vector<uint8_t> &payload) = 0;
  virtual void disconnect() = 0;
};
};// namespace core::messages

#endif// __CORE__MESSAGES_INSTRUCTION_HANDLER__H