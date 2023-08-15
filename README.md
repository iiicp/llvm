# llvm
llvm 2.6~3.x  study

目前所用电脑为 macbook pro2019  intel i9  x86_64 arch

- llvm/autoconf 文件用来检测当前的host triple，默认的llvm2.6携带的会检测失败，采用了llvm3.0里面的
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
