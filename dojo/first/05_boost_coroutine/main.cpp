#include "http_client.h"

#include <iostream>
#include <optional>

template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...)->overloaded<Ts...>;

namespace boost_coroutine {

static task<std::string> get_one_uri(HttpClient&        http_client,
                                     const std::string& uri_to_request) {
  return async([&http_client, uri_to_request](Await await) {
    auto result   = std::string{};
    auto next_uri = std::optional<std::string>{uri_to_request};
    while (next_uri) {
      const auto response = await(http_client.request_get(*next_uri));

      next_uri = std::nullopt;
      std::visit(
        overloaded{//
                   [&](Redirect redirect) { next_uri = redirect.target; },
                   [&](Content content) { result = content.content; }},
        response);
    }
    return result;
  });
};

task<std::vector<std::string>>
request_uris(HttpClient&                     http_client,
             const std::vector<std::string>& uris_to_request) {
  return async([&http_client, uris_to_request](Await await) {
    auto async_responses = std::vector<task<std::string>>{};
    std::transform(uris_to_request.begin(),
                   uris_to_request.end(),
                   std::back_inserter(async_responses),
                   [&](auto uri) { return get_one_uri(http_client, uri); });

    std::vector<std::string> responses;
    std::transform(async_responses.begin(),
                   async_responses.end(),
                   std::back_inserter(responses),
                   [&](auto&& task) {
                     auto result = await(task);
                     return result;
                   });
    return responses;
  });
}
}
