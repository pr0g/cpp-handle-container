namespace thh
{
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
    assert(element_ids_.size() == elements_.size());
    
    if (!has(handle)) {
      return false;
    }

    const size_t back = element_ids_.size() - 1;
    handles_[element_ids_[back]].lookup_ = handles_[handle.id_].lookup_;
    std::swap(elements_[handles_[handle.id_].lookup_], elements_[back]);
    std::swap(element_ids_[handles_[handle.id_].lookup_], element_ids_[back]);

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
    return const_cast<T*>(
      static_cast<const container_t&>(*this).resolve(handle));
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
