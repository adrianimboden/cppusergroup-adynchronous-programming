#include "http_client.h"

namespace callback {

void request_uris(HttpClient&                     http_client,
                  const std::vector<std::string>& uris_to_request,
                  const std::function<void(std::vector<std::string>)>& cb) {
  (void)http_client;
  (void)uris_to_request;
  cb({"42"});
}
}
