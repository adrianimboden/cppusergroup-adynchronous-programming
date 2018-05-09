#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "boost_coroutine/http_client.h"
#include "callback/http_client.h"
#include "coroutine_ts/http_client.h"
#include "sync/http_client.h"
#include "task/http_client.h"
#include "threaded/http_client.h"

#include "executor.h"
#include "http_client_implementations.h"

#include <boost/algorithm/string.hpp>

#include <gsl/gsl_util>

using namespace testing;
using namespace std::literals;

TEST(Test, sync) {
  using namespace synchronous;
  auto http_client = ConcreteHttpClient{};

  auto result = request_uris(
    http_client, {"/file1", "/file2", "/file3", "/extremeredirect"});

  EXPECT_THAT(result,
              ElementsAre("content1", "content1", "content2", "finally here"));
}

TEST(Test, threaded) {
  using namespace threaded;
  auto http_client = ConcreteHttpClient{};

  auto result = request_uris(
    http_client, {"/file1", "/file2", "/file3", "/extremeredirect"});

  EXPECT_THAT(result,
              ElementsAre("content1", "content1", "content2", "finally here"));
}

TEST(Test, callback) {
  using namespace callback;
  auto executor    = Executor{};
  auto http_client = ConcreteHttpClient{};

  auto result = std::vector<std::string>{};
  executor.add_work([&] {
    request_uris(http_client,
                 {"/file1", "/file2", "/file3", "/extremeredirect"},
                 [&](auto callback_result) { result = callback_result; });
  });
  executor.execute();

  EXPECT_THAT(result,
              ElementsAre("content1", "content1", "content2", "finally here"));
}

TEST(Test, task) {
  using namespace task_based;
  auto executor    = Executor{};
  auto http_client = ConcreteHttpClient{};

  auto result = std::vector<std::string>{};
  executor.add_work([&] {
    request_uris(http_client,
                 {"/file1", "/file2", "/file3", "/extremeredirect"})
      ->then([&](auto callback_result) { //
        result = callback_result;
      });
  });
  executor.execute();

  EXPECT_THAT(result,
              ElementsAre("content1", "content1", "content2", "finally here"));
}

TEST(Test, boost_coroutine_2) {
  using namespace boost_coroutine;
  auto executor    = Executor{};
  auto http_client = ConcreteHttpClient{};

  auto result = std::vector<std::string>{};
  executor.add_work([&] {
    request_uris(http_client,
                 {"/file1", "/file2", "/file3", "/extremeredirect"})
      ->then([&](auto result_from_async) { result = result_from_async; });
  });
  executor.execute();

  EXPECT_THAT(result,
              ElementsAre("content1", "content1", "content2", "finally here"));
}

TEST(Test, boost_coroutine) {
  using namespace boost_coroutine;
  auto executor    = Executor{};
  auto http_client = ConcreteHttpClient{};

  auto result = std::vector<std::string>{};
  executor.add_work([&] {
    request_uris(http_client,
                 {"/file1", "/file2", "/file3", "/extremeredirect"})
      ->then([&](auto result_from_async) { result = result_from_async; });
  });
  executor.execute();

  EXPECT_THAT(result,
              ElementsAre("content1", "content1", "content2", "finally here"));
}

TEST(Test, coroutines_ts) {
  using namespace coroutines_ts;
  auto executor    = Executor{};
  auto http_client = ConcreteHttpClient{};

  auto result = std::vector<std::string>{};
  executor.add_work([&] {
    request_uris(http_client,
                 {"/file1", "/file2", "/file3", "/extremeredirect"})
      ->then([&](auto result_from_async) { result = result_from_async; });
  });
  executor.execute();

  EXPECT_THAT(result,
              ElementsAre("content1", "content1", "content2", "finally here"));
}

TEST(Test, when_all) {
  auto               executor = Executor{};
  std::optional<int> value;
  executor.add_work([&] {
    task<int> first_number  = make_task<int>();
    task<int> second_number = make_task<int>();
    when_all(first_number, second_number)
      ->then([](std::tuple<int, int> values) { //
        return std::get<0>(values) + std::get<1>(values);
      })
      ->then([&](int sum) { //
        value = sum;
      });
    first_number->set_result(1);
    second_number->set_result(2);
  });
  executor.execute();
  ASSERT_THAT(value, Eq(3));
}
