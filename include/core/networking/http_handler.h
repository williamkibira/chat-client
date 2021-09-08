#ifndef __CORE_NETWORKING_HTTP_HANDLER__
#define __CORE_NETWORKING_HTTP_HANDLER__

#include <functional>
#include <protos/authentication.pb.h>
#include <protos/protocols.pb.h>
#include <optional>
namespace core::networking::http {
class HttpHandler
{
public:
  typedef std::function<void(std::optional<std::string> id, const std::string& error)> RegistrationCallback;
  typedef std::function<void(std::optional<UserDetails> user_details, const std::string& error)> FetchSessionCallback;
  typedef std::function<void(const std::string& error)> InvokationCallback;
  virtual void fetch_session(const Credentials& credentials, FetchSessionCallback fetchSessionCallback) = 0;
  virtual void register_user(const RegistrationDetails& registration_details, RegistrationCallback registrationCallback) = 0;
  virtual void refresh_session(InvokationCallback invocationCallback) = 0;
  virtual void end_session(InvokationCallback invocationCallback) = 0;
  virtual const std::string& fetch_current_token()=0;
};
};// namespace core::networking::http
#endif// __CORE_NETWORKING_HTTP_HANDLER__
