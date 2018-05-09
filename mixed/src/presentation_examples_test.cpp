#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "presentation_examples_test.h"
using namespace testing;

//
//
//

TEST(SimplifiedExamples, presentation_examples) {
  {
    using namespace synchronous;

    auto http_client = ConcreteHttpClient{};
    example(http_client); // blocks
  }

  {
    using namespace threaded;

    auto http_client = ConcreteHttpClient{};
    example_thread(http_client); // blocks
  }

  {
    using namespace threaded;

    auto http_client = ConcreteHttpClient{};
    example_async(http_client); // blocks
  }

  {
    using namespace threaded;

    auto              http_client = ConcreteHttpClient{};
    std::future<void> task        = example_async2(http_client);
    task.get(); // blocks
  }

  {
    using namespace callback;

    auto http_client = ConcreteHttpClient{};
    auto executor    = Executor{};
    executor.add_work([&] { example(http_client, [] { /*open end*/ }); });
    executor.execute(); // blocks
  }

  {
    using namespace task_based;

    auto http_client = ConcreteHttpClient{};
    auto executor    = Executor{};
    executor.add_work([&] { example(http_client); });
    executor.execute(); // blocks
  }

  {
    using namespace task_based;

    auto http_client = ConcreteHttpClient{};
    auto executor    = Executor{};
    executor.add_work([&] { example2(http_client); });
    executor.execute(); // blocks
  }

  {
    using namespace boost_coroutine;

    auto http_client = ConcreteHttpClient{};
    auto executor    = Executor{};
    executor.add_work([&] { example(http_client); });
    executor.execute(); // blocks
  }

  {
    using namespace coroutines_ts;

    auto http_client = ConcreteHttpClient{};
    auto executor    = Executor{};
    executor.add_work([&] { example_regular(http_client); });
    executor.execute(); // blocks
  }

  {
    using namespace coroutines_ts;

    auto http_client = ConcreteHttpClient{};
    auto executor    = Executor{};
    executor.add_work([&] { example(http_client); });
    executor.execute(); // blocks
  }
}
