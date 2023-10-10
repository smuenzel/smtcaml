open! Core

let backends : (module Smtcaml_intf.Interface_definitions.Bitvector_basic) list =
  [ (module Backend_bitwuzla)
  ; (module Backend_boolector)
  ; (module Backend_cvc5)
  ; (module Backend_z3)
  ]

module Test(Smt : Smtcaml_intf.Interface_definitions.Bitvector_basic) = struct
  let run () =
    let T t = Smt.create () in
    let s31 = Smt.sort_bitvector t 31 in
    let v7 = Smt.Bv.Numeral.int s31 7 in
    let v6 = Smt.Bv.Numeral.int s31 6 in
    let a = Smt.var_anon s31 in
    let pop = Smt.Bv.popcount ~result_bit_size:31 a in
    let m1 = Smt.Bv.dec a in
    let pop_m1 = Smt.Bv.popcount ~result_bit_size:31 m1 in
    Smt.assert_ t (Smt.Boolean.eq pop v7);
    Smt.assert_ t (Smt.Boolean.eq pop_m1 v6);
    match Smt.check_current_and_get_model t with
    | Unsatisfiable
    | Unknown _ as result ->
      print_s [%message "no model" ~_:(result : _ Smtcaml_intf.Solver_result.t)]
    | Satisfiable model ->
      print_s [%message "model"
          (Smt.Bv.Model.eval t model a : Fast_bitvector.t option)
          (Smt.Bv.Model.eval t model m1 : Fast_bitvector.t option)
          (Smt.Bv.Model.eval t model pop : Fast_bitvector.t option)
          (Smt.Bv.Model.eval t model pop_m1 : Fast_bitvector.t option)
      ]

end


let run () =
  List.iter backends
    ~f:(fun backend ->
        let (module B) = backend in
        Printf.printf "testing %s\n" B.backend_name;
        let module Test_b = Test(B) in
        Test_b.run ()
      )

let command : Command.t =
  Command.basic
    ~summary:"run advanced tests"
    [%map_open.Command
      let () = return ()
      in
      fun () ->
        run ()
    ]

let () =
  Command_unix.run command
