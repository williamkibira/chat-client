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
  // https://qr.ae/pNr0pG just to laugh.
  asio::async_read(
    tcp_socket,
    asio::buffer(read_tcp_buffer, MAX_TCP_LENGTH),
    [&](const std::error_code &error, size_t bytes_transferred) -> size_t {
      if (error) {
        spdlog::get("chat-client")->info("TCP READ ERROR {}", error.message());
        return 0;
      }
      spdlog::get("chat-client")->info("BYTES RECEIVED OVER TCP TO EVALUATE {}", bytes_transferred);

      if (bytes_transferred < TCP_HEADER_LENGTH) {
        return TCP_HEADER_LENGTH - bytes_transferred;
      }
      const int payload_size = ntohl(*reinterpret_cast<uint32_t *>(this->read_tcp_buffer.data() + 2));
      const int whole_message_length = payload_size + 6;
      size_t remaining = whole_message_length - bytes_transferred;
      remaining = std::max(remaining, (size_t)0);
      return remaining;
    },
    std::bind(&AsioSocketClient::on_tcp_read, this, std::placeholders::_1, std::placeholders::_2));
}

void AsioSocketClient::on_tcp_read(const std::error_code error, size_t bytes_transferred)
{
  if (!error) {
    if (bytes_transferred > 0) {
      auto type = static_cast<ResponseType>(ntohs(*reinterpret_cast<uint16_t *>(read_tcp_buffer.data())));
      if (this->instructionReceivedCallback) {
        this->instructionReceivedCallback(type, read_tcp_buffer);
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
  spdlog::get("chat-client")->info("DOING CHECK ON TCP WRITE");
  try {
    spdlog::get("chat-client")->warn("DOING WRITE ON TCP SOCKET");
    auto content = write_tcp_buffer.front();
    asio::async_write(tcp_socket,
      asio::buffer(content.data(), content.size()),
      std::bind(&AsioSocketClient::on_tcp_write, this, std::placeholders::_1, std::placeholders::_2));
    spdlog::get("chat-client")->warn("DONE WRITING");
  } catch (std::exception &e) {
    spdlog::get("chat-client")->error("TCP ERR:{}", e.what());
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
}

};// namespace core::networking::socket