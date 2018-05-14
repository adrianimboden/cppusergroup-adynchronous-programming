#pragma once

#include <experimental/coroutine>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "../http_client.h"
#include "../task.h"

template <typename... Args>
struct std::experimental::coroutine_traits<task<void>, Args...> {
  struct promise_type {
    task<void> p = make_task<void>();

    auto get_return_object() {
      return p;
    }
    std::experimental::suspend_never initial_suspend() {
      return {};
    }
    std::experimental::suspend_never final_suspend() {
      return {};
    }
    void set_exception(std::exception_ptr e) {
      (void)e;
      std::cerr << "exceptions not supported by this implementation"
                << std::endl;
      std::terminate();
    }
    void unhandled_exception() {
      std::terminate();
    }
    void return_void() {
      p->set_result();
    }
  };
};

template <typename R, typename... Args>
struct std::experimental::coroutine_traits<task<R>, Args...> {
  struct promise_type {
    task<R> p = make_task<R>();

    auto get_return_object() {
      return p;
    }
    std::experimental::suspend_never initial_suspend() {
      return {};
    }
    std::experimental::suspend_never final_suspend() {
      return {};
    }
    void set_exception(std::exception_ptr e) {
      (void)e;
      std::cerr << "exceptions not supported by this implementation"
                << std::endl;
      std::terminate();
    }
    void unhandled_exception() {
      std::terminate();
    }
    template <typename U>
    void return_value(U&& u) {
      p->set_result(std::forward<U>(u));
    }
  };
};

template <typename R>
auto operator co_await(task<R> f) {
  struct Awaiter {
    task<R>          input;
    std::optional<R> output;

    bool await_ready() {
      return output.has_value();
    }
    auto await_resume() {
      return *output;
    }
    void await_suspend(std::experimental::coroutine_handle<> coro) {
      input->then([this, coro](auto result) mutable {
        this->output = std::move(result);
        coro.resume();
      });
    }
  };
  return Awaiter{f, std::nullopt};
}

namespace coroutines_ts {

class HttpClient {
public:
  virtual ~HttpClient()                                          = default;
  virtual task<HttpResponse> request_get(const std::string& uri) = 0;
};

task<std::vector<std::string>>
request_uris(HttpClient&                     http_client,
             const std::vector<std::string>& uris_to_request);
}
