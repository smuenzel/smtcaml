open! Base

type solver
type sort

(*$ open! Core
  open Cppcaml_lib
  (* dune cinaps currently doesn't support adding link flags, and we need -linkall,
     this is a workaround *)
  external unit_x : unit -> unit = "caml_cvc5_unit"

  let () =
    let modify s =
      Option.value ~default:s (String.chop_prefix ~prefix:"cvc5_" s)
      |> filter_keyword
    in
    emit_api ()
      ~modify
  *)
external solver__setOption : solver -> string -> string -> unit = "caml_cvc5__Solver__setOption"
external solver__getBooleanSort : solver -> sort = "caml_cvc5__Solver__getBooleanSort"
external make_Solver : unit -> solver = "caml_cvc5__Solver__operator_new"
(*$*)

