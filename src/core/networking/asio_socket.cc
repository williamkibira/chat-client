#include <core/networking/asio_socket.h>
#include <spdlog/spdlog.h>
#include <core/definitions/message_types.h>
#include <core/definitions/header_length.h>

namespace core::networking::socket {
AsioSocketClient::AsioSocketClient(io_context &context, tcp::resolver::results_type &tcp_endpoints) : context(context),
                                                                                                      tcp_socket(context),
                                                                                                      tcp_endpoints(tcp_endpoints),
                                                                                                      read_tcp_buffer(0),
                                                                                                      write_tcp_buffer(0)
{
  this->raw_buffer = new uint8_t[MAX_TCP_LENGTH];
}

void AsioSocketClient::start()
{
  this->start_tcp_connect(this->tcp_endpoints);
}

void AsioSocketClient::stop()
{
  this->stop_tcp_connect();
}

void AsioSocketClient::sendPayload(RequestType requestType, const std::vector<uint8_t> &payload)
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
 try{
  asio::async_read(
    tcp_socket,
    asio::buffer(raw_buffer, MAX_TCP_LENGTH),
    [&](const std::error_code &error, size_t bytes_transferred) -> size_t {
      if (error) {
        spdlog::get("chat-client")->error("TCP READ ERROR {}", error.message());
        return 0;
      }
      spdlog::get("chat-client")->info("BYTES RECEIVED OVER TCP TO EVALUATE {}", bytes_transferred);

      if (bytes_transferred < TCP_HEADER_LENGTH) {
        return TCP_HEADER_LENGTH - bytes_transferred;
      }
      const int payload_size = ntohl(*reinterpret_cast<uint32_t *>(raw_buffer + 2));
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
          auto type = static_cast<ResponseType>(ntohs(*reinterpret_cast<uint16_t *>(raw_buffer)));
          if (this->instructionReceivedCallback) {
            this->instructionReceivedCallback(type, &raw_buffer[TCP_HEADER_LENGTH], (int)bytes_transferred - TCP_HEADER_LENGTH);
          }
        }
        do_tcp_read();
      } else if (error) {
        spdlog::get("chat-client")->error("TCP-ERROR ON READ {}", error.message());
      }
    });
 } catch(std::exception& e) {
   spdlog::get("chat-client")->error("FAILED TO READ: {}", e.what());
 }
}

void AsioSocketClient::on_tcp_read(const std::error_code error, size_t bytes_transferred)
{
  if (!error) {
    if (bytes_transferred > 0) {
      spdlog::get("chat-client")->info("RECEIVED : {} TO TRANSFER AS AN INSTRUCTION", bytes_transferred);
      auto type = static_cast<ResponseType>(ntohs(*reinterpret_cast<uint16_t *>(raw_buffer)));
      if (this->instructionReceivedCallback) {
        this->instructionReceivedCallback(type, &raw_buffer[TCP_HEADER_LENGTH], (int)bytes_transferred - TCP_HEADER_LENGTH);
      }
    }
    do_tcp_read();
  } else if (error) {
    spdlog::get("chat-client")->error("TCP-ERROR ON READ {}", error.message());
  }
}

void AsioSocketClient::write_over_tcp(const std::vector<uint8_t> &data)
{
  // asio::post(context,
  //   [this, data]() {

  //   });
  const bool write_in_progress = !write_tcp_buffer.empty();
  write_tcp_buffer.push_back(data);
  if (!write_in_progress) {
    do_tcp_write();
  }
}

void AsioSocketClient::do_tcp_write()
{
  // try {
  //   spdlog::get("chat-client")->warn("DOING WRITE ON TCP SOCKET");
  //   auto content = write_tcp_buffer.front();
  //   asio::async_write(tcp_socket,
  //     asio::buffer(content.data(), content.size()),
  //     std::bind(&AsioSocketClient::on_tcp_write, this, std::placeholders::_1, std::placeholders::_2));
  // } catch (std::exception &e) {
  //   spdlog::get("chat-client")->error("TCP ERR:{}", e.what());
  // }
  spdlog::get("chat-client")->info("PERFORMING BLOCKING WRITE");
  auto content = write_tcp_buffer.front();
  size_t written = asio::write(tcp_socket, asio::buffer(content.data(), content.size()));
  if (written < 0) {
    spdlog::get("chat-client")->error("WROTE LESS : {} OF {}", written, content.size());
  } else {
    spdlog::get("chat-client")->info("WROTE PAYLOAD OVER TCP SOCKET SIZE {}", written);
    write_tcp_buffer.pop_front();
    spdlog::get("chat-client")->info("POPPED THE FRONT OF THE QUEUE");
    if (!write_tcp_buffer.empty()) {
      do_tcp_write();
    }
    spdlog::get("chat-client")->info("We have no more to write");
    do_tcp_read();
  }
}

void AsioSocketClient::on_tcp_write(const std::error_code error, size_t bytes_transferred)
{
  spdlog::get("chat-client")->warn("I'M SUPPOSED TO LISTEN HERE RIGHT AFTER WRITING CORRECT {}", bytes_transferred);
  if (!error) {
    spdlog::get("chat-client")->info("WROTE PAYLOAD OVER TCP SOCKET SIZE {}", bytes_transferred);
    write_tcp_buffer.pop_front();
    if (!write_tcp_buffer.empty()) {
      do_tcp_write();
    }
  } else {
    spdlog::get("chat-client")->error("TCP WRITE FAILED {}", error.message());
  }
}

AsioSocketClient::~AsioSocketClient()
{
  delete[] raw_buffer;
}

};// namespace core::networking::socket