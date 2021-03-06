namespace thh
{
  template<typename T>
  inline void container_t<T>::try_allocate_more_handles()
  {
    if (handles_.size() < elements_.capacity()) {
      handles_.resize(elements_.capacity());

      assert(handles_.size() <= std::numeric_limits<int32_t>::max());

      const auto last_handle_size = static_cast<size_t>(last_handle_size_);
      for (size_t i = last_handle_size; i < handles_.size(); i++) {
        assert(i < std::numeric_limits<int32_t>::max() - 1);
        const auto handle_index = static_cast<int32_t>(i);
        handles_[i].handle_ = handle_t(handle_index, -1);
        handles_[i].lookup_ = -1;
        handles_[i].next_ = handle_index + 1;
      }
      last_handle_size_ = static_cast<int32_t>(handles_.size());
    }
  }

  template<typename T>
  inline handle_t container_t<T>::add()
  {
    assert(elements_.size() <= std::numeric_limits<int32_t>::max());

    const auto index = static_cast<int32_t>(elements_.size());
    elements_.emplace_back();
    element_ids_.emplace_back();

    try_allocate_more_handles();

    handles_[next_].lookup_ = index;
    handle_t* handle = &handles_[next_].handle_;
    handle->gen_++;

    element_ids_[index] = handle->id_;
    next_ = handles_[next_].next_;

    return *handle;
  }

  template<typename T>
  inline bool container_t<T>::has(const handle_t handle) const
  {
    assert(handles_.size() <= std::numeric_limits<int32_t>::max());

    if (handle.id_ >= static_cast<int32_t>(handles_.size())) {
      return false;
    }

    if (handle.id_ == -1) {
      return false;
    }

    const internal_handle_t& ih = handles_[handle.id_];
    return ih.handle_.gen_ == handle.gen_ && ih.lookup_ != -1;
  }

  template<typename T>
  inline bool container_t<T>::remove(const handle_t handle)
  {
    assert(element_ids_.size() == elements_.size());

    if (!has(handle)) {
      return false;
    }

    const size_t back = element_ids_.size() - 1;
    // find the handle of the last element currently stored and have it
    // point to the look-up of the element about to be removed
    handles_[element_ids_[back]].lookup_ = handles_[handle.id_].lookup_;
    // swap the last element with the element being removed
    std::swap(elements_[handles_[handle.id_].lookup_], elements_[back]);
    // swap the last element id with the element id being removed
    // (the element_ and element_ids_ vector have a one to one mapping)
    std::swap(element_ids_[handles_[handle.id_].lookup_], element_ids_[back]);

    // free handle being removed (make ready for reuse)
    handles_[handle.id_].lookup_ = -1;
    handles_[handle.id_].next_ = next_;
    next_ = handle.id_;

    // remove the last element (the element that was removed after the swap)
    element_ids_.pop_back();
    elements_.pop_back();

    return true;
  }

  template<typename T>
  inline int32_t container_t<T>::size() const
  {
    assert(element_ids_.size() == elements_.size());
    assert(elements_.size() <= std::numeric_limits<int32_t>::max());
    return static_cast<int32_t>(elements_.size());
  }

  template<typename T>
  int32_t container_t<T>::capacity() const
  {
    assert(handles_.size() <= std::numeric_limits<int32_t>::max());
    return static_cast<int32_t>(handles_.size());
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
    return const_cast<T*>(
      static_cast<const container_t&>(*this).resolve(handle));
  }

  template<typename T>
  void container_t<T>::reserve(const int32_t capacity)
  {
    assert(capacity > 0);

    elements_.reserve(capacity);
    element_ids_.reserve(capacity);

    try_allocate_more_handles();
  }

  template<typename T>
  void container_t<T>::clear()
  {
    assert(handles_.size() <= std::numeric_limits<int32_t>::max());

    elements_.clear();
    element_ids_.clear();

    for (size_t i = 0; i < handles_.size(); i++) {
      handles_[i].lookup_ = -1;
      handles_[i].next_ = static_cast<int32_t>(i) + 1;
    }

    next_ = 0;
  }

  template<typename T>
  template<typename Fn>
  void container_t<T>::enumerate(Fn&& fn)
  {
    for (auto& element : elements_) {
      fn(element);
    }
  }

  template<typename T>
  std::string container_t<T>::debug_handles() const
  {
    constexpr const char filled_glyph[] = "[o]";
    constexpr const char empty_glyph[] = "[x]";

    std::string buffer;
    for (int32_t i = 0; i < capacity(); i++) {
      const char* glyph = nullptr;
      if (handles_[i].lookup_ == -1) {
        glyph = empty_glyph;
      } else {
        glyph = filled_glyph;
      }
      buffer.append(glyph);
    }

    return buffer;
  }
} // namespace thh
