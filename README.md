# smtCaml

smtCaml provides OCaml interfaces to various smt solvers (boolector, cvc5, z3) using a
unified interface.
Bindings are semi-automatically generated using cppcaml, which uses C++ type information
to infer conversion and function types in OCaml.

## Usability
smtCaml is not usable yet.

## Build instructions
First go to the vendor directory and build the dependencies in `vendor-boolector` and
`vendor-cvc5`, in that order. Then, you can use dune.
