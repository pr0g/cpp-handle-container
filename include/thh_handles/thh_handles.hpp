#pragma once

#include <algorithm>
#include <cassert>
#include <vector>

namespace thh
{
  struct handle_t
  {
    int32_t id_;
    int32_t gen_;

    handle_t() = default;
    handle_t(const int32_t id, const int32_t gen)
      : id_(id), gen_(gen) {}
  };

  template<typename T>
  class container_t
  {
    struct internal_handle_t
    {
      handle_t handle_;
      int32_t lookup_;
      int32_t next_;
    };

    std::vector<T> elements_;
    std::vector<int32_t> element_ids_;
    std::vector<internal_handle_t> handles_;

    int32_t next_ = 0;
    size_t last_handle_size_ = 0;
  
  public:
    size_t size() const;
    size_t capacity() const;
    handle_t add();
    bool remove(handle_t handle);
    bool has(handle_t handle) const;
    const T* resolve(handle_t handle) const;
    T* resolve(handle_t handle);

    int debug_handles(int buffer_size, char buffer[]);
  };

  template<typename T>
  inline handle_t container_t<T>::add()
  {
    const int index = elements_.size();
    elements_.emplace_back();
    element_ids_.emplace_back();

    if (handles_.size() < elements_.capacity()) {
      handles_.resize(elements_.capacity());
    }

    for (size_t i = last_handle_size_; i < handles_.size(); ++i) {
      handles_[i].handle_ = handle_t(i, -1);
      handles_[i].lookup_ = -1;
      handles_[i].next_ = i + 1;
    }

    last_handle_size_ = handles_.size();

    handles_[next_].lookup_ = index;
    handle_t* handle = &handles_[next_].handle_;
    handle->gen_++;

    element_ids_[next_] = handle->id_;

    next_ = handles_[next_].next_;

    return *handle;
  }

  template<typename T>
  inline bool container_t<T>::has(const handle_t handle) const
  {
    if (handle.id_ >= handles_.size()) {
      return false;
    }

    const internal_handle_t& ih = handles_[handle.id_];
    return ih.handle_.gen_ == handle.gen_ && ih.lookup_ != -1;
  }

  template<typename T>
  inline bool container_t<T>::remove(const handle_t handle)
  {
    if (!has(handle)) {
      return false;
    }

    handles_[element_ids_[element_ids_.size() - 1]].lookup_ =
      handles_[handle.id_].lookup_;
    elements_[handles_[handle.id_].lookup_] = elements_[elements_.size() - 1];
    element_ids_[handles_[handle.id_].lookup_] = element_ids_[element_ids_.size() - 1];

    handles_[handle.id_].lookup_ = -1;
    handles_[handle.id_].next_ = next_;
    next_ = handle.id_;

    element_ids_.pop_back();
    elements_.pop_back();

    return true;
  }

  template<typename T>
  inline size_t container_t<T>::size() const
  {
    return elements_.size();
  }

  template<typename T>
  size_t container_t<T>::capacity() const
  {
    return handles_.size();
  }

  template<typename T>
  const T* container_t<T>::resolve(const handle_t handle) const
  {
    if (!has(handle)) {
      return nullptr;
    }

    return &elements_[handles_[handle.id_].lookup_];
  }

  template<typename T>
  T* container_t<T>::resolve(const handle_t handle)
  {
    return const_cast<T*>(static_cast<const container_t&>(*this).resolve(handle));
  }

  // debug
  template<typename T>
  int container_t<T>::debug_handles(int buffer_size, char buffer[]) {
    const char* filled_glyph = "[o]";
    const char* empty_glyph = "[x]";

    const int filled_glyph_len = strlen(filled_glyph);
    const int empty_glyph_len = strlen(empty_glyph);
    const int required_buffer_size =
      capacity() * std::max(filled_glyph_len, empty_glyph_len) + 1;

    if (buffer == nullptr) {
      return required_buffer_size;
    } else if (buffer_size < required_buffer_size) {
      return -1;
    } else {
      for (int i = 0; i < capacity(); i++) {
        const char* glyph = nullptr;
        if (handles_[i].lookup_ == -1) {
            glyph = empty_glyph;
        } else {
            glyph = filled_glyph;
        }

        strcat(buffer, glyph);
      }
      return 0;
    }
  }
} // namespace thh
