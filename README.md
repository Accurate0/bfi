# bfi - brainfuck interpreter

TODO:
  - [x] interpret AST at runtime
  - [ ] optimise the AST (http://calmerthanyouare.org/2015/01/07/optimizing-brainfuck.html)
      - [x] clear loops
      - [x] simplification (+x5 -x3 == +x2)
      - [ ] copy loops
      - [ ] multiplication loops
      - [x] scan loops
      - [ ] operation offsets
  - [ ] mmap the file instead of reading char by char
  - [ ] simplify or potentially remove AST (memory)
  - [ ] try guessing how much memory is needed for execution
  - [ ] dynamically adjust memory during execution (incase guess fails)
  - [ ] generate code and execute in memory
  - [ ] generate ELF (if even possible)
  - [ ] generate LLVM IR to feed to LLVM
  - [ ] expand this todo list
  - [ ] remove recursion to handle bigger programs (haven't had a stack overflow yet but...)
  - [ ] switch to an objectively better build system (cmake/meson + ninja)

pros:
  - simple
  - has an AST for expandability

cons:
  - naive
  - minimal optimisations
  - memory hungry due to AST (although gcc allocates 188,662 bytes with no input, are we the evil ones?)
