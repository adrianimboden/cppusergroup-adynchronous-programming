#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "boost_coroutine/http_client.h"
#include "callback/http_client.h"
#include "coroutine_ts/http_client.h"
#include "sync/http_client.h"
#include "task/http_client.h"
#include "threaded/http_client.h"

#include "executor.h"

#include <boost/algorithm/string.hpp>

#include <gsl/gsl_util>

#if 1
static inline auto log_msg = std::stringstream{};
#else
static inline auto& log_msg = std::cerr;
#endif

static HttpResponse request_get_impl(std::string_view uri) {
  if (uri == "/file1") {
    return Redirect{"/newplaceoffile1"};
  }
  if (uri == "/newplaceoffile1") {
    return Redirect{"/file2"};
  }
  if (uri == "/file2") {
    return Content{"content1"};
  }
  if (uri == "/file3") {
    return Content{"content2"};
  }
  if (boost::algorithm::starts_with(uri, "/extremeredirect")) {
    std::vector<std::string> parts;
    boost::algorithm::split(parts, uri, [](char c) { return c == '/'; });
    if (parts.size() == 2) {
      return Redirect{std::string{uri} + "/1"};
    }
    if (parts.size() == 3) {
      auto num = std::stol(parts[2]);
      if (num < 4) {
        return Redirect{"/extremeredirect/" + std::to_string(num + 1)};
      }
    }
    return Content{"finally here"};
  }
  return Content{""};
}

namespace synchronous {
class ConcreteHttpClient : public HttpClient {
public:
  HttpResponse request_get(std::string_view uri) override {
    log_msg << "request " << uri << "..." << std::endl;
    auto out
      = gsl::finally([&] { log_msg << "...request " << uri << std::endl; });
    return request_get_impl(uri);
  }
};
}

namespace threaded {
class ConcreteHttpClient : public HttpClient {
public:
  HttpResponse request_get(std::string_view uri) override {
    log_msg << "request " << uri << "..." << std::endl;
    auto out
      = gsl::finally([&] { log_msg << "...request " << uri << std::endl; });
    return request_get_impl(uri);
  }
};
}

namespace callback {

class ConcreteHttpClient : public HttpClient {
public:
  void request_get(const std::string&                       uri,
                   const std::function<void(HttpResponse)>& cb) override {
    log_msg << "request " << uri << "..." << std::endl;
    auto handle = [uri]() -> HttpResponse { return request_get_impl(uri); };
    get_current_executor().add_work([=] {
      log_msg << "...request " << uri << std::endl;
      get_current_executor().add_work([=] { cb(handle()); });
    });
  }
};
}

namespace task_based {

class ConcreteHttpClient : public HttpClient {
public:
  task<HttpResponse> request_get(const std::string& uri) override {
    log_msg << "request " << uri << "..." << std::endl;
    const auto handle
      = [uri]() -> HttpResponse { return request_get_impl(uri); };

    auto fut = make_task<HttpResponse>();
    get_current_executor().add_work([=] {
      log_msg << "...request " << uri << std::endl;

      fut->set_result(handle());
    });
    return fut;
  }
};
}

namespace boost_coroutine {

class ConcreteHttpClient : public HttpClient {
public:
  task<HttpResponse> request_get(const std::string& uri) override {
    log_msg << "request " << uri << "..." << std::endl;
    const auto handle = [=]() -> HttpResponse { return request_get_impl(uri); };

    auto fut = make_task<HttpResponse>();
    get_current_executor().add_work([=] {
      log_msg << "...request " << uri << std::endl;
      fut->set_result(handle());
    });
    return fut;
  }
};
}

namespace coroutines_ts {

class ConcreteHttpClient : public HttpClient {
public:
  task<HttpResponse> request_get(const std::string& uri) override {
    log_msg << "request " << uri << "..." << std::endl;
    const auto handle = [=]() -> HttpResponse { return request_get_impl(uri); };

    auto fut = make_task<HttpResponse>();
    get_current_executor().add_work([=] {
      log_msg << "...request " << uri << std::endl;
      fut->set_result(handle());
    });
    return fut;
  }
};
}
