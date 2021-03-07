@echo off

fd -0 -e hpp -e inl -e cpp -a | xargs -0 clang-format -i
