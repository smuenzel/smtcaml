open! Core

let () =
  let open Smtcaml_bindings.Boolector in
  let btor = make () in
  set_solver btor Cadical;
  let opts = all_options btor in
  List.iter opts ~f:(fun o -> print_s [%sexp (o : BOption.t)]);
  let tr = true_val btor in
  assert_ tr;
  let result = sat btor in
  print_stats btor;
  print_s ([%sexp_of: solver_result] result);
