#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <protos/protocols.pb.h>
#include <protos/identification.pb.h>
#include <fmt/format.h>
#include <iostream>
#include <string>
#include <core/networking/asio_socket.h>
#include <core/networking/libuv_socket.h>
#include <core/messages/protobuf_instruction_handler.h>
#include <chrono>
using namespace core::networking::socket;
using namespace core::messages;

auto main(int argc, char **argv) -> int
{
  #ifdef _WIN32
		// probably bug:
		// src/win/winapi.c assumes
		// that advapi32.dll has been already loaded,
		// so load it before using anything from winapi.c
		// for static build
		LoadLibrary("advapi32.dll");
	#endif
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  auto console = spdlog::stdout_color_mt("chat-client");
  console->info("LIB UV TRIAL");
  // asio::io_context context;
  // asio::ip::tcp::resolver tcp_resolver(context);
  //auto tcp_endpoints = tcp_resolver.resolve(asio::ip::tcp::v4(), "127.0.0.1", "5200");
  //auto client = std::make_shared<AsioSocketClient>(context, tcp_endpoints);
  uv_loop_t loop;
  uv_loop_init(&loop);
  console->info("LOOP INITIALIZED");
  auto client = std::make_shared<UVSocketClient>(&loop, std::string("127.0.0.1"), 5200);
  console->info("CREATED CLIENT INSTANCE");
  ProtobufInstructionHandler instructor;

  client->registerInstructionReceiver([&instructor, &console](ResponseType responseType, const std::vector<uint8_t> &payload) {
    console->info("GOT INSTRUCTION");
    instructor.onReceived(responseType, payload);
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
     // context.run();
     uv_run(&loop, UV_RUN_DEFAULT);
    }
    client->stop();
    console->info("WHAT'S KILLING YOU");
  } catch (std::exception &e) {
    console->error("FAILED: {}", e.what());
  }
  google::protobuf::ShutdownProtobufLibrary();
  return EXIT_SUCCESS;
}
