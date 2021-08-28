#ifndef __CORE_UV_SOCKET_TCP_CLIENT__H
#define __CORE_UV_SOCKET_TCP_CLIENT__H

#include <vector>
#include <deque>
#include <functional>
#include <cstdlib>
#include <uv.h>
#include <core/networking/socket.h>

namespace core::networking::socket {
class UVSocketClient : public SocketClient
{
public:
  UVSocketClient(uv_loop_t *loop, const std::string &ip_address, int port);
  ~UVSocketClient();
  void start() override;
  void stop() override;
  void sendPayload(RequestType requestType, const std::vector<uint8_t> &payload) override;
  void registerInstructionReceiver(InstructionReceivedCallback callback) override;
  void registerPayloadReceiver(PayloadReceivedCallback callback) override;

private:
  static void allocate_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
  static void on_connect(uv_connect_t * req, int status);
  static void on_tcp_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buffer);
  void write_over_tcp(const std::vector<uint8_t> &data);
  void do_tcp_write();
  
  uv_loop_t *loop;
  const std::string &ip_address;
  int port;
  uv_tcp_t socket;
  uv_connect_t connect;
  uv_write_t write_request;
  struct sockaddr_in dest;
  std::vector<uint8_t> read_tcp_buffer;
  std::deque<std::vector<uint8_t>> write_tcp_buffer;
  InstructionReceivedCallback instructionReceivedCallback;
  PayloadReceivedCallback payloadReceivedCallback;
};
};// namespace core::networking::socket

#endif//__CORE_UV_SOCKET_TCP_CLIENT__H