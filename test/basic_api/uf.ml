open! Core

module Test(Smt : Smtcaml_intf.Interface_definitions.Bitvector_uf) = struct
  let run () =
    let T t = Smt.create () in
    let s = Smt.sort_bitvector t 32 in
    let fs = Smt.sort_uninterpreted_function t ~domain:s ~codomain:s in
    let ff = Smt.var fs "f" in
    ignore ff;
    let a0 = Smt.Bv.Numeral.int s 12 in
    let a1 = Smt.Bv.Numeral.int s 23 in
    let a2 = Smt.Bv.Numeral.int s 99999 in
    let c = Smt.var s "c" in
    let sum = Smt.Bv.add a0 a1 in
    let app0 = Smt.Ufun.apply ff sum in
    let eq0 = Smt.Boolean.eq app0 a2 in
    let app1 = Smt.Ufun.apply ff sum in
    let eq1 = Smt.Boolean.eq app1 c in
    Smt.assert_ t eq0;
    Smt.assert_ t eq1;
    match Smt.check_current_and_get_model t with
    | Unsatisfiable
    | Unknown _ as result ->
      print_s [%message "no model" ~_:(result : _ Smtcaml_intf.Solver_result.t)]
    | Satisfiable model ->
      print_s [%message "model"
          (Smt.Model.eval_to_string_exn t model c : string)
          (Smt.Bv.Model.eval_exn t model c : Fast_bitvector.t)
          (Smt.Bv.Model.eval_exn t model sum : Fast_bitvector.t)
          (Smt.Bv.Model.eval_exn t model app0 : Fast_bitvector.t)
          (Smt.Bv.Model.eval_exn t model app1 : Fast_bitvector.t)
      ]

end

let%expect_test "bitwuzla" =
  let module T = Test(Backend_bitwuzla) in
  T.run ();
  [%expect {|
    (model
     ("Smt.Model.eval_to_string_exn t model c"
      #b00000000000000011000011010011111)
     ("Smt.Bv.Model.eval_exn t model c" 00000000000000011000011010011111)
     ("Smt.Bv.Model.eval_exn t model sum" 00000000000000000000000000100011)
     ("Smt.Bv.Model.eval_exn t model app0" 00000000000000011000011010011111)
     ("Smt.Bv.Model.eval_exn t model app1" 00000000000000011000011010011111)) |}]

let%expect_test "boolector" =
  let module T = Test(Backend_boolector) in
  T.run ();
  [%expect {|
    (model
     ("Smt.Model.eval_to_string_exn t model c" 00000000000000011000011010011111)
     ("Smt.Bv.Model.eval_exn t model c" 00000000000000011000011010011111)
     ("Smt.Bv.Model.eval_exn t model sum" 00000000000000000000000000100011)
     ("Smt.Bv.Model.eval_exn t model app0" 00000000000000011000011010011111)
     ("Smt.Bv.Model.eval_exn t model app1" 00000000000000011000011010011111)) |}]

let%expect_test "cvc5" =
  let module T = Test(Backend_cvc5) in
  T.run ();
  [%expect {|
    (model
     ("Smt.Model.eval_to_string_exn t model c"
      #b00000000000000011000011010011111)
     ("Smt.Bv.Model.eval_exn t model c" 00000000000000011000011010011111)
     ("Smt.Bv.Model.eval_exn t model sum" 00000000000000000000000000100011)
     ("Smt.Bv.Model.eval_exn t model app0" 00000000000000011000011010011111)
     ("Smt.Bv.Model.eval_exn t model app1" 00000000000000011000011010011111)) |}]

let%expect_test "z3" =
  let module T = Test(Backend_z3) in
  T.run ();
  [%expect {|
    (model ("Smt.Model.eval_to_string_exn t model c" #x0001869f)
     ("Smt.Bv.Model.eval_exn t model c" 00000000000000011000011010011111)
     ("Smt.Bv.Model.eval_exn t model sum" 00000000000000000000000000100011)
     ("Smt.Bv.Model.eval_exn t model app0" 00000000000000011000011010011111)
     ("Smt.Bv.Model.eval_exn t model app1" 00000000000000011000011010011111)) |}]
