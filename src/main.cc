#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <protos/protocols.pb.h>
#include <protos/identification.pb.h>
#include <fmt/format.h>
#include <iostream>
#include <string>
#include <core/networking/asio_socket.h>
#include <core/messages/protobuf_instruction_handler.h>
#include <chrono>
using namespace core::networking::socket;
using namespace core::messages;

auto main(int argc, char **argv) -> int
{
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  auto console = spdlog::stdout_color_mt("chat-client");
  asio::io_context context;
  asio::ip::tcp::resolver tcp_resolver(context);
  auto tcp_endpoints = tcp_resolver.resolve(asio::ip::tcp::v4(), "127.0.0.1", "5200");
  auto client = std::make_shared<AsioSocketClient>(context, tcp_endpoints);
  ProtobufInstructionHandler instructor;

  client->registerInstructionReceiver([&instructor, &console](ResponseType responseType, const uint8_t *buffer, int length) {
    console->info("GOT INSTRUCTION");
    instructor.onReceived(responseType, length, buffer);
  });

  client->registerPayloadReceiver([&](const std::vector<uint8_t> &payload) {
    //TODO: We will not be handling any TCP/IP payloads at this point
  });
  instructor.onInstructionIssued([&console, &client](RequestType requestType, const std::vector<uint8_t> &payload) {
    console->info("SENDING REQUEST");
    client->sendPayload(requestType, payload);
  });
  try {
    client->start();
    console->info("STARTING APPLICATION");
    
    while (true) {
     context.run();
     console->info("DEAD");
    }
    client->stop();
    console->info("WHAT'S KILLING YOU");
  } catch (std::exception &e) {
    console->error("FAILED: {}", e.what());
  }
  google::protobuf::ShutdownProtobufLibrary();
  return EXIT_SUCCESS;
}
