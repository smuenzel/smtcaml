open! Core

let () =
  let open Smtcaml_bindings.Boolector in
  let btor = make () in
  let tr = true_val btor in
  assert_ tr;
  let result = sat btor in
  Smtcaml_bindings.Boolector.print_stats btor;
  print_s ([%sexp_of: solver_result] result);
