#include "http_client.h"

#include <optional>

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...)->overloaded<Ts...>;

namespace synchronous {

std::vector<std::string> request_uris(HttpClient& http_client, const std::vector<std::string>& uris_to_request) {
    auto get_one_uri = [&](auto uri_to_request) {
        auto result   = std::string{};
        auto next_uri = std::optional<std::string>{uri_to_request};
        while (next_uri) {
            const auto response = http_client.request_get(*next_uri);

            next_uri = std::nullopt;
            std::visit(
                overloaded{//
                           [&](Redirect redirect) { next_uri = redirect.target; },
                           [&](Content content) { result = content.content; }},
                response);
        }
        return result;
    };

    auto responses = std::vector<std::string>{};
    std::transform(uris_to_request.begin(), uris_to_request.end(), std::back_inserter(responses), [&](auto uri) {
        return get_one_uri(uri);
    });
    return responses;
}
}
