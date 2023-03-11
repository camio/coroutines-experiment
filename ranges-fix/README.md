# Ranges Fix

Due to issue [#61317](https://github.com/llvm/llvm-project/issues/61317) in
LLVM, the standard `<ranges>` header could not be included from multiple
modules. As a workaround, I patched my system's ranges header to avoid the
issue. In this directory you will see the system's header and the patch I used
to make things work.
