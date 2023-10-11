open! Core

module Test(Smt : Smtcaml_intf.Interface_definitions.Bitvector_basic) = struct
  module A = Smtcaml_utils.Add_sub_over_under_flow.Make(Smt)

  let run kind =
    let T t = Smt.create () in
    let s = Smt.sort_bitvector t 32 in
    let a = Smt.var s "a" in
    let b = Smt.var s "b" in
    let fnative, fnonnative =
      match kind with
      | `Add_over signed -> Smt.Bv.is_add_overflow ~signed, A.is_add_overflow ~signed
      | `Sub_over -> Smt.Bv.is_sub_overflow, A.is_sub_overflow
      | `Add_under -> Smt.Bv.is_add_underflow, A.is_add_underflow
      | `Sub_under signed -> Smt.Bv.is_sub_underflow ~signed, A.is_sub_underflow ~signed
    in
    let ov_native = fnative a b in
    let ov_nonnative = fnonnative a b in
    let sum = Smt.Bv.add a b in
    let diff = Smt.Bv.sub a b in
    let ne = Smt.Boolean.neq ov_native ov_nonnative in
    Smt.assert_ t ne;
    match Smt.check_current_and_get_model t with
    | Unsatisfiable
    | Unknown _ as result ->
      print_s [%message "no model" ~_:(result : _ Smtcaml_intf.Solver_result.t)]
    | Satisfiable model ->
      print_s [%message "model"
          (Smt.Bv.Model.eval_exn t model a : Fast_bitvector.t)
          (Smt.Bv.Model.eval_exn t model b : Fast_bitvector.t)
          (Smt.Model.eval_bool_exn t model ov_native : bool)
          (Smt.Model.eval_bool_exn t model ov_nonnative : bool)
          (Smt.Bv.Model.eval_exn t model sum : Fast_bitvector.t)
          (Smt.Bv.Model.eval_exn t model diff : Fast_bitvector.t)
      ]

end

let%expect_test "z3" =
  let module T = Test(Backend_z3) in
  T.run (`Add_over true);
  [%expect {|
    ("no model" Unsatisfiable) |}];
  T.run (`Add_over false);
  [%expect {|
    ("no model" Unsatisfiable) |}];
  T.run (`Add_under);
  [%expect {|
    ("no model" Unsatisfiable) |}];
  T.run (`Sub_over);
  [%expect {|
    ("no model" Unsatisfiable) |}];
  T.run (`Sub_under true);
  [%expect {|
    ("no model" Unsatisfiable) |}];
  T.run (`Sub_under false);
  [%expect {|
    ("no model" Unsatisfiable) |}];
  ()
