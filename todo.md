# todo

- ~~add `reserve` functionality~~
- ~~add `clear` functionality~~
  - ~~is there a safe way to do this?~~ - Yes
  - ~~introduce container generation for extra safety? increment after each `clear` operation?~~ - No, just maintain internal handle vector (not perfect but should be okay in practice. To fully reset, `.swap(tmp)` technique is possible).
- ~~investigate benchmarking code~~
- ~~add a way to distinguish handles from separate containers~~
- ~~add iterator support (begin/end interface?)~~
- ~~consider special members (move only type?)~~ No, shouldn't limit all possible use cases.

## other

### potential names

- variable_indirect_array
- indirect_vector
- handle_vector
- sparse_array
- packed_sparse_array
- dense_array
- sequential_lookup
