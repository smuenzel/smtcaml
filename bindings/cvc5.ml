open! Base

type solver
type sort

type uint32_t = int
type int32_t = int

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
external make_Solver : unit -> solver = "caml_cvc5__Solver__operator_new"
external solver__getBooleanSort : solver -> sort = "caml_cvc5__Solver__getBooleanSort"
external solver__getIntegerSort : solver -> sort = "caml_cvc5__Solver__getIntegerSort"
external solver__getRealSort : solver -> sort = "caml_cvc5__Solver__getRealSort"
external solver__getRegExpSort : solver -> sort = "caml_cvc5__Solver__getRegExpSort"
external solver__getRoundingModeSort : solver -> sort = "caml_cvc5__Solver__getRoundingModeSort"
external solver__getStringSort : solver -> sort = "caml_cvc5__Solver__getStringSort"
external solver__mkArraySort : solver -> sort -> sort -> sort = "caml_cvc5__Solver__mkArraySort"
external solver__mkBitVectorSort : solver -> uint32_t -> sort = "caml_cvc5__Solver__mkBitVectorSort"
external solver__mkFloatingPointSort : solver -> uint32_t -> uint32_t -> sort = "caml_cvc5__Solver__mkFloatingPointSort"
external solver__mkFiniteFieldSort : solver -> string -> sort = "caml_cvc5__Solver__mkFiniteFieldSort"
external solver__mkFunctionSort : solver -> sort array -> sort -> sort = "caml_cvc5__Solver__mkFunctionSort"
external solver__setOption : solver -> string -> string -> unit = "caml_cvc5__Solver__setOption"
external sort__hasSymbol : sort -> bool = "caml_cvc5__Sort__hasSymbol"
external sort__getSymbol : sort -> string = "caml_cvc5__Sort__getSymbol"
external sort__isNull : sort -> bool = "caml_cvc5__Sort__isNull"
external sort__isBoolean : sort -> bool = "caml_cvc5__Sort__isBoolean"
external sort__isInteger : sort -> bool = "caml_cvc5__Sort__isInteger"
external sort__isReal : sort -> bool = "caml_cvc5__Sort__isReal"
external sort__isInstantiated : sort -> bool = "caml_cvc5__Sort__isInstantiated"
(*$*)

