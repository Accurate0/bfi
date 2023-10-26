# bfi - brainfuck interpreter/jit

TODO:
  - [x] interpret AST at runtime
  - [ ] optimise the AST (http://calmerthanyouare.org/2015/01/07/optimizing-brainfuck.html)
      - [x] clear loops
      - [x] simplification (+x5 -x3 == +x2)
      - [ ] copy loops
      - [ ] multiplication loops
      - [x] scan loops
      - [ ] operation offsets

pros:
  - simple
  - has an AST for expandability

cons:
  - naive
  - minimal optimisations
  - memory hungry due to AST
