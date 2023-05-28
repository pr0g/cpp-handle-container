namespace thh
{
  template<typename Tag>
  bool operator==(
    const typed_handle_t<Tag>& lhs, const typed_handle_t<Tag>& rhs)
  {
    return lhs.gen_ == rhs.gen_ && lhs.id_ == rhs.id_;
  }

  template<typename Tag>
  bool operator!=(
    const typed_handle_t<Tag>& lhs, const typed_handle_t<Tag>& rhs)
  {
    return !(lhs == rhs);
  }

  template<typename T, typename Tag>
  void handle_vector_t<T, Tag>::try_allocate_handles()
  {
    if (handles_.size() < elements_.capacity()) {
      const auto last_handle_size = handles_.size();
      handles_.resize(elements_.capacity());
      assert(handles_.size() <= std::numeric_limits<int32_t>::max());
      for (size_t i = last_handle_size; i < handles_.size(); i++) {
        assert(i < std::numeric_limits<int32_t>::max() - 1);
        const auto handle_index = static_cast<int32_t>(i);
        handles_[handle_index].handle_ = typed_handle_t<Tag>(handle_index, -1);
        handles_[handle_index].lookup_ = -1;
        handles_[handle_index].next_ = handle_index + 1;
      }
    }
  }

  template<typename T, typename Tag>
  template<typename... Args>
  typed_handle_t<Tag> handle_vector_t<T, Tag>::add(Args&&... args)
  {
    assert(elements_.size() <= std::numeric_limits<int32_t>::max());

    const auto index = static_cast<int32_t>(elements_.size());

    // allocate new element
    elements_.emplace_back(std::forward<Args>(args)...);
    element_ids_.emplace_back();

    // if backing store increased, create additional handles for newly available
    // elements
    try_allocate_handles();

    // map handle to newly allocated element
    handles_[next_].lookup_ = index;
    // increment the generation of the handle
    typed_handle_t<Tag>* handle = &handles_[next_].handle_;
    handle->gen_++;

    // map the element back to the handle it's bound to
    element_ids_[index] = handle->id_;
    // update the next available handle
    next_ = handles_[next_].next_;

    return *handle;
  }

  template<typename T, typename Tag>
  template<typename Fn>
  void handle_vector_t<T, Tag>::call(const typed_handle_t<Tag> handle, Fn&& fn)
  {
    if (T* element = resolve(handle)) {
      fn(*element);
    }
  }

  template<typename T, typename Tag>
  template<typename Fn>
  void handle_vector_t<T, Tag>::call(
    const typed_handle_t<Tag> handle, Fn&& fn) const
  {
    if (const T* element = resolve(handle)) {
      fn(*element);
    }
  }

  template<typename T, typename Tag>
  template<typename Fn>
  decltype(auto) handle_vector_t<T, Tag>::call_return(
    typed_handle_t<Tag> handle, Fn&& fn)
  {
    if (T* element = resolve(handle)) {
      return std::optional(fn(*element));
    }
    return std::optional<decltype(fn(*(static_cast<T*>(nullptr))))>{};
  }

  template<typename T, typename Tag>
  template<typename Fn>
  decltype(auto) handle_vector_t<T, Tag>::call_return(
    typed_handle_t<Tag> handle, Fn&& fn) const
  {
    if (const T* element = resolve(handle)) {
      return std::optional(fn(*element));
    }
    return std::optional<decltype(fn(*(static_cast<const T*>(nullptr))))>{};
  }

  template<typename T, typename Tag>
  bool handle_vector_t<T, Tag>::has(const typed_handle_t<Tag> handle) const
  {
    assert(handles_.size() <= std::numeric_limits<int32_t>::max());

    if (handle.id_ >= static_cast<int32_t>(handles_.size())) {
      return false;
    }

    if (handle.id_ == -1) {
      return false;
    }

    // ensure the handle matches the one stored internally and is referencing a
    // valid element
    const internal_handle_t& ih = handles_[handle.id_];
    return ih.handle_.gen_ == handle.gen_ && ih.lookup_ != -1;
  }

  template<typename T, typename Tag>
  bool handle_vector_t<T, Tag>::remove(const typed_handle_t<Tag> handle)
  {
    assert(element_ids_.size() == elements_.size());

    if (!has(handle)) {
      return false;
    }

    using std::swap;
    const size_t back = element_ids_.size() - 1;
    // find the handle of the last element currently stored and have it
    // point to the look-up of the element about to be removed
    handles_[element_ids_[back]].lookup_ = handles_[handle.id_].lookup_;
    // swap the last element with the element being removed
    swap(elements_[handles_[handle.id_].lookup_], elements_[back]);
    // swap the last element id with the element id being removed
    // (the element_ and element_ids_ vector have a one to one mapping)
    swap(element_ids_[handles_[handle.id_].lookup_], element_ids_[back]);

    // free handle being removed (make ready for reuse)
    handles_[handle.id_].lookup_ = -1;
    handles_[handle.id_].next_ = next_;
    next_ = handle.id_;

    // remove the last element (the element that was removed after the swap)
    element_ids_.pop_back();
    elements_.pop_back();

    return true;
  }

  template<typename T, typename Tag>
  int32_t handle_vector_t<T, Tag>::size() const
  {
    assert(element_ids_.size() == elements_.size());
    assert(elements_.size() <= std::numeric_limits<int32_t>::max());
    return static_cast<int32_t>(elements_.size());
  }

  template<typename T, typename Tag>
  int32_t handle_vector_t<T, Tag>::capacity() const
  {
    assert(handles_.size() <= std::numeric_limits<int32_t>::max());
    return static_cast<int32_t>(handles_.size());
  }

  template<typename T, typename Tag>
  const T* handle_vector_t<T, Tag>::resolve(
    const typed_handle_t<Tag> handle) const
  {
    if (!has(handle)) {
      return nullptr;
    }
    return &elements_[handles_[handle.id_].lookup_];
  }

  template<typename T, typename Tag>
  T* handle_vector_t<T, Tag>::resolve(const typed_handle_t<Tag> handle)
  {
    return const_cast<T*>(
      static_cast<const handle_vector_t&>(*this).resolve(handle));
  }

  template<typename T, typename Tag>
  void handle_vector_t<T, Tag>::reserve(const int32_t capacity)
  {
    assert(capacity > 0);

    elements_.reserve(capacity);
    element_ids_.reserve(capacity);

    try_allocate_handles();
  }

  template<typename T, typename Tag>
  void handle_vector_t<T, Tag>::clear()
  {
    assert(handles_.size() <= std::numeric_limits<int32_t>::max());

    elements_.clear();
    element_ids_.clear();

    // reset handles but leave generation untouched (ensures existing external
    // handles cannot be used again with the container)
    for (size_t i = 0; i < handles_.size(); i++) {
      handles_[i].lookup_ = -1;
      handles_[i].next_ = static_cast<int32_t>(i) + 1;
    }

    next_ = 0;
  }

  template<typename T, typename Tag>
  typed_handle_t<Tag> handle_vector_t<T, Tag>::handle_from_index(
    const int32_t index) const
  {
    if (index < 0 || index >= static_cast<int32_t>(element_ids_.size())) {
      return typed_handle_t<Tag>{};
    }
    return handles_[element_ids_[index]].handle_;
  }

  template<typename T, typename Tag>
  std::optional<int32_t> handle_vector_t<T, Tag>::index_from_handle(
    const typed_handle_t<Tag> handle) const
  {
    if (!has(handle)) {
      return std::nullopt;
    }
    return handles_[handle.id_].lookup_;
  }

  template<typename T, typename Tag>
  bool handle_vector_t<T, Tag>::empty() const
  {
    assert(elements_.empty() == element_ids_.empty());
    return elements_.empty();
  }

  template<typename T, typename Tag>
  T& handle_vector_t<T, Tag>::operator[](const int32_t position)
  {
    return const_cast<T&>(
      static_cast<const handle_vector_t<T, Tag>&>(*this).operator[](position));
  }

  template<typename T, typename Tag>
  const T& handle_vector_t<T, Tag>::operator[](const int32_t position) const
  {
    assert(position <= elements_.size());
    return elements_[position];
  }

  template<typename T, typename Tag>
  T* handle_vector_t<T, Tag>::data()
  {
    return const_cast<T*>(
      static_cast<const handle_vector_t<T, Tag>&>(*this).data());
  }

  template<typename T, typename Tag>
  const T* handle_vector_t<T, Tag>::data() const
  {
    return elements_.data();
  }

  template<typename T, typename Tag>
  auto handle_vector_t<T, Tag>::begin() -> iterator
  {
    return elements_.begin();
  }

  template<typename T, typename Tag>
  auto handle_vector_t<T, Tag>::begin() const -> const_iterator
  {
    return elements_.begin();
  }

  template<typename T, typename Tag>
  auto handle_vector_t<T, Tag>::cbegin() const -> const_iterator
  {
    return elements_.cbegin();
  }

  template<typename T, typename Tag>
  auto handle_vector_t<T, Tag>::rbegin() -> reverse_iterator
  {
    return elements_.rbegin();
  }

  template<typename T, typename Tag>
  auto handle_vector_t<T, Tag>::rbegin() const -> const_reverse_iterator
  {
    return elements_.rbegin();
  }

  template<typename T, typename Tag>
  auto handle_vector_t<T, Tag>::crbegin() const -> const_reverse_iterator
  {
    return elements_.crbegin();
  }

  template<typename T, typename Tag>
  auto handle_vector_t<T, Tag>::end() -> iterator
  {
    return elements_.end();
  }

  template<typename T, typename Tag>
  auto handle_vector_t<T, Tag>::end() const -> const_iterator
  {
    return elements_.end();
  }

  template<typename T, typename Tag>
  auto handle_vector_t<T, Tag>::cend() const -> const_iterator
  {
    return elements_.cend();
  }

  template<typename T, typename Tag>
  auto handle_vector_t<T, Tag>::rend() -> reverse_iterator
  {
    return elements_.rend();
  }

  template<typename T, typename Tag>
  auto handle_vector_t<T, Tag>::rend() const -> const_reverse_iterator
  {
    return elements_.rend();
  }

  template<typename T, typename Tag>
  auto handle_vector_t<T, Tag>::crend() const -> const_reverse_iterator
  {
    return elements_.crend();
  }

  template<typename T, typename Tag>
  std::string handle_vector_t<T, Tag>::debug_handles() const
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

  namespace detail
  {
    // inspired by Raymond Chen, OldNewThing blog
    // https://devblogs.microsoft.com/oldnewthing/20170102-00/?p=95095
    template<typename... Iter>
    void apply_permutation(
      const int32_t begin, const int32_t end, std::vector<int>& indices,
      Iter... iters)
    {
      using std::swap;
      for (int32_t i = begin; i < end; i++) {
        auto current = i;
        while (i != indices[current - begin]) {
          auto next = indices[current - begin];
          ([&](const auto it) { swap(it[current - begin], it[next - begin]); }(
             iters),
           ...);
          indices[current - begin] = current;
          current = next;
        }
        indices[current - begin] = current;
      }
    }
  } // namespace detail

  template<typename T, typename Tag>
  void handle_vector_t<T, Tag>::fixup_handles(
    const int32_t begin, const int32_t end)
  {
    for (int32_t i = begin; i < end; ++i) {
      handles_[element_ids_[i - begin]].lookup_ = i - begin;
    }
  }

  template<typename T, typename Tag>
  template<typename Compare>
  void handle_vector_t<T, Tag>::sort(Compare&& compare)
  {
    sort(0, size(), std::forward<Compare>(compare));
  }

  template<typename T, typename Tag>
  template<typename Compare>
  void handle_vector_t<T, Tag>::sort(
    const int32_t begin, const int32_t end, Compare&& compare)
  {
    const auto range = std::min(size() - begin, end - begin);
    std::vector<int32_t> indices(range);
    std::iota(indices.begin(), indices.end(), begin);
    std::sort(indices.begin(), indices.end(), std::forward<Compare>(compare));
    detail::apply_permutation(
      begin, begin + range, indices, elements_.begin() + begin,
      element_ids_.begin() + begin);
    fixup_handles(begin, begin + range);
  }

  template<typename T, typename Tag>
  template<typename Predicate>
  int32_t handle_vector_t<T, Tag>::partition(Predicate&& predicate)
  {
    std::vector<int32_t> indices(size());
    std::iota(indices.begin(), indices.end(), 0);
    const auto second = std::partition(
      indices.begin(), indices.end(), std::forward<Predicate>(predicate));
    detail::apply_permutation(
      0, size(), indices, elements_.begin(), element_ids_.begin());
    fixup_handles(0, size());
    return int32_t(second - indices.begin());
  }
} // namespace thh
