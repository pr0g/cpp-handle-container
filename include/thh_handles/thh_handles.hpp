#pragma once

#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

namespace thh
{
  // default tag to circumvent type safety
  struct default_tag_t
  {
  };

  // weak handle to underlying data stored in the container
  // note: fields should not be modified
  template<typename Tag>
  struct typed_handle_t
  {
    int32_t id_ = -1;
    int32_t gen_ = -1;

    typed_handle_t() = default;
    typed_handle_t(const int32_t id, const int32_t gen) : id_(id), gen_(gen) {}
  };

  using handle_t = typed_handle_t<default_tag_t>;

  // storage for type T that is created in-place
  // may be accessed by resolving the returned typed_handle_t from add()
  template<typename T, typename Tag = default_tag_t>
  class container_t
  {
    // internal mapping from external handle to internal element
    // maintains a reference to the next free handle
    struct internal_handle_t
    {
      typed_handle_t<Tag> handle_;
      int32_t lookup_ = -1;
      int32_t next_ = -1;
    };

    // backing container for elements (vector remains tightly packed)
    std::vector<T> elements_;
    // parallel vector of ids that map from elements back to the corresponding
    // handle
    std::vector<int32_t> element_ids_;
    // sparse vector of handles to elements
    std::vector<internal_handle_t> handles_;

    // index of the next handle to be allocated
    int32_t next_ = 0;

    // increases the number of availble handles when the underlying container of
    // elements (T) grows (the capacity increases)
    void try_allocate_more_handles();

  public:
    // creates an element T in-place and returns a handle to it
    typed_handle_t<Tag> add();
    // removes the element referenced by the handle
    // returns true if the elements was removed, false otherwise (the handle was
    // invalid)
    bool remove(typed_handle_t<Tag> handle);
    // returns if the container still has the element referenced by the handle
    [[nodiscard]] bool has(typed_handle_t<Tag> handle) const;
    // returns a constant pointer to the underlying element T referenced by the
    // handle
    [[nodiscard]] const T* resolve(typed_handle_t<Tag> handle) const;
    // returns a mutable pointer to the underlying element T referenced by the
    // handle
    [[nodiscard]] T* resolve(typed_handle_t<Tag> handle);
    // returns the number of elements currently allocated by the container
    [[nodiscard]] int32_t size() const;
    // returns the number of available handles (includes element storage that is
    // reserved but not yet in use)
    [[nodiscard]] int32_t capacity() const;
    // reserves underlying memory for the number of elements specified
    void reserve(int32_t capacity);
    // removes all elements and invalidates all handles
    // note: capacity remains unchanged, internal handles are not cleared
    void clear();
    // enumerate each element stored in the container invoking the provided
    // function
    template<typename Fn>
    void enumerate(Fn&& fn);
    // return an ascii representation of the currently allocated handles
    // note: useful for debugging purposes
    [[nodiscard]] std::string debug_handles() const;
  };
} // namespace thh

#include "thh_handles.inl"
