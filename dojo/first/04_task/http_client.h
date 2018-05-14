#pragma once

#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "../http_client.h"
#include "../task.h"

namespace task_based {

class HttpClient {
public:
  virtual ~HttpClient()                                          = default;
  virtual task<HttpResponse> request_get(const std::string& uri) = 0;
};

task<std::vector<std::string>>
request_uris(HttpClient&                     http_client,
             const std::vector<std::string>& uris_to_request);
}
