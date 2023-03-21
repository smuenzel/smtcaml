open! Core

let () =
  let btor = Smtcaml_bindings.Boolector.make () in
  Smtcaml_bindings.Boolector.print_stats btor
