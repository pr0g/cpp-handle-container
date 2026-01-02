namespace thh
{
  template<typename Tag, typename Index, typename Gen>
  bool operator==(
    const typed_handle_t<Tag, Index, Gen>& lhs,
    const typed_handle_t<Tag, Index, Gen>& rhs)
  {
    return lhs.gen_ == rhs.gen_ && lhs.id_ == rhs.id_;
  }

  template<typename Tag, typename Index, typename Gen>
  bool operator!=(
    const typed_handle_t<Tag, Index, Gen>& lhs,
    const typed_handle_t<Tag, Index, Gen>& rhs)
  {
    return !(lhs == rhs);
  }

  template<typename Tag, typename Index, typename Gen>
  bool operator<(
    const typed_handle_t<Tag, Index, Gen>& lhs,
    const typed_handle_t<Tag, Index, Gen>& rhs)
  {
    if (lhs.id_ == rhs.id_) {
      return lhs.gen_ < rhs.gen_;
    }
    return lhs.id_ < rhs.id_;
  }

  template<typename Tag, typename Index, typename Gen>
  bool operator>(
    const typed_handle_t<Tag, Index, Gen>& lhs,
    const typed_handle_t<Tag, Index, Gen>& rhs)
  {
    return rhs < lhs;
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  void handle_vector_t<T, Tag, Index, Gen>::try_allocate_handles()
  {
    if (handles_.size() - depleted_handles_ < elements_.capacity()) {
      const auto last_handle_size = handles_.size();
      const auto handle_count = elements_.capacity() + depleted_handles_;
      assert(handle_count <= std::numeric_limits<Index>::max());
      handles_.resize(handle_count);
      for (size_t i = last_handle_size; i < handles_.size(); i++) {
        assert(i <= std::numeric_limits<Index>::max());
        const auto handle_index = static_cast<Index>(i);
        handles_[handle_index].gen_ = -1;
        handles_[handle_index].lookup_ = -1;
        handles_[handle_index].next_ = handle_index + 1;
      }
      dequeue_ = static_cast<Index>(last_handle_size);
      enqueue_ = static_cast<Index>(handles_.size() - 1);
    }
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  template<typename... Args>
  typed_handle_t<Tag, Index, Gen> handle_vector_t<T, Tag, Index, Gen>::add(
    Args&&... args)
  {
    const auto lookup = static_cast<Index>(elements_.size());

    assert(lookup <= std::numeric_limits<Index>::max());

    // allocate new element
    elements_.emplace_back(std::forward<Args>(args)...);
    element_ids_.emplace_back();

    // if backing store increased, create additional
    // handles for newly available elements
    try_allocate_handles();

    while (dequeue_ < static_cast<Index>(handles_.size())
           && handles_[dequeue_].gen_ == std::numeric_limits<Gen>::max()) {
      // skip handle for allocation if generation has reached its limit
      const auto dequeue_before = dequeue_;
      dequeue_ = handles_[dequeue_].next_;
      depleted_handles_++;
      // ensure we don't get stuck in an infinite loop (may happen if we
      // currently only have one handle and it uses up all its generations)
      if (dequeue_before == dequeue_) {
        dequeue_++;
        break;
      }
    }

    // if several handles have been depleted, create additional handles for
    // available element capacity
    try_allocate_handles();

    const auto index = dequeue_;
    // increment the generation of the handle
    auto& internal_handle = handles_[index];
    assert(internal_handle.lookup_ == -1); // ensure handle is free
    internal_handle.gen_++;

    // map handle to newly allocated element
    internal_handle.lookup_ = lookup;

    // map the element back to the handle it's bound to
    element_ids_[lookup] = index;
    // update the next available handle
    dequeue_ = internal_handle.next_;

    return {index, internal_handle.gen_};
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  template<typename Fn>
  void handle_vector_t<T, Tag, Index, Gen>::call(
    const typed_handle_t<Tag, Index, Gen> handle, Fn&& fn)
  {
    if (T* element = resolve(handle)) {
      fn(*element);
    }
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  template<typename Fn>
  void handle_vector_t<T, Tag, Index, Gen>::call(
    const typed_handle_t<Tag, Index, Gen> handle, Fn&& fn) const
  {
    if (const T* element = resolve(handle)) {
      fn(*element);
    }
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  template<typename Fn>
  decltype(auto) handle_vector_t<T, Tag, Index, Gen>::call_return(
    typed_handle_t<Tag, Index, Gen> handle, Fn&& fn)
  {
    if (T* element = resolve(handle)) {
      return std::optional(fn(*element));
    }
    return std::optional<decltype(fn(*(static_cast<T*>(nullptr))))>{};
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  template<typename Fn>
  decltype(auto) handle_vector_t<T, Tag, Index, Gen>::call_return(
    typed_handle_t<Tag, Index, Gen> handle, Fn&& fn) const
  {
    if (const T* element = resolve(handle)) {
      return std::optional(fn(*element));
    }
    return std::optional<decltype(fn(*(static_cast<const T*>(nullptr))))>{};
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  bool handle_vector_t<T, Tag, Index, Gen>::has(
    const typed_handle_t<Tag, Index, Gen> handle) const
  {
    assert(handles_.size() <= std::numeric_limits<Index>::max());

    if (handle.id_ < 0 || handle.id_ >= static_cast<Index>(handles_.size())) {
      return false;
    }

    // ensure the handle matches the one stored internally
    // and is referencing a valid element
    const internal_handle_t& ih = handles_[handle.id_];
    return ih.gen_ == handle.gen_ && ih.lookup_ >= 0
        && ih.lookup_ < static_cast<Index>(elements_.size());
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  bool handle_vector_t<T, Tag, Index, Gen>::remove(
    const typed_handle_t<Tag, Index, Gen> handle)
  {
    assert(element_ids_.size() == elements_.size());

    if (!has(handle)) {
      return false;
    }

    using std::swap;
    auto& internal_handle = handles_[handle.id_];
    const auto lookup = internal_handle.lookup_;
    // find the handle of the last element currently stored and have it
    // point to the look-up of the element about to be removed
    handles_[element_ids_.back()].lookup_ = lookup;
    // swap the last element with the element being removed and then pop_back
    // (the element and element_ids vector have a one to one mapping)
    swap(elements_[lookup], elements_.back());
    elements_.pop_back();
    swap(element_ids_[lookup], element_ids_.back());
    element_ids_.pop_back();

    // free handle being removed (make ready for reuse)
    internal_handle.lookup_ = -1;

    handles_[handle.id_].next_ = handles_[enqueue_].next_;
    handles_[enqueue_].next_ = handle.id_;
    enqueue_ = handle.id_;

    if (dequeue_ == static_cast<Index>(handles_.size())) {
      dequeue_ = enqueue_;
    }

    return true;
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  Index handle_vector_t<T, Tag, Index, Gen>::size() const
  {
    assert(element_ids_.size() == elements_.size());
    assert(elements_.size() <= std::numeric_limits<Index>::max());
    return static_cast<Index>(elements_.size());
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  Index handle_vector_t<T, Tag, Index, Gen>::capacity() const
  {
    assert(handles_.size() <= std::numeric_limits<Index>::max());
    return static_cast<Index>(handles_.size());
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  const T* handle_vector_t<T, Tag, Index, Gen>::resolve(
    const typed_handle_t<Tag, Index, Gen> handle) const
  {
    if (!has(handle)) {
      return nullptr;
    }
    return &elements_[handles_[handle.id_].lookup_];
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  T* handle_vector_t<T, Tag, Index, Gen>::resolve(
    const typed_handle_t<Tag, Index, Gen> handle)
  {
    return const_cast<T*>(
      static_cast<const handle_vector_t&>(*this).resolve(handle));
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  void handle_vector_t<T, Tag, Index, Gen>::reserve(const Index capacity)
  {
    assert(capacity > 0);

    elements_.reserve(capacity);
    element_ids_.reserve(capacity);

    try_allocate_handles();
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  void handle_vector_t<T, Tag, Index, Gen>::clear()
  {
    assert(handles_.size() <= std::numeric_limits<Index>::max());

    elements_.clear();
    element_ids_.clear();

    // reset handles but leave generation untouched (ensures existing external
    // handles cannot be used again with the container)
    for (size_t i = 0; i < handles_.size(); i++) {
      handles_[i].lookup_ = -1;
      handles_[i].next_ = static_cast<Index>(i) + 1;
    }

    depleted_handles_ = 0;
    dequeue_ = 0;
    enqueue_ = static_cast<Index>(handles_.size() - 1);
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  typed_handle_t<Tag, Index, Gen> handle_vector_t<
    T, Tag, Index, Gen>::handle_from_index(const Index index) const
  {
    if (index < 0 || index >= static_cast<Index>(element_ids_.size())) {
      return typed_handle_t<Tag, Index, Gen>{};
    }
    const auto handle = element_ids_[index];
    return {handle, handles_[handle].gen_};
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  std::optional<Index> handle_vector_t<T, Tag, Index, Gen>::index_from_handle(
    const typed_handle_t<Tag, Index, Gen> handle) const
  {
    if (!has(handle)) {
      return std::nullopt;
    }
    return handles_[handle.id_].lookup_;
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  bool handle_vector_t<T, Tag, Index, Gen>::empty() const
  {
    assert(elements_.empty() == element_ids_.empty());
    return elements_.empty();
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  T& handle_vector_t<T, Tag, Index, Gen>::operator[](const Index position)
  {
    return const_cast<T&>(
      static_cast<const handle_vector_t<T, Tag, Index, Gen>&>(*this).operator[](
        position));
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  const T& handle_vector_t<T, Tag, Index, Gen>::operator[](
    const Index position) const
  {
    assert(position <= static_cast<int64_t>(elements_.size()));
    return elements_[position];
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  T* handle_vector_t<T, Tag, Index, Gen>::data()
  {
    return const_cast<T*>(
      static_cast<const handle_vector_t<T, Tag, Index, Gen>&>(*this).data());
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  const T* handle_vector_t<T, Tag, Index, Gen>::data() const
  {
    return elements_.data();
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  auto handle_vector_t<T, Tag, Index, Gen>::begin() -> iterator
  {
    return elements_.begin();
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  auto handle_vector_t<T, Tag, Index, Gen>::begin() const -> const_iterator
  {
    return elements_.begin();
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  auto handle_vector_t<T, Tag, Index, Gen>::cbegin() const -> const_iterator
  {
    return elements_.cbegin();
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  auto handle_vector_t<T, Tag, Index, Gen>::rbegin() -> reverse_iterator
  {
    return elements_.rbegin();
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  auto handle_vector_t<T, Tag, Index, Gen>::rbegin() const
    -> const_reverse_iterator
  {
    return elements_.rbegin();
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  auto handle_vector_t<T, Tag, Index, Gen>::crbegin() const
    -> const_reverse_iterator
  {
    return elements_.crbegin();
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  auto handle_vector_t<T, Tag, Index, Gen>::end() -> iterator
  {
    return elements_.end();
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  auto handle_vector_t<T, Tag, Index, Gen>::end() const -> const_iterator
  {
    return elements_.end();
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  auto handle_vector_t<T, Tag, Index, Gen>::cend() const -> const_iterator
  {
    return elements_.cend();
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  auto handle_vector_t<T, Tag, Index, Gen>::rend() -> reverse_iterator
  {
    return elements_.rend();
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  auto handle_vector_t<T, Tag, Index, Gen>::rend() const
    -> const_reverse_iterator
  {
    return elements_.rend();
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  auto handle_vector_t<T, Tag, Index, Gen>::crend() const
    -> const_reverse_iterator
  {
    return elements_.crend();
  }

  namespace detail
  {
    // inspired by Raymond Chen, OldNewThing blog
    // https://devblogs.microsoft.com/oldnewthing/20170102-00/?p=95095
    template<typename Index, typename... Iter>
    void apply_permutation(
      const Index begin, const Index end, std::vector<Index>& indices,
      Iter... iters)
    {
      using std::swap;
      for (Index i = begin; i < end; i++) {
        auto current = i;
        while (i != indices[current - begin]) {
          const auto next = indices[current - begin];
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

  template<typename T, typename Tag, typename Index, typename Gen>
  void handle_vector_t<T, Tag, Index, Gen>::fixup_handles(
    const Index begin, const Index end)
  {
    for (Index i = begin; i < end; ++i) {
      handles_[element_ids_[i - begin]].lookup_ = i - begin;
    }
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  template<typename Compare>
  void handle_vector_t<T, Tag, Index, Gen>::sort(Compare&& compare)
  {
    sort(Index(0), size(), std::forward<Compare>(compare));
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  template<typename Compare>
  void handle_vector_t<T, Tag, Index, Gen>::sort(
    const Index begin, const Index end, Compare&& compare)
  {
    const auto range = std::min(size() - begin, end - begin);
    std::vector<Index> indices(range);
    std::iota(indices.begin(), indices.end(), begin);
    std::sort(indices.begin(), indices.end(), std::forward<Compare>(compare));
    detail::apply_permutation<Index>(
      begin, begin + range, indices, elements_.begin() + begin,
      element_ids_.begin() + begin);
    fixup_handles(begin, begin + range);
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  template<typename Predicate>
  Index handle_vector_t<T, Tag, Index, Gen>::partition(Predicate&& predicate)
  {
    std::vector<Index> indices(size());
    std::iota(indices.begin(), indices.end(), 0);
    const auto second = std::partition(
      indices.begin(), indices.end(), std::forward<Predicate>(predicate));
    detail::apply_permutation(
      Index(0), size(), indices, elements_.begin(), element_ids_.begin());
    fixup_handles(Index(0), size());
    return Index(second - indices.begin());
  }

  template<typename T, typename Tag, typename Index, typename Gen>
  std::string debug_handles(
    const handle_vector_t<T, Tag, Index, Gen>& handle_vector)
  {
    constexpr std::string_view filled_glyph = "[o]";
    constexpr std::string_view empty_glyph = "[x]";
    constexpr std::string_view depleted_glyph = "[!]";

    const auto& handles = handle_vector.handles_;

    std::string buffer;
    for (Index i = 0; i < handle_vector.capacity(); i++) {
      std::string_view glyph;
      if (handles[i].gen_ == std::numeric_limits<Gen>::max()) {
        glyph = depleted_glyph;
      } else if (handles[i].lookup_ == -1) {
        glyph = empty_glyph;
      } else {
        glyph = filled_glyph;
      }
      buffer.append(glyph);
    }

    return buffer;
  }
} // namespace thh
