#include <core/networking/cpr_http_handler.h>
#include <cpr/cpr.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
namespace core::networking::http {
CPRHttpHandler::CPRHttpHandler(const std::string &account_service_base_url, const std::string &authorization_service_base_url)
  : _account_base_url(account_service_base_url),
    _authorization_base_url(authorization_service_base_url)
{
}
CPRHttpHandler::~CPRHttpHandler()
{
}
void CPRHttpHandler::fetch_session(const Credentials &credentials, FetchSessionCallback fetchSessionCallback)
{
  const std::string content = credentials.SerializeAsString();

  cpr::Response response = cpr::Post(cpr::Url{ fmt::format("{0}{1}", _authorization_base_url, "/request-session") },
    cpr::Body(content),
    cpr::Header{ { "Content-Type", "application/x-protobuf" } });
  if (response.status_code != 200) {
    fetchSessionCallback(std::nullopt, response.error.message);
  } else {
    this->token.clear_token();
    this->token.ParseFromString(response.text);
    response = cpr::Get(cpr::Url{ fmt::format("{0}{1}", _authorization_base_url, "/user-details") },
      cpr::Bearer{ token.token() });
    if (response.status_code != 200) {
      fetchSessionCallback(std::nullopt, response.error.message);
    } else {
      UserDetails details;
      details.ParseFromString(response.text);
      fetchSessionCallback(std::optional<std::reference_wrapper<UserDetails>>{ details }, { "" });
    }
  }
}
void CPRHttpHandler::register_user(const RegistrationDetails &registration_details, RegistrationCallback registrationCallback)
{
  const std::string content = registration_details.SerializeAsString();
  cpr::Response response = cpr::Post(cpr::Url{ fmt::format("{0}{1}", _account_base_url, "/register") },
    cpr::Body(content),
    cpr::Header{ { "Content-Type", "application/x-protobuf" } });
  if (response.status_code != 201) {
    registrationCallback(std::nullopt, response.error.message);
  } else {
    registrationCallback(std::optional<std::reference_wrapper<std::string>>{ response.text }, { "" });
  }
}
void CPRHttpHandler::refresh_session(InvokationCallback invocationCallback)
{
  cpr::Response response = cpr::Get(cpr::Url{ fmt::format("{0}{1}", _authorization_base_url, "/renew-session") },
    cpr::Bearer{ token.token() });
  if (response.status_code != 200) {
    invocationCallback(response.error.message);
  } else {
    token.clear_token();
    token.ParseFromString(response.text);
    invocationCallback({ "" });
  }
}
void CPRHttpHandler::end_session(InvokationCallback invocationCallback)
{
  cpr::Response response = cpr::Put(cpr::Url{ fmt::format("{0}{1}", _authorization_base_url, "/end-session") },
    cpr::Bearer{ token.token() });
  if (response.status_code != 200) {
    invocationCallback(response.error.message);
  } else {
    token.clear_token();
    token.ParseFromString(response.text);
    invocationCallback({ "" });
  }
}
const std::string &CPRHttpHandler::fetch_current_token()
{
  return token.token();
}
}// namespace core::networking::http