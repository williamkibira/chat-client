#ifndef __CORE_NETWORKING_CLIENT__H
#define __CORE_NETWORKING_CLIENT__H

#include <vector>
#include <deque>
#include <functional>
#include <cstdlib>
#include <core/definitions/message_types.h>
namespace core::networking::socket {
typedef std::deque<std::vector<uint8_t>> payload_queue;

using namespace core::definitions;
class SocketClient
{
public:
  typedef std::function<void(ResponseType, const std::vector<uint8_t>)> InstructionReceivedCallback;
  typedef std::function<void(const std::vector<uint8_t> &)> PayloadReceivedCallback;
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual void sendPayload(RequestType requestType, std::vector<uint8_t> & payload) = 0;
  virtual void registerInstructionReceiver(InstructionReceivedCallback callback) = 0;
  virtual void registerPayloadReceiver(PayloadReceivedCallback callback) = 0;
};
}// namespace core::networking::socket
#endif// __CORE_NETWORKING_CLIENT__H