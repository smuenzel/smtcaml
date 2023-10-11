open! Core

module Test_const(Smt : Smtcaml_intf.Interface_definitions.Bitvector_uf) = struct
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
      let fun_model =
        Smt.Ufun.Model.eval_to_list_exn t model ff
          Smt.Bv.Model.eval_exn
          Smt.Bv.Model.eval_exn
      in
      print_s [%message "model"
        (*
          (Smt.Model.eval_to_string_exn t model c : string)
          (Smt.Bv.Model.eval_exn t model c : Fast_bitvector.t)
          (Smt.Bv.Model.eval_exn t model sum : Fast_bitvector.t)
          (Smt.Bv.Model.eval_exn t model app0 : Fast_bitvector.t)
          (Smt.Bv.Model.eval_exn t model app1 : Fast_bitvector.t)
           *)
          (fun_model
           : (Fast_bitvector.t, Fast_bitvector.t) Smtcaml_intf.Ufun_interp.t
          )
      ]

end

module Test_store(Smt : Smtcaml_intf.Interface_definitions.Bitvector_uf) = struct
  let run () =
    let T t = Smt.create () in
    let s = Smt.sort_bitvector t 32 in
    let fs = Smt.sort_uninterpreted_function t ~domain:s ~codomain:s in
    let ff = Smt.var fs "f" in
    ignore ff;
    let da = Smt.var_anon s in
    let db = Smt.var_anon s in
    let dar = Smt.var_anon s in
    let dbr = Smt.var_anon s in
    Smt.assert_ t (Smt.Boolean.neq dar dbr);
    Smt.assert_ t (Smt.Boolean.neq da db);
    Smt.assert_ t (Smt.Boolean.eq dar (Smt.Ufun.apply ff da));
    Smt.assert_ t (Smt.Boolean.eq dbr (Smt.Ufun.apply ff db));
    match Smt.check_current_and_get_model t with
    | Unsatisfiable
    | Unknown _ as result ->
      print_s [%message "no model" ~_:(result : _ Smtcaml_intf.Solver_result.t)]
    | Satisfiable model ->
      let fun_model =
        Smt.Ufun.Model.eval_to_list_exn t model ff
          Smt.Bv.Model.eval_exn
          Smt.Bv.Model.eval_exn
      in
      print_s [%message "model"
          (fun_model
           : (Fast_bitvector.t, Fast_bitvector.t) Smtcaml_intf.Ufun_interp.t
          )
      ]

end

let%expect_test "bitwuzla" =
  let module T1 = Test_const(Backend_bitwuzla) in
  T1.run ();
  [%expect {|
    (model
     (fun_model
      ((values
        ((00000000000000000000000000100011 00000000000000011000011010011111)))
       (else_val (00000000000000000000000000000000))))) |}]
  ;
  let module T2 = Test_store(Backend_bitwuzla) in
  T2.run ();
  [%expect {|
    (model
     (fun_model
      ((values
        ((11111111111111111111111111111111 11111111111111111111111111111111)
         (11111111111111111111111111111110 11111111111111111111111111111110)))
       (else_val (00000000000000000000000000000000))))) |}]
  ;
  ()

let%expect_test "boolector" =
  let module T1 = Test_const(Backend_boolector) in
  T1.run ();
  [%expect {|
    (model
     (fun_model
      ((values
        ((00000000000000000000000000100011 00000000000000011000011010011111)))
       (else_val ())))) |}]
  ;
  let module T2 = Test_store(Backend_boolector) in
  T2.run ();
  [%expect {|
    (model
     (fun_model
      ((values
        ((11111111111111111111111111111111 11111111111111111111111111111111)
         (11111111111111111111111111111110 11111111111111111111111111111110)))
       (else_val ())))) |}]
  ;
  ()

let%expect_test "cvc5" =
  let module T1 = Test_const(Backend_cvc5) in
  T1.run ();
  [%expect {|
    (model
     (fun_model ((values ()) (else_val (00000000000000011000011010011111))))) |}]
  ;
  let module T2 = Test_store(Backend_cvc5) in
  T2.run ();
  [%expect {|
    (model
     (fun_model
      ((values
        ((11111111111111111111111111111111 11111111111111111111111111111111)))
       (else_val (11111111111111111111111111111110))))) |}]
  ;
  ()

let%expect_test "z3" =
  let module T1 = Test_const(Backend_z3) in
  T1.run ();
  [%expect {|
    (model
     (fun_model ((values ()) (else_val (00000000000000011000011010011111))))) |}]
  ;
  let module T2 = Test_store(Backend_z3) in
  T2.run ();
  [%expect {|
    (model
     (fun_model
      ((values
        ((00000000000000000000000000000001 00000000000000000000000000000001)))
       (else_val (00000000000000000000000000000000))))) |}]
  ;
  ()
