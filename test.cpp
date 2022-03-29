#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "thh_handles/thh_handles.hpp"

#include <numeric>

TEST_CASE("CanAllocContainer")
{
  thh::handle_vector_t<char> handle_vector;
  CHECK(true);
}

TEST_CASE("ContainerSizeZeroAfterInit")
{
  thh::handle_vector_t<char> handle_vector;
  const auto handle_vector_size = handle_vector.size();
  CHECK(handle_vector_size == 0);
}

TEST_CASE("InitialHandleReturnedIsZero")
{
  thh::handle_vector_t<char> handle_vector;
  thh::handle_t handle = handle_vector.add();
  CHECK(handle.id_ == 0);
}

TEST_CASE("ContainerSizeIsOneAfterSingleAdd")
{
  thh::handle_vector_t<char> handle_vector;
  [[maybe_unused]] thh::handle_t handle = handle_vector.add();
  CHECK(handle_vector.size() == 1);
}

TEST_CASE("ContainerSizeGrowsWithConsecutiveAdds")
{
  thh::handle_vector_t<char> handle_vector;
  thh::handle_t handle1 = handle_vector.add();
  thh::handle_t handle2 = handle_vector.add();
  thh::handle_t handle3 = handle_vector.add();

  CHECK(handle1.id_ == 0);
  CHECK(handle2.id_ == 1);
  CHECK(handle3.id_ == 2);
  CHECK(handle_vector.size() == 3);
}

TEST_CASE("ContainerHasAddedHandle")
{
  thh::handle_vector_t<char> handle_vector;
  thh::handle_t handle = handle_vector.add();
  CHECK(handle_vector.has(handle));
}

TEST_CASE("EmptyContainerDoesNotHaveHandle")
{
  thh::handle_vector_t<char> handle_vector;
  thh::handle_t handle(0, 0);
  CHECK(!handle_vector.has(handle));
}

TEST_CASE("ContainerDoesNotHaveHandleId")
{
  thh::handle_vector_t<char> handle_vector;
  [[maybe_unused]] thh::handle_t handle = handle_vector.add();
  thh::handle_t other_handle(1, 0);
  CHECK(!handle_vector.has(other_handle));
}

TEST_CASE("ContainerDoesNotHaveHandleGen")
{
  thh::handle_vector_t<char> handle_vector;
  [[maybe_unused]] thh::handle_t handle = handle_vector.add();
  thh::handle_t other_handle(0, 1);
  CHECK(!handle_vector.has(other_handle));
}

TEST_CASE("RemoveDecreasesSize")
{
  thh::handle_vector_t<char> handle_vector;
  thh::handle_t handle1 = handle_vector.add();
  [[maybe_unused]] thh::handle_t handle2 = handle_vector.add();
  [[maybe_unused]] thh::handle_t handle3 = handle_vector.add();

  CHECK(handle_vector.size() == 3);

  const bool removed = handle_vector.remove(handle1);

  CHECK(removed);
  CHECK(handle_vector.size() == 2);
}

TEST_CASE("HandleReusedAfterRemoval")
{
  thh::handle_vector_t<char> handle_vector;
  thh::handle_t initial_handle = handle_vector.add();
  handle_vector.remove(initial_handle);
  thh::handle_t next_handle = handle_vector.add();

  CHECK(next_handle.id_ == 0);
  CHECK(next_handle.gen_ == 1);
}

TEST_CASE("CannotRemoveInvalidHandle")
{
  thh::handle_vector_t<char> handle_vector;
  thh::handle_t invalid_handle(-1, -1);
  const bool removed = handle_vector.remove(invalid_handle);
  CHECK(removed == false);
}

TEST_CASE("CanRemoveAddedHandle")
{
  thh::handle_vector_t<char> handle_vector;
  thh::handle_t handle = handle_vector.add();
  bool removed = handle_vector.remove(handle);
  bool has = handle_vector.has(handle);
  CHECK(removed == true);
  CHECK(has == false);
}

TEST_CASE("MultipleAddsAndRemovesReturnsExpectedSize")
{
  thh::handle_vector_t<char> handle_vector;
  thh::handle_t handle1 = handle_vector.add();
  thh::handle_t handle2 = handle_vector.add();
  thh::handle_t handle3 = handle_vector.add();
  [[maybe_unused]] thh::handle_t handle4 = handle_vector.add();
  [[maybe_unused]] thh::handle_t handle5 = handle_vector.add();
  bool removed1 = handle_vector.remove(handle1);
  bool removed2 = handle_vector.remove(handle2);
  bool removed3 = handle_vector.remove(handle3);
  CHECK(removed1);
  CHECK(removed2);
  CHECK(removed3);

  [[maybe_unused]] thh::handle_t handle6 = handle_vector.add();
  [[maybe_unused]] thh::handle_t handle7 = handle_vector.add();
  [[maybe_unused]] thh::handle_t handle8 = handle_vector.add();
  [[maybe_unused]] thh::handle_t handle9 = handle_vector.add();
  [[maybe_unused]] thh::handle_t handle10 = handle_vector.add();

  CHECK(handle_vector.size() == 7);
}

TEST_CASE("AddAndRemoveHandlesReverseOrder")
{
  thh::handle_vector_t<char> handle_vector;
  constexpr const size_t element_count = 10;
  thh::handle_t handles[element_count];
  for (auto& handle : handles) {
    handle = handle_vector.add();
  }

  CHECK(handle_vector.size() == element_count);

  bool removed = true;
  for (int32_t i = element_count - 1; i >= 0; i--) {
    removed &= !!handle_vector.remove(handles[i]);
  }

  CHECK(removed);
  CHECK(handle_vector.size() == 0);

  for (auto& handle : handles) {
    CHECK(!handle_vector.has(handle));
  }
}

TEST_CASE("AddAndRemoveHandlesOrdered")
{
  thh::handle_vector_t<char> handle_vector;
  constexpr const size_t element_count = 10;
  thh::handle_t handles[element_count];
  for (auto& handle : handles) {
    handle = handle_vector.add();
  }

  CHECK(handle_vector.size() == element_count);

  bool removed = true;
  for (auto& handle : handles) {
    removed &= !!handle_vector.remove(handle);
  }

  CHECK(removed);
  CHECK(handle_vector.size() == 0);

  for (auto& handle : handles) {
    CHECK(!handle_vector.has(handle));
  }
}

TEST_CASE("CanGetElementViaHandle")
{
  thh::handle_vector_t<char> handle_vector;
  thh::handle_t handle = handle_vector.add();
  const char* c = nullptr;
  handle_vector.call(handle, [&c](const char& value) { c = &value; });
  CHECK(c != nullptr);
}

TEST_CASE("AddTwoHandlesAndUpdateObjects")
{
  struct test_t
  {
    int a_ = 0;
    int b_ = 0;
  };
  thh::handle_vector_t<test_t> handle_vector;

  thh::handle_t handle1 = handle_vector.add();
  thh::handle_t handle2 = handle_vector.add();

  handle_vector.call(handle1, [](auto& test1) {
    test1.a_ = 6;
    test1.b_ = 4;
  });
  handle_vector.call(handle2, [](auto& test2) {
    test2.a_ = 4;
    test2.b_ = 2;
  });

  handle_vector.call(handle1, [](const auto& test1) {
    CHECK(test1.a_ == 6);
    CHECK(test1.b_ == 4);
  });
  handle_vector.call(handle2, [](const auto& test2) {
    CHECK(test2.a_ == 4);
    CHECK(test2.b_ == 2);
  });
}

TEST_CASE("OriginalHandleCannotAccessElementAfterRemoval")
{
  thh::handle_vector_t<int> handle_vector;
  thh::handle_t handle = handle_vector.add();
  handle_vector.remove(handle);
  CHECK(!handle_vector.has(handle));
  CHECK(handle_vector.call_return(handle, [](const auto&) {
    return true;
  }) == std::nullopt);
}

TEST_CASE("ElementsRemainPackedAfterRemoval")
{
  thh::handle_vector_t<float> handle_vector;
  thh::handle_t handles[5];
  for (auto& handle : handles) {
    handle = handle_vector.add();
  }
  handle_vector.remove(handles[2]);

  const float* begin = nullptr;
  handle_vector.call(
    handles[0], [&begin](const auto& value) { begin = &value; });
  const float* was_end = nullptr;
  handle_vector.call(
    handles[4], [&was_end](const auto& value) { was_end = &value; });
  const float* new_end = nullptr;
  handle_vector.call(
    handles[3], [&new_end](const auto& value) { new_end = &value; });

  CHECK(was_end - begin == 2);
  CHECK(new_end - begin == 3);
}

TEST_CASE("ContainerDebugVisualization")
{
  thh::handle_vector_t<float> handle_vector;
  const size_t handle_count = 5;
  thh::handle_t handles[handle_count];
  handle_vector.reserve(handle_count);
  for (auto& handle : handles) {
    handle = handle_vector.add();
  }

  handle_vector.remove(handles[2]);
  handle_vector.remove(handles[0]);

  const std::string buffer = handle_vector.debug_handles();
  const std::string expected_buffer = "[x][o][x][o][o]";
  CHECK(expected_buffer == buffer);
}

TEST_CASE("EnsureHandlesReaddedInOrder")
{
  thh::handle_vector_t<float> handle_vector;
  const size_t handle_count = 5;
  thh::handle_t handles[handle_count];
  handle_vector.reserve(handle_count);
  for (auto& handle : handles) {
    handle = handle_vector.add();
  }

  std::string expected_buffer;
  for (int32_t i = 0; i < handle_vector.capacity(); i++) {
    expected_buffer.append("[x]");
  }

  for (auto& handle : handles) {
    handle_vector.remove(handle);
  }

  std::string buffer = handle_vector.debug_handles();
  CHECK(expected_buffer == buffer);

  thh::handle_t first_new_handle = handle_vector.add();
  buffer = handle_vector.debug_handles();
  expected_buffer = "[x][x][x][x][o]";
  CHECK(expected_buffer == buffer);

  thh::handle_t second_new_handle = handle_vector.add();
  buffer = handle_vector.debug_handles();
  expected_buffer = "[x][x][x][o][o]";
  CHECK(expected_buffer == buffer);

  const float* begin = nullptr;
  handle_vector.call(
    first_new_handle, [&begin](const auto& value) { begin = &value; });
  const float* end = nullptr;
  handle_vector.call(
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

  thh::handle_vector_t<resource_t> handle_vector;
  const auto resource_handle = handle_vector.add();

  int value = 100;
  handle_vector.call(
    resource_handle, [&value](auto& resource) { resource.resource_ = &value; });

  handle_vector.remove(resource_handle);

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
  thh::handle_vector_t<entity_t> entities;
  std::vector<thh::handle_t> entity_handles;
  for (size_t i = 0; i < entity_handle_count; ++i) {
    entity_handles.push_back(entities.add());
  }

  for (entity_t& entity : entities) {
    entity.x_ += 1;
    entity.y_ += 2;
  }

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

  thh::handle_vector_t<entity_t> entities;
  std::vector<thh::handle_t> entity_handles;
  for (size_t i = 0; i < 10; ++i) {
    entity_handles.push_back(entities.add());
  }

  int total_width = 0;
  int total_height = 0;
  for (const entity_t& entity : entities) {
    total_width += entity.w_;
    total_height += entity.h_;
  }

  CHECK(total_height == 10);
  CHECK(total_width == 20);
}

TEST_CASE("HandleResolvesAfterInternalMove")
{
  thh::handle_vector_t<int> handle_vector;
  thh::handle_t handles[5];
  for (auto& handle : handles) {
    handle = handle_vector.add();
  }
  for (size_t i = 0; i < std::size(handles); ++i) {
    handle_vector.call(
      handles[i], [i](int& value) { value = static_cast<int32_t>(i) + 1; });
  }

  void* address_before =
    handle_vector.call_return(handles[0], [](int& value) { return &value; })
      .value();

  handle_vector.remove(handles[0]);
  const auto* last =
    handle_vector.call_return(handles[4], [](int& value) { return &value; })
      .value();

  CHECK(*last == 5);
  CHECK(last == address_before);
}

TEST_CASE("ElementsCanBeReserved")
{
  thh::handle_vector_t<int> handle_vector;
  handle_vector.reserve(10);
  CHECK(handle_vector.size() == 0);
  CHECK(handle_vector.capacity() == 10);
}

TEST_CASE("ElementsCanBeReservedAfterFirstUse")
{
  thh::handle_vector_t<int> handle_vector;
  thh::handle_t handles[5];
  for (auto& handle : handles) {
    handle = handle_vector.add();
  }

  handle_vector.reserve(10);

  for (auto& handle : handles) {
    CHECK(handle_vector.has(handle));
  }

  CHECK(handle_vector.size() == 5);
  CHECK(handle_vector.capacity() == 10);
}

TEST_CASE("ContainerCanBeCleared")
{
  thh::handle_vector_t<int> handle_vector;
  thh::handle_t handles[10];
  for (auto& handle : handles) {
    handle = handle_vector.add();
  }

  handle_vector.clear();

  CHECK(handle_vector.size() == 0);
  for (auto& handle : handles) {
    CHECK(!handle_vector.has(handle));
  }
}

TEST_CASE("FirstHandleReturnedAfterClear")
{
  thh::handle_vector_t<int> handle_vector;
  thh::handle_t handles[10];
  for (auto& handle : handles) {
    handle = handle_vector.add();
  }

  handle_vector.clear();

  thh::handle_t next_handle = handle_vector.add();
  CHECK(next_handle.id_ == 0);
  CHECK(next_handle.gen_ == 1);
}

TEST_CASE("FirstElementReturnedAfterClear")
{
  thh::handle_vector_t<int> handle_vector;
  thh::handle_t handles[10];
  for (auto& handle : handles) {
    handle = handle_vector.add();
  }

  const void* begin = nullptr;
  handle_vector.call(
    handles[0], [&begin](const auto& value) { begin = &value; });

  handle_vector.clear();

  thh::handle_t next_handle = handle_vector.add();

  const void* element = nullptr;
  handle_vector.call(
    next_handle, [&element](const auto& value) { element = &value; });

  CHECK(begin == element);
}

TEST_CASE("ContainerGrowsCorrectlyAfterClear")
{
  thh::handle_vector_t<int> handle_vector;
  const size_t initial_handle_count = 10;
  std::vector<thh::handle_t> handles;
  for (size_t i = 0; i < initial_handle_count; ++i) {
    handles.push_back(handle_vector.add());
  }

  const auto capacity_before_clear = handle_vector.capacity();

  const auto difference_required_for_grow =
    capacity_before_clear - handle_vector.size();
  const auto grow_size = handles.size() + difference_required_for_grow;

  handle_vector.clear();

  for (auto& handle : handles) {
    CHECK(!handle_vector.has(handle));
  }

  handles.clear();

  for (size_t i = 0; i < grow_size + 1; ++i) {
    handles.push_back(handle_vector.add());
  }

  const thh::handle_t another_handle = handle_vector.add();
  const auto next_id = grow_size + 1;
  CHECK(another_handle.id_ == next_id);
  CHECK(another_handle.gen_ == 0);

  handle_vector.remove(handles[5]);

  const thh::handle_t next_handle = handle_vector.add();
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

  thh::handle_vector_t<resource_t> handle_vector;
  std::vector<thh::handle_t> handles;
  const size_t initial_handle_count = 10;
  for (size_t i = 0; i < initial_handle_count; ++i) {
    handles.push_back(handle_vector.add());
  }

  for (auto& handle : handles) {
    handle_vector.remove(handle);
  };

  CHECK(handle_vector.size() == 0);
}

TEST_CASE("TaggedHandle")
{
  thh::handle_vector_t<float> float_handle_vector;
  thh::handle_vector_t<float, struct height_tag_t> height_handle_vector;
  thh::handle_vector_t<float, struct width_tag_t> width_handle_vector;

  using width_handle_t = thh::typed_handle_t<struct width_tag_t>;
  using height_handle_t = thh::typed_handle_t<struct height_tag_t>;

  // thh::typed_handle_t<struct width_tag_t>
  [[maybe_unused]] const width_handle_t width_handle =
    width_handle_vector.add();
  // thh::typed_handle_t<struct height_tag_t>
  [[maybe_unused]] const height_handle_t height_handle =
    height_handle_vector.add();
  [[maybe_unused]] const thh::handle_t float_handle = float_handle_vector.add();

  // note - lines do not compile (type mismatch error)
  // width_handle_vector.call(height_handle, [](const auto&) {});
  // width_handle_vector.call(float_handle, [](const auto&) {});

  const float* width = nullptr;
  width_handle_vector.call(
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

  thh::handle_vector_t<no_default_constructor_t> handle_vector;
  const auto handle = handle_vector.add(4);

  handle_vector.call(handle, [](const auto& value) { CHECK(value.i_ == 4); });

  handle_vector.remove(handle);
  CHECK(handle_vector.call_return(handle, [](const auto&) {
    return true;
  }) == std::nullopt);
}

TEST_CASE("ValuesAccessedThroughIterators")
{
  thh::handle_vector_t<int> handle_vector;
  constexpr const size_t element_count = 10;
  thh::handle_t handles[element_count];
  for (auto& handle : handles) {
    handle = handle_vector.add();
  }

  std::for_each(
    handle_vector.begin(), handle_vector.end(),
    [i = 0](auto& elem) mutable { elem = i++; });

  for (size_t i = 0; i < element_count; ++i) {
    handle_vector.call(
      handles[i], [i](const auto& value) { CHECK(value == i); });
  }
}

TEST_CASE("AccumulateWithIterators")
{
  thh::handle_vector_t<int> handle_vector;
  constexpr const size_t element_count = 10;
  thh::handle_t handles[element_count];
  for (auto& handle : handles) {
    const auto h = handle_vector.add();
    handle_vector.call(h, [](auto& value) { value = 5; });
    handle = h;
  }

  const auto total = std::accumulate(
    handle_vector.begin(), handle_vector.end(), 0,
    [](int acc, const auto& value) {
      acc += value;
      return acc;
    });

  CHECK(total == 50);
}

TEST_CASE("FindWithIterators")
{
  thh::handle_vector_t<int> handle_vector;
  constexpr const size_t element_count = 10;
  thh::handle_t handles[element_count];

  for (size_t i = 0; i < element_count; ++i) {
    const auto h = handle_vector.add();
    handle_vector.call(h, [i](auto& value) { value = static_cast<int>(i); });
    handles[i] = h;
  }

  const auto found = std::find_if(
    handle_vector.cbegin(), handle_vector.cend(),
    [](const int value) { return value == 8; });

  CHECK(found != handle_vector.cend());
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

  thh::handle_vector_t<incrementer_t> handle_vector;
  const thh::handle_t handle = handle_vector.add();

  int result = 0;
  handle_vector.call(handle, [&result](incrementer_t& incrementer) {
    result = incrementer.pre_increment();
  });

  CHECK(result == 1);

  int next_result = 0;
  const auto& handle_vector_ref = handle_vector;
  handle_vector_ref.call(
    handle, [&next_result](const incrementer_t& incrementer) {
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

  thh::handle_vector_t<Test> handle_vector;
  thh::handle_t handle = handle_vector.add(Test::make_test());

  auto result =
    handle_vector.call_return(handle, [](Test value) { return value; });

  CHECK(result.value().v() == 5);

  auto next_result = handle_vector.call_return(handle, [](Test& value) {
    value.inc();
    return value;
  });

  CHECK(next_result.value().v() == 6);

  const auto& handle_vector_ref = handle_vector;
  auto last_result = handle_vector_ref.call_return(
    handle, [](const Test& value) { return value; });

  CHECK(last_result.value().v() == 6);
}

TEST_CASE("InvokeCallReturnFails")
{
  thh::handle_vector_t<int> handle_vector;
  thh::handle_t handle = handle_vector.add(10);

  auto result =
    handle_vector.call_return(handle, [](int value) { return value; });
  CHECK(result.value() == 10);

  // called with invalid handle
  auto next_result =
    handle_vector.call_return(thh::handle_t{}, [](int value) { return value; });
  CHECK(!next_result.has_value());

  const auto& handle_vector_ref = handle_vector;
  auto last_result = handle_vector_ref.call_return(
    thh::handle_t{}, [](const int& value) { return value; });

  CHECK(!last_result.has_value());
}

TEST_CASE("DefaultContainerIsEmpty")
{
  thh::handle_vector_t<int> handle_vector;
  CHECK(handle_vector.empty());
}

TEST_CASE("ContainerIsNotEmptyAfterAdd")
{
  thh::handle_vector_t<int> handle_vector;
  [[maybe_unused]] thh::handle_t handle = handle_vector.add(10);

  CHECK(!handle_vector.empty());
}

TEST_CASE("ContainerIsEmptyAfterRemove")
{
  thh::handle_vector_t<int> handle_vector;
  thh::handle_t handle = handle_vector.add(10);
  handle_vector.remove(handle);

  CHECK(handle_vector.empty());
}

TEST_CASE("ConstContainerEmptyCheck")
{
  thh::handle_vector_t<int> handle_vector;
  handle_vector.add();

  handle_vector.clear();

  const thh::handle_vector_t<int>& const_handle_vector = handle_vector;
  CHECK(const_handle_vector.empty());
}
