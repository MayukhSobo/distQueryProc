#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <coroutine>
#include <exception>
#include <string>
#include <utility> // For std::pair

struct Generator {
  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;

  Generator(handle_type h) : coro(h) {}
  handle_type coro;

  ~Generator() {
    if (coro)
      coro.destroy();
  }

  struct promise_type {
    std::pair<int, int> value; // Pair for (year, score)
    std::suspend_always yield_value(std::pair<int, int> val) {
      value = val;
      return {};
    }

    Generator get_return_object() {
      return Generator{handle_type::from_promise(*this)};
    }
    std::suspend_always initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() {}
    void unhandled_exception() { std::terminate(); }
  };

  std::pair<int, int> next() {
    coro.resume();
    return coro.done() ? std::make_pair(-1, -1) : coro.promise().value;
  }
};

Generator process_file_coroutine(const std::string &file_name);

void engine_process(int port);

#endif // ENGINE_HPP
