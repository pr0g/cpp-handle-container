@echo off

fd -0 -e cpp -a | xargs -0 clang-tidy -p build/compile_commands.json -header-filter="thh_.*" -fix
