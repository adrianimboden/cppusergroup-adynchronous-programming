#include "http_client.h"

#include <iostream>
#include <optional>
#include <sstream>

#if 1
static inline auto log_msg = std::stringstream{};
#else
static inline auto& log_msg = std::cerr;
#endif

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...)->overloaded<Ts...>;

namespace coroutines_ts {

task<std::vector<std::string>> request_uris(HttpClient&                     http_client,
                                              const std::vector<std::string>& uris_to_request) {
    const auto get_one_uri = [&](auto uri_to_request) -> task<std::string> {
        auto result   = std::string{};
        auto next_uri = std::optional<std::string>{uri_to_request};
        while (next_uri) {
            log_msg << "request " << *next_uri << "..." << std::endl;
            const auto response = co_await http_client.request_get(*next_uri);
            log_msg << "...request " << *next_uri << std::endl;

            next_uri = std::nullopt;
            std::visit(
                overloaded{//
                           [&](Redirect redirect) { next_uri = redirect.target; },
                           [&](Content content) { result = content.content; }},
                response);
        }
        co_return result;
    };

    auto async_responses = std::vector<task<std::string>>{};
    std::transform(uris_to_request.begin(), uris_to_request.end(), std::back_inserter(async_responses), [&](auto uri) {
        return get_one_uri(uri);
    });

    std::vector<std::string> responses;
    for (auto&& async_response : async_responses) {
        log_msg << "co_await..." << std::endl;
        responses.emplace_back(co_await async_response);
        log_msg << "...co_await: " << (co_await async_response) << std::endl;
    }
    co_return responses;
}
}
