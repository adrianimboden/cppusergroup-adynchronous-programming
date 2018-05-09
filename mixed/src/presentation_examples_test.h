#pragma once

#include "boost_coroutine/http_client.h"
#include "callback/http_client.h"
#include "coroutine_ts/http_client.h"
#include "sync/http_client.h"
#include "task/http_client.h"
#include "threaded/http_client.h"

#include "http_client_implementations.h"

#include <future>
//
//
//
//
static void use_response(HttpResponse response) {
  (void)response;
}

namespace synchronous {
void example(HttpClient& http_client) {
  HttpResponse response = http_client.request_get("/file2"); // blocks

  use_response(response);
}
}

namespace threaded {
void example_thread(HttpClient& http_client) {
  auto        response = std::optional<HttpResponse>{};
  std::thread th{[&] { //
    response = http_client.request_get("/file2");
  }};
  /*do something else*/
  th.join(); // blocks

  use_response(*response);
}

void example_async(HttpClient& http_client) {
  std::future<HttpResponse> response_task
    = std::async(std::launch::async, [&http_client] { //
        return http_client.request_get("/file2");
      });
  /*do something else*/
  HttpResponse response = response_task.get(); // blocks
  use_response(response);
}

std::future<void> example_async2(HttpClient& http_client) {
  return std::async(std::launch::async, [&http_client] { //
    HttpResponse response = http_client.request_get("/file2");
    use_response(response);
  });
}
}

namespace callback {

void example(HttpClient& http_client, const std::function<void()>& cb) {
  http_client.request_get("/file2", [cb](HttpResponse response) { //
    use_response(response);
    cb();
  });
}
}
namespace task_based {
void example(HttpClient& http_client) {
  task<HttpResponse> response_task = http_client.request_get("/file2");
  response_task->then([](HttpResponse response) { //
    use_response(response);
  });
}

task<void> example2(HttpClient& http_client) {
  task<HttpResponse> response_task = http_client.request_get("/file2");
  task<void> finish_task = response_task->then([](HttpResponse response) { //
    use_response(response);
  });
  return finish_task;
}
}

namespace boost_coroutine {

task<void> example(HttpClient& http_client) {
  return async([&http_client](Await await) { //
    HttpResponse response = await(http_client.request_get("/file2"));
    use_response(response);
  });
}
}

namespace coroutines_ts {

void example_regular(HttpClient& http_client) {
  task<HttpResponse> response_task = http_client.request_get("/file2");

  response_task->then([](HttpResponse response) { //
    use_response(response);
  });
}

task<void> example(HttpClient& http_client) {
  HttpResponse response = co_await http_client.request_get("/file2");

  use_response(response);
}
}

namespace boost_coroutine_nicer {
using namespace boost_coroutine;

task<void> example(HttpClient& http_client) {
  return async([&http_client](Await await) { //
    HttpResponse response = await(http_client.request_get("/file2"));

    use_response(response);
  });
}
}
