# llvm
llvm 2.6~3.x  study

目前所用电脑为 macbook pro2019  intel i9  x86_64 arch
- 保留autoconf下面的文件，检测默认的host triple，也可以直接删除
- config-ix.cmake 写死了HostTargetTriple  x86_64-apple-darwin22.5.0
- llvm/Support/ConstantRange 暂未include llvm/Instructions.h，先注释掉了依赖函数
- llvm/Support/TargetRegistry.cpp 全部注释掉了，因为依赖 target
- llvm/Support/CommandLine.cpp  注释掉#include "llvm/Target/TargetRegistry.h"，注释掉了1146行
- llvm/Support/CommandLine.cpp 1051行，替换std::ptr_fun
```cpp
    Opts.erase(std::remove_if(Opts.begin(), Opts.end(),
  [&](std::pair<std::string, llvm::cl::Option *> &pair)->bool
  {return ShowHidden ? isReallyHidden(pair) : isHidden(pair);}),
  Opts.end());

    // Eliminate Hidden or ReallyHidden arguments, depending on ShowHidden
    Opts.erase(std::remove_if(Opts.begin(), Opts.end(),
                          std::ptr_fun(ShowHidden ? isReallyHidden : isHidden)),
               Opts.end());
```
