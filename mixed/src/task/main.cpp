#include "http_client.h"

#include <iostream>
#include <optional>

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...)->overloaded<Ts...>;

namespace task_based {

static task<std::string> request_get(HttpClient& http_client, std::string uri) {
    return http_client.request_get(uri)->then([&http_client, uri](auto response) {
        return std::visit(
            overloaded{                                    //
                       [&http_client](Redirect redirect) { //
                           return request_get(http_client, redirect.target);
                       },
                       [](Content content) { //
                           return create_finished_task(content.content);
                       }},
            response);
    });
}

task<std::vector<std::string>> request_uris(HttpClient& http_client, const std::vector<std::string>& uris_to_request) {

    auto tasks = std::vector<task<std::string>>{};
    std::transform(uris_to_request.begin(), uris_to_request.end(), std::back_inserter(tasks), [&](auto uri) {
        return request_get(http_client, uri);
    });

    return when_all(tasks.begin(), tasks.end());
}
}
