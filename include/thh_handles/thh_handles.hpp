#pragma once

#include <algorithm>
#include <cassert>
#include <optional>
#include <string>
#include <utility>
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
    int32_t id_ = -1; // value to uniquely identify the handle
    int32_t gen_ = -1; // current handle generation

    typed_handle_t() = default;
    typed_handle_t(const int32_t id, const int32_t gen) : id_(id), gen_(gen) {}
  };

  // handle equality operators
  template<typename Tag>
  bool operator==(
    const typed_handle_t<Tag>& lhs, const typed_handle_t<Tag>& rhs);
  template<typename Tag>
  bool operator!=(
    const typed_handle_t<Tag>& lhs, const typed_handle_t<Tag>& rhs);

  using handle_t = typed_handle_t<default_tag_t>;

  // storage for type T that is created in-place
  // may be accessed by resolving the returned typed_handle_t from add()
  // note: provide a custom tag to create a type-safe container-handle pair
  template<typename T, typename Tag = default_tag_t>
  class container_t
  {
    // internal mapping from external handle to internal element
    // maintains a reference to the next free handle
    struct internal_handle_t
    {
      typed_handle_t<Tag> handle_; // handle to be looked up
      int32_t lookup_ = -1; // mapping to element
      int32_t next_ = -1; // index of next available handle
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
    // explicit alignment padding variable
    int32_t padding_unused_ = 0;

    // increases the number of availble handles when the underlying container of
    // elements (T) grows (the capacity increases)
    void try_allocate_more_handles();

  public:
    // creates an element T in-place and returns a handle to it
    // note: args allow arguments to be passed directly to the type constructor
    // useful if the type does not support a default constructor
    template<typename... Args>
    typed_handle_t<Tag> add(Args&&... args);
    // creates an element T in-place and returns a handle and pointer to it
    // immediately (convenience function)
    template<typename... Args>
    std::pair<typed_handle_t<Tag>, T*> add_and_resolve(Args&&... args);
    // invokes a callable object (usually a lambda) on a particular element in
    // the container
    template<typename Fn>
    void call(typed_handle_t<Tag> handle, Fn&& fn);
    // invokes a callable object (usually a lambda) on a particular element in
    // the container (const overload)
    template<typename Fn>
    void call(typed_handle_t<Tag> handle, Fn&& fn) const;
    // invokes a callable object (usually a lambda) on a particular element in
    // the container and returns a std::optional containing either the result
    // or an empty optional (as the handle may not have been successfully
    // resolved)
    template<typename Fn>
    decltype(auto) call_return(typed_handle_t<Tag> handle, Fn&& fn);
    // invokes a callable object (usually a lambda) on a particular element in
    // the container and returns a std::optional containing either the result
    // or an empty optional (as the handle may not have been successfully
    // resolved) (const overload)
    template<typename Fn>
    decltype(auto) call_return(typed_handle_t<Tag> handle, Fn&& fn) const;
    // removes the element referenced by the handle
    // returns true if the element was removed, false otherwise (the handle was
    // invalid or could not be found in the container)
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
    // returns if the container has any elements or not
    [[nodiscard]] bool empty() const;
    // returns an iterator to the beginning of the elements
    auto begin() -> typename decltype(elements_)::iterator;
    // returns a const iterator to the beginning of the elements
    auto begin() const -> typename decltype(elements_)::const_iterator;
    // returns a const iterator to the beginning of the elements
    auto cbegin() const -> typename decltype(elements_)::const_iterator;
    // returns an iterator to the end of the elements
    auto end() -> typename decltype(elements_)::iterator;
    // returns a const iterator to the end of the elements
    auto end() const -> typename decltype(elements_)::const_iterator;
    // returns a const iterator to the end of the elements
    auto cend() const -> typename decltype(elements_)::const_iterator;
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
