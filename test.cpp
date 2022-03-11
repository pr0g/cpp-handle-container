#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "thh_handles/thh_handles.hpp"

#include <numeric>

TEST_CASE("CanAllocContainer")
{
  thh::container_t<char> container;
  CHECK(true);
}

TEST_CASE("ContainerSizeZeroAfterInit")
{
  thh::container_t<char> container;
  const auto container_size = container.size();
  CHECK(container_size == 0);
}

TEST_CASE("InitialHandleReturnedIsZero")
{
  thh::container_t<char> container;
  thh::handle_t handle = container.add();
  CHECK(handle.id_ == 0);
}

TEST_CASE("ContainerSizeIsOneAfterSingleAdd")
{
  thh::container_t<char> container;
  [[maybe_unused]] thh::handle_t handle = container.add();
  CHECK(container.size() == 1);
}

TEST_CASE("ContainerSizeGrowsWithConsecutiveAdds")
{
  thh::container_t<char> container;
  thh::handle_t handle1 = container.add();
  thh::handle_t handle2 = container.add();
  thh::handle_t handle3 = container.add();

  CHECK(handle1.id_ == 0);
  CHECK(handle2.id_ == 1);
  CHECK(handle3.id_ == 2);
  CHECK(container.size() == 3);
}

TEST_CASE("ContainerHasAddedHandle")
{
  thh::container_t<char> container;
  thh::handle_t handle = container.add();
  CHECK(container.has(handle));
}

TEST_CASE("EmptyContainerDoesNotHaveHandle")
{
  thh::container_t<char> container;
  thh::handle_t handle(0, 0);
  CHECK(!container.has(handle));
}

TEST_CASE("ContainerDoesNotHaveHandleId")
{
  thh::container_t<char> container;
  [[maybe_unused]] thh::handle_t handle = container.add();
  thh::handle_t other_handle(1, 0);
  CHECK(!container.has(other_handle));
}

TEST_CASE("ContainerDoesNotHaveHandleGen")
{
  thh::container_t<char> container;
  [[maybe_unused]] thh::handle_t handle = container.add();
  thh::handle_t other_handle(0, 1);
  CHECK(!container.has(other_handle));
}

TEST_CASE("RemoveDecreasesSize")
{
  thh::container_t<char> container;
  thh::handle_t handle1 = container.add();
  [[maybe_unused]] thh::handle_t handle2 = container.add();
  [[maybe_unused]] thh::handle_t handle3 = container.add();

  CHECK(container.size() == 3);

  const bool removed = container.remove(handle1);

  CHECK(removed);
  CHECK(container.size() == 2);
}

TEST_CASE("HandleReusedAfterRemoval")
{
  thh::container_t<char> container;
  thh::handle_t initial_handle = container.add();
  container.remove(initial_handle);
  thh::handle_t next_handle = container.add();

  CHECK(next_handle.id_ == 0);
  CHECK(next_handle.gen_ == 1);
}

TEST_CASE("CannotRemoveInvalidHandle")
{
  thh::container_t<char> container;
  thh::handle_t invalid_handle(-1, -1);
  const bool removed = container.remove(invalid_handle);
  CHECK(removed == false);
}

TEST_CASE("CanRemoveAddedHandle")
{
  thh::container_t<char> container;
  thh::handle_t handle = container.add();
  bool removed = container.remove(handle);
  bool has = container.has(handle);
  CHECK(removed == true);
  CHECK(has == false);
}

TEST_CASE("MultipleAddsAndRemovesReturnsExpectedSize")
{
  thh::container_t<char> container;
  thh::handle_t handle1 = container.add();
  thh::handle_t handle2 = container.add();
  thh::handle_t handle3 = container.add();
  [[maybe_unused]] thh::handle_t handle4 = container.add();
  [[maybe_unused]] thh::handle_t handle5 = container.add();
  bool removed1 = container.remove(handle1);
  bool removed2 = container.remove(handle2);
  bool removed3 = container.remove(handle3);
  CHECK(removed1);
  CHECK(removed2);
  CHECK(removed3);

  [[maybe_unused]] thh::handle_t handle6 = container.add();
  [[maybe_unused]] thh::handle_t handle7 = container.add();
  [[maybe_unused]] thh::handle_t handle8 = container.add();
  [[maybe_unused]] thh::handle_t handle9 = container.add();
  [[maybe_unused]] thh::handle_t handle10 = container.add();

  CHECK(container.size() == 7);
}

TEST_CASE("AddAndRemoveHandlesReverseOrder")
{
  thh::container_t<char> container;
  constexpr const size_t element_count = 10;
  thh::handle_t handles[element_count];
  for (auto& handle : handles) {
    handle = container.add();
  }

  CHECK(container.size() == element_count);

  bool removed = true;
  for (int32_t i = element_count - 1; i >= 0; i--) {
    removed &= !!container.remove(handles[i]);
  }

  CHECK(removed);
  CHECK(container.size() == 0);

  for (auto& handle : handles) {
    CHECK(!container.has(handle));
  }
}

TEST_CASE("AddAndRemoveHandlesOrdered")
{
  thh::container_t<char> container;
  constexpr const size_t element_count = 10;
  thh::handle_t handles[element_count];
  for (auto& handle : handles) {
    handle = container.add();
  }

  CHECK(container.size() == element_count);

  bool removed = true;
  for (auto& handle : handles) {
    removed &= !!container.remove(handle);
  }

  CHECK(removed);
  CHECK(container.size() == 0);

  for (auto& handle : handles) {
    CHECK(!container.has(handle));
  }
}

TEST_CASE("CanGetElementViaHandle")
{
  thh::container_t<char> container;
  thh::handle_t handle = container.add();
  const char* c = nullptr;
  container.call(handle, [&c](const char& value) { c = &value; });
  CHECK(c != nullptr);
}

TEST_CASE("AddTwoHandlesAndUpdateObjects")
{
  struct test_t
  {
    int a_ = 0;
    int b_ = 0;
  };
  thh::container_t<test_t> container;

  thh::handle_t handle1 = container.add();
  thh::handle_t handle2 = container.add();

  container.call(handle1, [](auto& test1) {
    test1.a_ = 6;
    test1.b_ = 4;
  });
  container.call(handle2, [](auto& test2) {
    test2.a_ = 4;
    test2.b_ = 2;
  });

  container.call(handle1, [](const auto& test1) {
    CHECK(test1.a_ == 6);
    CHECK(test1.b_ == 4);
  });
  container.call(handle2, [](const auto& test2) {
    CHECK(test2.a_ == 4);
    CHECK(test2.b_ == 2);
  });
}

TEST_CASE("OriginalHandleCannotAccessElementAfterRemoval")
{
  thh::container_t<int> container;
  thh::handle_t handle = container.add();
  container.remove(handle);
  CHECK(!container.has(handle));
  CHECK(container.call_return(handle, [](const auto&) {
    return true;
  }) == std::nullopt);
}

TEST_CASE("ElementsRemainPackedAfterRemoval")
{
  thh::container_t<float> container;
  thh::handle_t handles[5];
  for (auto& handle : handles) {
    handle = container.add();
  }
  container.remove(handles[2]);

  const float* begin = nullptr;
  container.call(handles[0], [&begin](const auto& value) { begin = &value; });
  const float* was_end = nullptr;
  container.call(
    handles[4], [&was_end](const auto& value) { was_end = &value; });
  const float* new_end = nullptr;
  container.call(
    handles[3], [&new_end](const auto& value) { new_end = &value; });

  CHECK(was_end - begin == 2);
  CHECK(new_end - begin == 3);
}

TEST_CASE("ContainerDebugVisualization")
{
  thh::container_t<float> container;
  const size_t handle_count = 5;
  thh::handle_t handles[handle_count];
  container.reserve(handle_count);
  for (auto& handle : handles) {
    handle = container.add();
  }

  container.remove(handles[2]);
  container.remove(handles[0]);

  const std::string buffer = container.debug_handles();
  const std::string expected_buffer = "[x][o][x][o][o]";
  CHECK(expected_buffer == buffer);
}

TEST_CASE("EnsureHandlesReaddedInOrder")
{
  thh::container_t<float> container;
  const size_t handle_count = 5;
  thh::handle_t handles[handle_count];
  container.reserve(handle_count);
  for (auto& handle : handles) {
    handle = container.add();
  }

  std::string expected_buffer;
  for (int32_t i = 0; i < container.capacity(); i++) {
    expected_buffer.append("[x]");
  }

  for (auto& handle : handles) {
    container.remove(handle);
  }

  std::string buffer = container.debug_handles();
  CHECK(expected_buffer == buffer);

  thh::handle_t first_new_handle = container.add();
  buffer = container.debug_handles();
  expected_buffer = "[x][x][x][x][o]";
  CHECK(expected_buffer == buffer);

  thh::handle_t second_new_handle = container.add();
  buffer = container.debug_handles();
  expected_buffer = "[x][x][x][o][o]";
  CHECK(expected_buffer == buffer);

  const float* begin = nullptr;
  container.call(
    first_new_handle, [&begin](const auto& value) { begin = &value; });
  const float* end = nullptr;
  container.call(
    second_new_handle, [&end](const auto& value) { end = &value; });

  // ensure objects are tightly packed
  ptrdiff_t size = end - begin;
  CHECK(size == 1);
}

TEST_CASE("EnsureResourceCleanedUpAfterRemoval")
{
  struct resource_t
  {
    resource_t() = default;
    resource_t(const resource_t&) = default;
    resource_t& operator=(const resource_t&) = default;
    ~resource_t() { *resource_ = 42; }
    int* resource_ = nullptr;
  };

  thh::container_t<resource_t> container;
  const auto resource_handle = container.add();

  int value = 100;
  container.call(
    resource_handle, [&value](auto& resource) { resource.resource_ = &value; });

  container.remove(resource_handle);

  CHECK(value == 42);
}

TEST_CASE("EnumerateMutableElements")
{
  struct entity_t
  {
    int x_ = 0;
    int y_ = 0;
  };

  const auto entity_handle_count = 10;
  thh::container_t<entity_t> entities;
  std::vector<thh::handle_t> entity_handles;
  for (size_t i = 0; i < entity_handle_count; ++i) {
    entity_handles.push_back(entities.add());
  }

  entities.enumerate([](entity_t& entity) {
    entity.x_ += 1;
    entity.y_ += 2;
  });

  CHECK(entity_handles.size() == entity_handle_count);
  for (const auto& entity_handle : entity_handles) {
    entities.call(entity_handle, [](const entity_t& entity) {
      CHECK(entity.x_ == 1);
      CHECK(entity.y_ == 2);
    });
  }
}

TEST_CASE("EnumerateImmutableElements")
{
  struct entity_t
  {
    int w_ = 2;
    int h_ = 1;
  };

  thh::container_t<entity_t> entities;
  std::vector<thh::handle_t> entity_handles;
  for (size_t i = 0; i < 10; ++i) {
    entity_handles.push_back(entities.add());
  }

  int total_width = 0;
  int total_height = 0;
  entities.enumerate(
    [&total_width, &total_height](const entity_t& entity) mutable {
      total_width += entity.w_;
      total_height += entity.h_;
    });

  CHECK(total_height == 10);
  CHECK(total_width == 20);
}

TEST_CASE("HandleResolvesAfterInternalMove")
{
  thh::container_t<int> container;
  thh::handle_t handles[5];
  for (auto& handle : handles) {
    handle = container.add();
  }
  for (size_t i = 0; i < std::size(handles); ++i) {
    container.call(
      handles[i], [i](int& value) { value = static_cast<int32_t>(i) + 1; });
  }

  void* address_before =
    container.call_return(handles[0], [](int& value) { return &value; })
      .value();

  container.remove(handles[0]);
  const auto* last =
    container.call_return(handles[4], [](int& value) { return &value; })
      .value();

  CHECK(*last == 5);
  CHECK(last == address_before);
}

TEST_CASE("ElementsCanBeReserved")
{
  thh::container_t<int> container;
  container.reserve(10);
  CHECK(container.size() == 0);
  CHECK(container.capacity() == 10);
}

TEST_CASE("ElementsCanBeReservedAfterFirstUse")
{
  thh::container_t<int> container;
  thh::handle_t handles[5];
  for (auto& handle : handles) {
    handle = container.add();
  }

  container.reserve(10);

  for (auto& handle : handles) {
    CHECK(container.has(handle));
  }

  CHECK(container.size() == 5);
  CHECK(container.capacity() == 10);
}

TEST_CASE("ContainerCanBeCleared")
{
  thh::container_t<int> container;
  thh::handle_t handles[10];
  for (auto& handle : handles) {
    handle = container.add();
  }

  container.clear();

  CHECK(container.size() == 0);
  for (auto& handle : handles) {
    CHECK(!container.has(handle));
  }
}

TEST_CASE("FirstHandleReturnedAfterClear")
{
  thh::container_t<int> container;
  thh::handle_t handles[10];
  for (auto& handle : handles) {
    handle = container.add();
  }

  container.clear();

  thh::handle_t next_handle = container.add();
  CHECK(next_handle.id_ == 0);
  CHECK(next_handle.gen_ == 1);
}

TEST_CASE("FirstElementReturnedAfterClear")
{
  thh::container_t<int> container;
  thh::handle_t handles[10];
  for (auto& handle : handles) {
    handle = container.add();
  }

  const void* begin = nullptr;
  container.call(handles[0], [&begin](const auto& value) { begin = &value; });

  container.clear();

  thh::handle_t next_handle = container.add();

  const void* element = nullptr;
  container.call(
    next_handle, [&element](const auto& value) { element = &value; });

  CHECK(begin == element);
}

TEST_CASE("ContainerGrowsCorrectlyAfterClear")
{
  thh::container_t<int> container;
  const size_t initial_handle_count = 10;
  std::vector<thh::handle_t> handles;
  for (size_t i = 0; i < initial_handle_count; ++i) {
    handles.push_back(container.add());
  }

  const auto capacity_before_clear = container.capacity();

  const auto difference_required_for_grow =
    capacity_before_clear - container.size();
  const auto grow_size = handles.size() + difference_required_for_grow;

  container.clear();

  for (auto& handle : handles) {
    CHECK(!container.has(handle));
  }

  handles.clear();

  for (size_t i = 0; i < grow_size + 1; ++i) {
    handles.push_back(container.add());
  }

  const thh::handle_t another_handle = container.add();
  const auto next_id = grow_size + 1;
  CHECK(another_handle.id_ == next_id);
  CHECK(another_handle.gen_ == 0);

  container.remove(handles[5]);

  const thh::handle_t next_handle = container.add();
  CHECK(next_handle.id_ == 5);
  CHECK(next_handle.gen_ == 2);
}

TEST_CASE("HoldMoveOnlyType")
{
  struct resource_t
  {
    resource_t() {} // user provided constructor
    resource_t(resource_t&&) = default;
    resource_t& operator=(resource_t&&) = default;
  };

  thh::container_t<resource_t> container;
  std::vector<thh::handle_t> handles;
  const size_t initial_handle_count = 10;
  for (size_t i = 0; i < initial_handle_count; ++i) {
    handles.push_back(container.add());
  }

  for (auto& handle : handles) {
    container.remove(handle);
  };

  CHECK(container.size() == 0);
}

TEST_CASE("TaggedHandle")
{
  thh::container_t<float> float_container;
  thh::container_t<float, struct height_tag_t> height_container;
  thh::container_t<float, struct width_tag_t> width_container;

  using width_handle_t = thh::typed_handle_t<struct width_tag_t>;
  using height_handle_t = thh::typed_handle_t<struct height_tag_t>;

  // thh::typed_handle_t<struct width_tag_t>
  [[maybe_unused]] const width_handle_t width_handle = width_container.add();
  // thh::typed_handle_t<struct height_tag_t>
  [[maybe_unused]] const height_handle_t height_handle = height_container.add();
  [[maybe_unused]] const thh::handle_t float_handle = float_container.add();

  // note - lines do not compile (type mismatch error)
  // width_container.call(height_handle, [](const auto&) {});
  // width_container.call(float_handle, [](const auto&) {});

  const float* width = nullptr;
  width_container.call(
    width_handle, [&width](const auto& value) { width = &value; });
  CHECK(width != nullptr);
}

TEST_CASE("SupportNonDefaultConstructibleType")
{
  struct no_default_constructor_t
  {
    explicit no_default_constructor_t(int i) : i_(i) {}
    int i_ = 0;
  };

  thh::container_t<no_default_constructor_t> container;
  const auto handle = container.add(4);

  container.call(handle, [](const auto& value) { CHECK(value.i_ == 4); });

  container.remove(handle);
  CHECK(container.call_return(handle, [](const auto&) {
    return true;
  }) == std::nullopt);
}

TEST_CASE("ValuesAccessedThroughIterators")
{
  thh::container_t<int> container;
  constexpr const size_t element_count = 10;
  thh::handle_t handles[element_count];
  for (auto& handle : handles) {
    handle = container.add();
  }

  std::for_each(
    container.begin(), container.end(),
    [i = 0](auto& elem) mutable { elem = i++; });

  for (size_t i = 0; i < element_count; ++i) {
    container.call(handles[i], [i](const auto& value) { CHECK(value == i); });
  }
}

TEST_CASE("AccumulateWithIterators")
{
  thh::container_t<int> container;
  constexpr const size_t element_count = 10;
  thh::handle_t handles[element_count];
  for (auto& handle : handles) {
    const auto h = container.add();
    container.call(h, [](auto& value) { value = 5; });
    handle = h;
  }

  const auto total = std::accumulate(
    container.begin(), container.end(), 0, [](int acc, const auto& value) {
      acc += value;
      return acc;
    });

  CHECK(total == 50);
}

TEST_CASE("FindWithIterators")
{
  thh::container_t<int> container;
  constexpr const size_t element_count = 10;
  thh::handle_t handles[element_count];

  for (size_t i = 0; i < element_count; ++i) {
    const auto h = container.add();
    container.call(h, [i](auto& value) { value = static_cast<int>(i); });
    handles[i] = h;
  }

  const auto found =
    std::find_if(container.cbegin(), container.cend(), [](const int value) {
      return value == 8;
    });

  CHECK(found != container.cend());
  CHECK(*found == 8);
}

TEST_CASE("HandleEqualityCheckPasses")
{
  thh::handle_t handle_a{0, 1};
  thh::handle_t handle_b{0, 1};
  CHECK(handle_a == handle_b);
}

TEST_CASE("HandleEqualityCheckFails")
{
  {
    thh::handle_t handle_a{1, 0};
    thh::handle_t handle_b{0, 0};
    CHECK(handle_a != handle_b);
  }

  {
    thh::handle_t handle_a{2, 1};
    thh::handle_t handle_b{2, 0};
    CHECK(handle_a != handle_b);
  }
}

TEST_CASE("InvokeCall")
{
  struct incrementer_t
  {
    int pre_increment() { return ++counter_; }
    int counter() const { return counter_; }

  private:
    int counter_ = 0;
  };

  thh::container_t<incrementer_t> container;
  const thh::handle_t handle = container.add();

  int result = 0;
  container.call(handle, [&result](incrementer_t& incrementer) {
    result = incrementer.pre_increment();
  });

  CHECK(result == 1);

  int next_result = 0;
  const auto& container_ref = container;
  container_ref.call(handle, [&next_result](const incrementer_t& incrementer) {
    next_result = incrementer.counter();
  });

  CHECK(next_result == 1);
}

TEST_CASE("InvokeCallReturn")
{
  class Test
  {
  private:
    Test() = default;
    int v_ = 5;

  public:
    static Test make_test() { return Test(); }
    int v() const { return v_; }
    void inc() { v_++; }
  };

  thh::container_t<Test> container;
  thh::handle_t handle = container.add(Test::make_test());

  auto result = container.call_return(handle, [](Test value) { return value; });

  CHECK(result.value().v() == 5);

  auto next_result = container.call_return(handle, [](Test& value) {
    value.inc();
    return value;
  });

  CHECK(next_result.value().v() == 6);

  const auto& container_ref = container;
  auto last_result =
    container_ref.call_return(handle, [](const Test& value) { return value; });

  CHECK(last_result.value().v() == 6);
}

TEST_CASE("InvokeCallReturnFails")
{
  thh::container_t<int> container;
  thh::handle_t handle = container.add(10);

  auto result = container.call_return(handle, [](int value) { return value; });
  CHECK(result.value() == 10);

  // called with invalid handle
  auto next_result =
    container.call_return(thh::handle_t{}, [](int value) { return value; });
  CHECK(!next_result.has_value());

  const auto& container_ref = container;
  auto last_result = container_ref.call_return(
    thh::handle_t{}, [](const int& value) { return value; });

  CHECK(!last_result.has_value());
}

TEST_CASE("DefaultContainerIsEmpty")
{
  thh::container_t<int> container;
  CHECK(container.empty());
}

TEST_CASE("ContainerIsNotEmptyAfterAdd")
{
  thh::container_t<int> container;
  [[maybe_unused]] thh::handle_t handle = container.add(10);

  CHECK(!container.empty());
}

TEST_CASE("ContainerIsEmptyAfterRemove")
{
  thh::container_t<int> container;
  thh::handle_t handle = container.add(10);
  container.remove(handle);

  CHECK(container.empty());
}

TEST_CASE("ConstContainerEmptyCheck")
{
  thh::container_t<int> container;
  container.add();

  container.clear();

  const thh::container_t<int>& const_container = container;
  CHECK(const_container.empty());
}
