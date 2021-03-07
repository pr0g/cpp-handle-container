## todo

- ~~add `reserve` functionality~~
- ~~add `clear` functionality~~
  - ~~is there a safe way to do this?~~ - Yes
  - ~~introduce container generation for extra safety? increment after each `clear` operation?~~ - No, just maintain internal handle vector (not perfect but should be okay in practice. To fully reset, `.swap(tmp)` technique is possible).
- ~~investigate benchmarking code~~
- ~~add a way to distinguish handles from separate containers~~
- consider special members (move only type?)
- add iterator support (begin/end interface?)
