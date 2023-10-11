open! Core

module Test(Smt : Smtcaml_intf.Interface_definitions.Bitvector_basic) = struct
  module M = Smtcaml_utils.Mux.Make(Smt)

  let run () =
    let T t = Smt.create () in
    let s = Smt.sort_bitvector t 32 in
    let in_list = List.init 5 ~f:(fun _ -> Smt.var_anon s) in
    let mux = M.create t in_list in
    Smt.assert_ t mux.constraints;
    List.iteri in_list ~f:(fun i input ->
        Smt.Boolean.eq input (Smt.Bv.Numeral.int_e input (i + 1))
        |> Smt.assert_ t
      );
    match Smt.check_current_and_get_model t with
    | Unsatisfiable
    | Unknown _ as result ->
      print_s [%message "no model" ~_:(result : _ Smtcaml_intf.Solver_result.t)]
    | Satisfiable model ->
      print_s [%message "model"
        (Smt.Bv.Model.eval_exn t model mux.input_selector : Fast_bitvector.t)
        (Smt.Bv.Model.eval_exn t model mux.output : Fast_bitvector.t)
      ]

end

let%expect_test "bitwuzla" =
  let module T = Test(Backend_bitwuzla) in
  T.run ();
  [%expect {|
    (model ("Smt.Bv.Model.eval_exn t model mux.input_selector" 10000)
     ("Smt.Bv.Model.eval_exn t model mux.output"
      00000000000000000000000000000101)) |}]

let%expect_test "boolector" =
  let module T = Test(Backend_boolector) in
  T.run ();
  [%expect {|
    (model ("Smt.Bv.Model.eval_exn t model mux.input_selector" 10000)
     ("Smt.Bv.Model.eval_exn t model mux.output"
      00000000000000000000000000000101)) |}]

let%expect_test "cvc5" =
  let module T = Test(Backend_cvc5) in
  T.run ();
  [%expect {|
    (model ("Smt.Bv.Model.eval_exn t model mux.input_selector" 10000)
     ("Smt.Bv.Model.eval_exn t model mux.output"
      00000000000000000000000000000101)) |}]

let%expect_test "z3" =
  let module T = Test(Backend_z3) in
  T.run ();
  [%expect {|
    (model ("Smt.Bv.Model.eval_exn t model mux.input_selector" 10000)
     ("Smt.Bv.Model.eval_exn t model mux.output"
      00000000000000000000000000000101)) |}]
