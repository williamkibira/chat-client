#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <protos/protocols.pb.h>
#include <protos/identification.pb.h>
#include <fmt/format.h>
#include <iostream>
#include <string>
#include <core/networking/asio_socket.h>
#include <core/messages/protobuf_instruction_handler.h>
#include <core/networking/cpr_http_handler.h>
#include <chrono>
#include <google/protobuf/util/time_util.h>

using namespace core::networking::socket;
using namespace core::networking::http;
using namespace core::messages;
using namespace google::protobuf::util;

auto main(int argc, char **argv) -> int
{
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  auto console = spdlog::stdout_color_mt("chat-client");
  asio::io_context context;
  asio::ip::tcp::resolver tcp_resolver(context);
  auto tcp_endpoints = tcp_resolver.resolve(asio::ip::tcp::v4(), "127.0.0.1", "5200");
  auto client = std::make_shared<AsioSocketClient>(context, tcp_endpoints);
  CPRHttpHandler httpHandler(
    std::string("http://localhost:5000/api/v1/account-service/accounts"),
    std::string("http://localhost:5100/api/v1/authorization-service"));
  ProtobufInstructionHandler instructor;

  client->registerInstructionReceiver([&instructor, &console, &httpHandler](ResponseType responseType, const std::vector<uint8_t> &payload) {
    // FOR THE TIME BEING WE CAN ASSUME THE TYPE OF INSTRUCTIONS TO BE HANDLED
    if (responseType == ResponseType::REQUEST_IDENTITY) {
      console->info("SERVER IS REQUESTING CLIENT IDENTITY");
      Credentials credentials;
      credentials.set_email("williamkibira@gmail.com");
      credentials.set_password("willnux90");
      credentials.set_device_id("DELL XPS 15: HOME PC");
      httpHandler.fetch_session(credentials, [&](std::optional<UserDetails> user_details, const std::string &error) {
        if (user_details.has_value()) {
          Identification identification;
          auto *device = new Device();
          device->set_name("WINDOWS DEV");
          device->set_operating_system("WINDOWS 10 HOME EDITION");
          device->set_version("20H10");
          device->set_ip_address("127.0.0.1");
          identification.set_token(httpHandler.fetch_current_token());
          identification.set_allocated_device(device);
          console->info("SENDING IDENTIFICATION TO SERVER");
          instructor.sendIdentification(identification);
        } else {
          console->error("ERROR: {}", error);
        }
      });
    } else if (responseType == ResponseType::IDENTITY_REJECTION) {
      console->info("YOUR IDENTITY WAS NOT ACCEPTED");
      Failure failure;
      failure.ParseFromArray(payload.data(), payload.size());
      console->error("ERROR: {}", failure.error());
      console->error("DETAILS: {}", failure.details());
      console->error("OCCURRED AT: {}", TimeUtil::ToString(failure.occurred_at()));
      instructor.disconnect();
    } else if (responseType == ResponseType::IDENTITY_ACCEPTED) {
      Info success;
      success.ParseFromArray(payload.data(), payload.size());
      console->info("MESSAGE: {}", success.message());
      console->info("DETAILS: {}", success.details());
      console->info("OCCURRED AT: {}", TimeUtil::ToString(success.occurred_at()));
    } else if (responseType == ResponseType::DISCONNECTION_ACCEPTED) {
      Info success;
      success.ParseFromArray(payload.data(), payload.size());
      console->info("MESSAGE: {}", success.message());
      console->info("DETAILS: {}", success.details());
      console->info("OCCURRED AT: {}", TimeUtil::ToString(success.occurred_at()));
    }
  });

  client->registerPayloadReceiver([&](const std::vector<uint8_t> &payload) {
    //TODO: We will not be handling any UDP/IP payloads at this point
  });

  instructor.onInstructionIssued([&console, &client](RequestType requestType, std::vector<uint8_t> &payload) {
    client->sendPayload(requestType, payload);
  });
  try {
    client->start();
    console->info("STARTING APPLICATION");
    while (true) {
      context.run();
    }
    client->stop();
  } catch (std::exception &e) {
    console->error("FAILED: {}", e.what());
  }
  google::protobuf::ShutdownProtobufLibrary();
  return EXIT_SUCCESS;
}
