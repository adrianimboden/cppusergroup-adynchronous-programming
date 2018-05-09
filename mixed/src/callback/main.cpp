#include "http_client.h"

#include <optional>

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...)->overloaded<Ts...>;

namespace callback {

static void request_get(HttpClient& http_client, std::string uri, const std::function<void(std::string)>& cb) {
    http_client.request_get(uri, [cb, &http_client](auto&& response) {
        std::visit(
            overloaded{//
                       [&](Redirect redirect) { request_get(http_client, redirect.target, cb); },
                       [&](Content content) { cb(content.content); }},
            response);
    });
}

void request_uris(HttpClient&                                          http_client,
                  const std::vector<std::string>&                      uris_to_request,
                  const std::function<void(std::vector<std::string>)>& cb) {
    const auto all_results    = std::make_shared<std::vector<std::string>>(uris_to_request.size());
    size_t     index          = 0;
    auto       finish_counter = std::make_shared<size_t>(0);
    for (auto uri_to_request : uris_to_request) {
        request_get(http_client, uri_to_request, [=](auto one_result) {
            (*all_results)[index] = std::move(one_result);
            ++(*finish_counter);
            if (all_results->size() == *finish_counter) {
                cb(*all_results);
            }
        });
        ++index;
    }
}
}
