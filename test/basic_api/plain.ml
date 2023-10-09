open! Core

module Test(Smt : Smtcaml_intf.Interface_definitions.Bitvector_basic) = struct
  let run () =
    let T t = Smt.create () in
    let s = Smt.sort_bitvector t 32 in
    let a = Smt.Bv.Numeral.int s 12 in
    let b = Smt.Bv.Numeral.int s 99999 in
    let c = Smt.var s "c" in
    let ac = Smt.Bv.add a c in
    let eq = Smt.Boolean.eq ac b in
    Smt.assert_ t eq;
    match Smt.check_current_and_get_model t with
    | Unsatisfiable
    | Unknown _ as result ->
      print_s [%message "no model" ~_:(result : _ Smtcaml_intf.Solver_result.t)]
    | Satisfiable model ->
      print_s [%message "model"
          (Smt.Model.eval_to_string t model c : string option)
          (Smt.Bv.Model.eval t model c : Fast_bitvector.t option)
      ]

end

let%expect_test "bitwuzla" =
  let module T = Test(Backend_bitwuzla) in
  T.run ();
  [%expect {|
    (model
     ("Smt.Model.eval_to_string t model c" (#b00000000000000011000011010010011))
     ("Smt.Bv.Model.eval t model c" (00000000000000011000011010010011))) |}]

let%expect_test "boolector" =
  let module T = Test(Backend_boolector) in
  T.run ();
  [%expect {|
    (model
     ("Smt.Model.eval_to_string t model c" (00000000000000011000011010010011))
     ("Smt.Bv.Model.eval t model c" (00000000000000011000011010010011))) |}]

let%expect_test "cvc5" =
  let module T = Test(Backend_cvc5) in
  T.run ();
  [%expect {|
    (model
     ("Smt.Model.eval_to_string t model c" (#b00000000000000011000011010010011))
     ("Smt.Bv.Model.eval t model c" (00000000000000011000011010010011))) |}]

let%expect_test "z3" =
  let module T = Test(Backend_z3) in
  T.run ();
  [%expect {|
    (model ("Smt.Model.eval_to_string t model c" (#x00018693))
     ("Smt.Bv.Model.eval t model c" (00000000000000011000011010010011))) |}]
