#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "01_sync/http_client.h"
#include "02_threaded/http_client.h"
#include "03_callback/http_client.h"
#include "04_task/http_client.h"
#include "05_boost_coroutine/http_client.h"
#include "06_coroutine_ts/http_client.h"

#include "executor.h"
#include "http_client_implementations.h"

#include <boost/algorithm/string.hpp>

#include <gsl/gsl_util>

using namespace testing;
using namespace std::literals;

TEST(DISABLED_Test, _01_sync) {
  using namespace synchronous;
  auto http_client = ConcreteHttpClient{};

  auto result = request_uris(
    http_client, {"/file1", "/file2", "/file3", "/extremeredirect"});

  EXPECT_THAT(result,
              ElementsAre("content1", "content1", "content2", "finally here"));
}

TEST(DISABLED_Test, _02_threaded) {
  using namespace threaded;
  auto http_client = ConcreteHttpClient{};

  auto result = request_uris(
    http_client, {"/file1", "/file2", "/file3", "/extremeredirect"});

  EXPECT_THAT(result,
              ElementsAre("content1", "content1", "content2", "finally here"));
}

TEST(DISABLED_Test, _03_callback) {
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

TEST(DISABLED_Test, _04_task) {
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

TEST(DISABLED_Test, _05_boost_coroutine) {
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

TEST(DISABLED_Test, _06_coroutines_ts) {
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
