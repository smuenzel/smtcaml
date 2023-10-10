open! Core

module type TS = Smtcaml_intf.Interface_definitions.Bitvector_basic
type ts = (module TS)

module Test(Smt : TS) = struct
  let run () =
    let T t = Smt.create () in
    let s31 = Smt.sort_bitvector t 31 in
    let a = Smt.var_anon s31 in
    let ax = Smt.Bv.zero_extend ~extra_zeros:3 a in
    let az =
      Smt.Bv.concat_list
        [ Smt.Bv.Numeral.zero ~length:1 t
        ; Smt.Bv.Numeral.zero ~length:2 t
        ; a
        ]
    in
    Smt.(assert_not t (Boolean.eq ax az));
    match Smt.check_current_and_get_model t with
    | Unsatisfiable
    | Unknown _ as result ->
      print_s [%message "no model" ~_:(result : _ Smtcaml_intf.Solver_result.t)]
    | Satisfiable model ->
      print_s [%message "model"
          (Smt.Model.eval_to_string t model a : string option)
          (Smt.Model.eval_to_string t model ax : string option)
      ]

end

let%expect_test "bitwuzla" =
  let module T = Test(Backend_bitwuzla) in
  T.run ();
  [%expect {|
    ("no model" Unsatisfiable) |}]

let%expect_test "boolector" =
  let module T = Test(Backend_boolector) in
  T.run ();
  [%expect {|
    ("no model" Unsatisfiable) |}]

let%expect_test "cvc5" =
  let module T = Test(Backend_cvc5) in
  T.run ();
  [%expect {|
    ("no model" Unsatisfiable) |}]

let%expect_test "z3" =
  let module T = Test(Backend_z3) in
  T.run ();
  [%expect {|
    ("no model" Unsatisfiable) |}]
