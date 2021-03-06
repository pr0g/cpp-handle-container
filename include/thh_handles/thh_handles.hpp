#pragma once

#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

namespace thh
{
  struct handle_t
  {
    int32_t id_ = -1;
    int32_t gen_ = -1;

    handle_t() = default;
    handle_t(const int32_t id, const int32_t gen) : id_(id), gen_(gen) {}
  };

  template<typename T>
  class container_t
  {
    struct internal_handle_t
    {
      handle_t handle_;
      int32_t lookup_ = -1;
      int32_t next_ = -1;
    };

    std::vector<T> elements_;
    std::vector<int32_t> element_ids_;
    std::vector<internal_handle_t> handles_;

    int32_t last_handle_size_ = 0;
    int32_t next_ = 0;

    void try_allocate_more_handles();

  public:
    handle_t add();
    bool remove(handle_t handle);
    bool has(handle_t handle) const;
    const T* resolve(handle_t handle) const;
    T* resolve(handle_t handle);
    int32_t size() const;
    int32_t capacity() const;
    void reserve(int32_t capacity);
    void clear();

    template<typename Fn>
    void enumerate(Fn&& fn);

    std::string debug_handles() const;
  };
} // namespace thh

#include "thh_handles.inl"
