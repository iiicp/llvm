# TODO: Support other architectures. See Makefile.
#add_definitions(-DENABLE_X86_JIT)

set(LLVMJIT
  Intercept.cpp
  JIT.cpp
  JITDwarfEmitter.cpp
  JITEmitter.cpp
  JITMemoryManager.cpp
  MacOSJITEventListener.cpp
  OProfileJITEventListener.cpp
  TargetSelect.cpp
        CACHE INTERNAL ""
  )
