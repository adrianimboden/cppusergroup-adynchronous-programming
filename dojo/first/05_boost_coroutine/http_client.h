#pragma once

#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "../http_client.h"
#include "../task.h"
#include <boost/coroutine2/all.hpp>

namespace boost_coroutine {

struct Await {
public:
  using Continue = std::function<void()>;
  using Yield    = std::function<void()>;

  explicit Await(Continue continue_coroutine, Yield yield)
    : continue_coroutine_{std::move(continue_coroutine)}
    , yield_{std::move(yield)} {
  }

  template <typename T>
  T operator()(task<T> fut) {
    if constexpr (std::is_same_v<T, void>) {
      fut->then([&]() { //
        continue_coroutine_();
      });
      yield_();
    } else {
      std::optional<T> result;
      fut->then([&](auto got_result) { //
        result = got_result;
        continue_coroutine_();
      });
      yield_();
      return *result;
    }
  }

private:
  Continue continue_coroutine_;
  Yield    yield_;
};

template <typename Fn>
auto async(Fn fn) {
  using T = std::decay_t<decltype(fn(std::declval<Await>()))>;

  using Coro = boost::coroutines2::coroutine<int>;

  auto fut = make_task<T>();

  auto ptr_to_ptr_to_coro
    = std::make_shared<std::shared_ptr<Coro::pull_type>>();
  *ptr_to_ptr_to_coro = std::make_shared<Coro::pull_type>(
    boost::coroutines2::fixedsize_stack(),
    [fut, ptr_to_ptr_to_coro, fn](auto& yield) {
      Await await{[&] { //
                    auto ptr_to_coro = *ptr_to_ptr_to_coro;
                    get_current_executor().add_work([ptr_to_coro] { //
                      assert(ptr_to_coro);
                      auto& coro = *ptr_to_coro;
                      coro(); // continue
                    });
                  },
                  [&] {
                    yield(0); // suspend
                  }};
      if constexpr (std::is_same_v<std::decay_t<decltype(fn(await))>, void>) {
        fn(await);
        get_current_executor().add_work([=] { //
          fut->set_result();
        });
      } else {
        auto result = fn(await);
        get_current_executor().add_work([=] { //
          fut->set_result(std::move(result));
        });
      }
    });
  return fut;
}

class HttpClient {
public:
  virtual ~HttpClient()                                          = default;
  virtual task<HttpResponse> request_get(const std::string& uri) = 0;
};

task<std::vector<std::string>>
request_uris(HttpClient&                     http_client,
             const std::vector<std::string>& uris_to_request);
}
