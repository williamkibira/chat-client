#ifndef __CORE_ASIO_SOCKET_TCP_CLIENT__H
#define __CORE_ASIO_SOCKET_TCP_CLIENT__H

#include <asio.hpp>
#include <vector>
#include <deque>
#include <functional>
#include <cstdlib>

#include <core/networking/socket.h>

namespace core::networking::socket {

using asio::ip::tcp;
using asio::io_context;

class AsioSocketClient : public SocketClient
{
public:
  AsioSocketClient(io_context &context, tcp::resolver::results_type &tcp_endpoints);
  ~AsioSocketClient();
  void start() override;
  void stop() override;
  void sendPayload(RequestType requestType, const std::vector<uint8_t> &payload) override;
  void registerInstructionReceiver(InstructionReceivedCallback callback) override;
  void registerPayloadReceiver(PayloadReceivedCallback callback) override;


private:
  // CALLS
  void start_tcp_connect(tcp::resolver::results_type endpoint);
  void stop_tcp_connect();
  void on_tcp_write(const std::error_code error, size_t bytes_transferred);
  void do_tcp_read();
  void on_tcp_read(const std::error_code error, size_t bytes_transferred);
  void write_over_tcp(const std::vector<uint8_t> &data);
  void do_tcp_write();
  // DATA
  io_context &context;
  tcp::socket tcp_socket;
  tcp::resolver::results_type &tcp_endpoints;
  std::vector<uint8_t> read_tcp_buffer;
  std::deque<std::vector<uint8_t>> write_tcp_buffer;
  InstructionReceivedCallback instructionReceivedCallback;
  PayloadReceivedCallback payloadReceivedCallback;
};
};// namespace core::networking::socket

#endif// __CORE_ASIO_SOCKET_TCP_CLIENT__H