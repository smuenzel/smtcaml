# smtCaml

smtCaml provides OCaml interfaces to various smt solvers (boolector, cvc5, z3) using a
unified interface.
Bindings are semi-automatically generated using `cppcaml`, which uses C++ type information
to infer conversion and function types in OCaml.

## Usability
smtCaml is not usable yet.

## Build instructions
First go to the vendor directory and build the dependencies in `vendor-boolector` and
`vendor-cvc5`, in that order. Then, you can use dune.

## cppcaml
`cppcaml` is a C++ library that allows specification of conversion properties for C++
types, and declaration of API functions. The conversion to and from ocaml values are
automatically inferred from the declared properties and custom conversion functions.
This relies on the function parameters and return types in C++ being correctly declared.
The function names and types are automatically communicated to OCaml by storing them
in a special linker section, which can then be retrieved using cinaps, in order to
generate the OCaml `external` declarations.
