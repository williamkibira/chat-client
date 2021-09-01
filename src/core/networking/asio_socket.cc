#include <core/networking/asio_socket.h>
#include <spdlog/spdlog.h>
#include <core/definitions/message_types.h>
#include <core/definitions/header_length.h>
#include <iostream>
namespace core::networking::socket {
AsioSocketClient::AsioSocketClient(io_context &context, tcp::resolver::results_type &tcp_endpoints) : context(context),
                                                                                                      tcp_socket(context),
                                                                                                      tcp_endpoints(tcp_endpoints),
                                                                                                      read_tcp_buffer(MAX_TCP_LENGTH),
                                                                                                      write_tcp_buffer(0)
{
}

void AsioSocketClient::start()
{
  this->start_tcp_connect(this->tcp_endpoints);
}

void AsioSocketClient::stop()
{
  this->stop_tcp_connect();
}

void AsioSocketClient::sendPayload(RequestType requestType, std::vector<uint8_t> &payload)
{
  if (requestType == RequestType::AUDIO_OUT || requestType == RequestType::VIDEO_OUT) {
    //THIS WILL GO ONTO A UDP COMMUNICATION LINE SO JUST LOG AS NOT IMPLEMENTED
  } else {
    write_over_tcp(payload);
  }
}

void AsioSocketClient::registerInstructionReceiver(InstructionReceivedCallback callback)
{
  this->instructionReceivedCallback = callback;
}

void AsioSocketClient::registerPayloadReceiver(PayloadReceivedCallback callback)
{
  this->payloadReceivedCallback = callback;
}
// PRIVATE CALLS
void AsioSocketClient::start_tcp_connect(tcp::resolver::results_type endpoint)
{
  async_connect(tcp_socket, endpoint, [this](std::error_code error, tcp::endpoint) {
    if (!error) {
      spdlog::get("chat-client")->info("TCP CONNECTED");
      do_tcp_read();
    } else {
      spdlog::get("chat-client")->error("TCP CONNECT ERROR {}", error.message());
    }
  });
}

void AsioSocketClient::stop_tcp_connect()
{

  asio::post(context, [this]() { 
    if (tcp_socket.is_open()) {
    tcp_socket.close();
  } });
}

void AsioSocketClient::do_tcp_read()
{
  asio::async_read(
    tcp_socket,
    asio::buffer(read_tcp_buffer.data(), read_tcp_buffer.size()),
    [&](const std::error_code &error, size_t bytes_transferred) -> size_t {
      if (error) {
        spdlog::get("chat-client")->error("TCP READ ERROR {}", error.message());
        return 0;
      }
      if (bytes_transferred < TCP_HEADER_LENGTH) {
        return TCP_HEADER_LENGTH - bytes_transferred;
      }
      const int payload_size = ntohl(*reinterpret_cast<uint32_t *>(read_tcp_buffer.data() + 2));
      const int whole_message_length = payload_size + TCP_HEADER_LENGTH;
      size_t remaining = whole_message_length - bytes_transferred;
      remaining = std::max(remaining, (size_t)0);
      spdlog::get("chat-client")->info("REMAINING: {}", remaining);
      return remaining;
    },
    [this](const std::error_code &error, size_t bytes_transferred) -> void {
      if (!error) {
        if (bytes_transferred > 0) {
          spdlog::get("chat-client")->info("RECEIVED : {} TO TRANSFER AS AN INSTRUCTION", bytes_transferred);
          auto type = static_cast<ResponseType>(ntohs(*reinterpret_cast<uint16_t *>(read_tcp_buffer.data())));
          if (this->instructionReceivedCallback) {
            std::vector<uint8_t> packet(read_tcp_buffer.cbegin() + TCP_HEADER_LENGTH, read_tcp_buffer.cend());
            this->instructionReceivedCallback(type, packet);
          }
        }
        do_tcp_read();
      } else {
        spdlog::get("chat-client")->error("TCP-ERROR ON READ {}", error.message());
        tcp_socket.close();
      }
    });
}

void AsioSocketClient::write_over_tcp(std::vector<uint8_t> &data)
{
  // asio::post(context,
  //   [this, &data]() {

  //   });
  const bool write_in_progress = !write_tcp_buffer.empty();
  write_tcp_buffer.push_back(std::move(data));
  if (!write_in_progress) {
    do_tcp_write();
  }
}

void AsioSocketClient::do_tcp_write()
{
  auto content = write_tcp_buffer.front();
  asio::async_write(tcp_socket,
    asio::buffer(content.data(), content.size()),
    [this](const std::error_code& error, size_t bytes_transferred) -> void {
      spdlog::get("chat-client")->warn("I'M SUPPOSED TO LISTEN HERE RIGHT AFTER WRITING CORRECT {}", bytes_transferred);
      if (!error) {
        spdlog::get("chat-client")->info("WROTE PAYLOAD OVER TCP SOCKET SIZE {}", bytes_transferred);
        write_tcp_buffer.pop_front();
        if (!write_tcp_buffer.empty()) {
          do_tcp_write();
        }
      } else {
        spdlog::get("chat-client")->error("TCP WRITE FAILED {}", error.message());
        tcp_socket.close();
      }
    });
}

AsioSocketClient::~AsioSocketClient()
{
  read_tcp_buffer.clear();
  write_tcp_buffer.clear();
}

};// namespace core::networking::socket