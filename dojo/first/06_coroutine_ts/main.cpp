#include "http_client.h"

namespace coroutines_ts {

task<std::vector<std::string>>
request_uris(HttpClient&                     http_client,
             const std::vector<std::string>& uris_to_request) {
  (void)http_client;
  (void)uris_to_request;
  co_return std::vector<std::string>{{"42"}};
}
}
