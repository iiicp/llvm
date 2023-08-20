# llvm
llvm 3.0  study

目前所用电脑为 macbook pro2019  intel i9  x86_64 arch

```cpp
CodeExtractor.cpp:728:28: error: no matching constructor for initialization of 'std::vector<BasicBlock *>'

fix to 
        
class SuccIterator : public std::iterator<std::random_access_iterator_tag, BB_,
                                          int, BB_ *, BB_ *>
``` 

```cpp 
LoopInfo.h:831:19: error: no matching member function for call to 'insert'

fix to

for (typename InvBlockTraits::ChildIteratorType I =
                                                    InvBlockTraits::child_begin(X),
                                                E = InvBlockTraits::child_end(X); I != E; ++I)
  TodoStack.push_back(*I);
```