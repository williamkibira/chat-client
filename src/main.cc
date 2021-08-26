#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <protos/protocols.pb.h>
#include <fmt/format.h>
#include <iostream>
#include <string>
#include <asio.hpp>
auto main(int argc, char **argv) -> int {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  auto console = spdlog::stdout_color_mt("hub");

  RegistrationDetails details;
  details.set_first_name("sample-first-name");
  details.set_last_name("sample-last-name");
  details.set_email("sample.email@gmail.com");
  details.set_photo("XXXD");
  details.set_password("SSDJEJ");
  details.add_roles()->append("FOXER");
  std::string content;
  details.SerializeToString(&content);

  std::cout << "STD::STREAM WORKS OK" << std::endl;
  std::cout << content << std::endl;
  fmt::print("CONTENT FMT :{} \n", content);
  console->info("LOGGER WORKS : {}", content);
  google::protobuf::ShutdownProtobufLibrary();
  return EXIT_SUCCESS;
}
