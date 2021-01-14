# bfi - brainfuck interpreter

TODO:
  - [x] interpret AST at runtime
  - [ ] optimise the AST
      - [ ] try guessing how much memory is needed for execution
      - [ ] dynamically adjust memory during execution (incase guess fails)
  - [ ] generate code and execute in memory
  - [ ] generate ELF (if even possible)
  - [ ] generate LLVM IR to feed to LLVM
  - [ ] expand this todo list

pros:
  - simple
  - has an AST for expandability
  
cons:
  - naive
  - minimal optimisations
