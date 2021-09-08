#ifndef __CORE_NETWORKING_CPR_HTTP_HANDLER__
#define __CORE_NETWORKING_CPR_HTTP_HANDLER__

#include <core/networking/http_handler.h>

namespace core::networking::http {
class CPRHttpHandler : public HttpHandler
{
public:
  CPRHttpHandler(const std::string& account_service_base_url, const std::string& authorization_service_base_url);
  ~CPRHttpHandler();
  void fetch_session(const Credentials& credentials, FetchSessionCallback fetchSessionCallback) override;
  void register_user(const RegistrationDetails& registration_details, RegistrationCallback registrationCallback) override;
  void refresh_session(InvokationCallback invocationCallback) override;
  void end_session(InvokationCallback invocationCallback) override;
  const std::string &fetch_current_token() override;
private:
  Token token;
  std::string _account_base_url;
  std::string _authorization_base_url;
};
};// namespace core::networking::http
#endif// __CORE_NETWORKING_CPR_HTTP_HANDLER__