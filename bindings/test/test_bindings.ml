open! Core

let () =
  print_endline "Boolector";
  let open Smtcaml_bindings.Boolector in
  let api_registry = Cppcaml_lib.get_api_registry () in
  print_s
    [%message "registry"
        (api_registry : Cppcaml_lib.api_registry_entry list)
    ];
  let btor = make () in
  set_solver btor Cadical;
  let opts = all_options btor in
  List.iter opts ~f:(fun o -> print_s [%sexp (o : BOption.t)]);
  set_opt
    btor
    (List.find_exn opts ~f:(fun o -> String.equal o.name_short "i")).b
    1
  ;
  (*
  let tr = true_ btor in
  assert_ tr;
  let result = sat btor in
  print_stats btor;
     *)
  let sort = bool_sort btor in
  let a = var sort "a" in
  let b = var sort "b" in
  let c = var sort "c" in
  let xa = and_ a (and_ b c) in
  let xb = not c in
  assume xa;
  assume xb;
  let result = sat btor in
  print_s ([%sexp_of: solver_result] result);
  print_s [%message "" (get_failed_assumptions btor : _ array)];
  print_s [%message ""
      (failed xa : bool)
      (failed xb : bool)
  ];
  ()

let () =
  print_endline "CVC5";
  let open Smtcaml_bindings.Cvc5 in
  let s = new_Solver () in
  let bool = solver__getBooleanSort s in
  print_s [%message ""
      (sort__isInstantiated bool : bool)
      (sort__isBoolean bool : bool)
  ];
  ()
