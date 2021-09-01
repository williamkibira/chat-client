#include <core/networking/libuv_socket.h>
#include <spdlog/spdlog.h>
#include <core/definitions/message_types.h>
#include <core/definitions/header_length.h>

namespace core::networking::socket {

UVSocketClient::UVSocketClient(uv_loop_t *loop, const std::string &ip_address, int port) : loop(loop),
                                                                                           ip_address(ip_address),
                                                                                           port(port),
                                                                                           read_tcp_buffer(MAX_TCP_LENGTH)

{
  this->loop->data = this;
  this->write_request = std::unique_ptr<uv_write_t>(new (uv_write_t));
  this->write_request->data = this;
  this->connect.data = this;
  this->socket.data = this;
}
UVSocketClient::~UVSocketClient()
{
  this->write_request.release();
  this->write_request.reset();
}

void UVSocketClient::start()
{
  uv_tcp_init(loop, &socket);
  uv_ip4_addr(ip_address.c_str(), port, &this->dest);
  uv_tcp_connect(&connect, &socket, (const struct sockaddr *)&this->dest, UVSocketClient::on_connect);
}
void UVSocketClient::stop()
{
}

void UVSocketClient::sendPayload(RequestType requestType, std::vector<uint8_t> &payload)
{
  if (requestType == RequestType::AUDIO_OUT || requestType == RequestType::VIDEO_OUT) {
    //THIS WILL GO ONTO A UDP COMMUNICATION LINE SO JUST LOG AS NOT IMPLEMENTED
  } else {
    write_over_tcp(payload);
  }
}

void UVSocketClient::registerInstructionReceiver(InstructionReceivedCallback callback)
{
  this->instructionReceivedCallback = callback;
}
void UVSocketClient::registerPayloadReceiver(PayloadReceivedCallback callback)
{
  this->payloadReceivedCallback = callback;
}

void UVSocketClient::write_over_tcp(std::vector<uint8_t> &data)
{
  const bool write_in_progress = !write_tcp_buffer.empty();
  write_tcp_buffer.push_back(std::move(data));
  if (!write_in_progress) {
    do_tcp_write();
  }
}

void UVSocketClient::allocate_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buffer)
{
  spdlog::get("chat-client")->info("ALLOCATING MEMORY FOR READ");
  auto instance = static_cast<UVSocketClient *>(handle->data);
  instance->read_tcp_buffer.clear();
  *buffer = uv_buf_init((char *)instance->read_tcp_buffer.data(), instance->read_tcp_buffer.capacity());
  spdlog::get("chat-client")->info("ALLOCATED MEMORY SUCCESSFULLY");
}

void UVSocketClient::on_connect(uv_connect_t *req, int status)
{
  if (status == 0) {
    spdlog::get("chat-client")->info("UV ESTABLISHED CONNECTION");
    req->handle->data = req->data;
    uv_read_start(req->handle, UVSocketClient::allocate_buffer, UVSocketClient::on_tcp_read);
  } else {
    spdlog::get("chat-client")->error("FAILED ON CONNECT: {} {}", uv_strerror(status), uv_err_name(status));
  }
}

//What would have been the TCP payload receiver
void UVSocketClient::on_tcp_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buffer)
{
  auto instance = static_cast<UVSocketClient *>(stream->data);
  if (nread == UV_EOF) {
    spdlog::get("chat-client")->error("DISCONNECTED WITH EOF");
    uv_read_stop(stream);
  } else if (nread > 0) {
      spdlog::get("chat-client")->info("CONTENT SIZE: {}", nread);
      auto type = static_cast<ResponseType>(ntohs(*reinterpret_cast<uint16_t *>(buffer->base)));
      spdlog::get("chat-client")->info("GOT ORDER FROM SERVER");
      if (instance->instructionReceivedCallback) {
        instance->instructionReceivedCallback(type, instance->read_tcp_buffer);
      }
  } else {
    uv_read_stop(stream);
  }
  uv_read_start(instance->connect.handle, UVSocketClient::allocate_buffer, UVSocketClient::on_tcp_read);
}

void UVSocketClient::do_tcp_write()
{
  spdlog::get("chat-client")->info("WRITE BUFFER TO TCP SOCKET");
  if (write_tcp_buffer.empty()) {
    spdlog::get("chat-client")->info("WRITE BUFFER IS NOW EMPTY");
    uv_read_start(connect.handle, UVSocketClient::allocate_buffer, UVSocketClient::on_tcp_read);
  } else {
    auto content = write_tcp_buffer.front();
    uv_buf_t buffer;
    buffer.base = (char *)content.data();
    buffer.len = content.size();
    try {
      // spdlog::get("chat-client")->info("SUBMITTING REQUEST TO SEND QUEUE");
      // uv_write(this->write_request.get(), connect.handle, &buffer, 1, [](uv_write_t *req, int status) -> void {
      //   spdlog::get("chat-client")->warn("SUCCESSFULLY WROTE DATA , PREPARE FOR A RECALL");
      //   auto instance = static_cast<UVSocketClient *>(req->data);
      //   spdlog::get("chat-client")->warn("GOT INSTANCE OF HANDLE");
      //   if (status == 0) {
      //     instance->write_tcp_buffer.pop_front();
      //     if (!instance->write_tcp_buffer.empty()) {
      //       instance->do_tcp_write();
      //     }
      //   } else {
      //     spdlog::get("chat-client")->error("FAILED ON WRITE: {} {}", uv_strerror(status), uv_err_name(status));
      //   }
      // });
     int status = uv_try_write(connect.handle, &buffer, 1);
     if(status == 0) {
       spdlog::get("chat-client")->info("WRITE WAS OK");
     } else {
       spdlog::get("chat-client")->error("FAILED ON WRITE: {} {}", uv_strerror(uv_translate_sys_error(status)), uv_err_name(uv_translate_sys_error(status)));
     }
    }catch(std::exception& e) {
      spdlog::get("chat-client")->error("FAILED ON WRITER: {}", e.what());
    }
  }
}
}// namespace core::networking::socket