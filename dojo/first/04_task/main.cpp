#include "http_client.h"

namespace task_based {

task<std::vector<std::string>>
request_uris(HttpClient&                     http_client,
             const std::vector<std::string>& uris_to_request) {
  (void)http_client;
  (void)uris_to_request;

  auto task = make_task<std::vector<std::string>>();
  task->set_result({"42"});
  return task;
}
}
