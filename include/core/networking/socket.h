#ifndef __CORE_NETWORKING_CLIENT__H
#define __CORE_NETWORKING_CLIENT__H

#include <asio.hpp>
#include <vector>
#include <deque>
#include <functional>
#include <cstdlib>
#include <memory>
#include <core/definitions/message_types.h>
namespace core::networking::socket {
typedef std::deque<std::vector<uint8_t>> payload_queue;

using namespace core::definitions;
class SocketClient: public std::enable_shared_from_this<SocketClient>
{
public:
  typedef std::function<void(ResponseType, const uint8_t *buffer, int length)> InstructionReceivedCallback;
  typedef std::function<void(const std::vector<uint8_t> &)> PayloadReceivedCallback;
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual void sendPayload(RequestType requestType, const std::vector<uint8_t> & payload) = 0;
  virtual void registerInstructionReceiver(InstructionReceivedCallback callback) = 0;
  virtual void registerPayloadReceiver(PayloadReceivedCallback callback) = 0;
};
}// namespace core::networking::socket
#endif// __CORE_NETWORKING_CLIENT__H