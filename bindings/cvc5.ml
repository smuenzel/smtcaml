open! Base

type solver
type sort
type term

type uint32_t = int
type int32_t = int
type uint64_t = int

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
external solver__getBooleanSort : (* class Solver *) solver -> sort = "caml_cvc5__Solver__getBooleanSort"
external solver__getIntegerSort : (* class Solver *) solver -> sort = "caml_cvc5__Solver__getIntegerSort"
external solver__getRealSort : (* class Solver *) solver -> sort = "caml_cvc5__Solver__getRealSort"
external solver__getRegExpSort : (* class Solver *) solver -> sort = "caml_cvc5__Solver__getRegExpSort"
external solver__getRoundingModeSort : (* class Solver *) solver -> sort = "caml_cvc5__Solver__getRoundingModeSort"
external solver__getStringSort : (* class Solver *) solver -> sort = "caml_cvc5__Solver__getStringSort"
external solver__mkArraySort : (* class Solver *) solver -> sort -> sort -> sort = "caml_cvc5__Solver__mkArraySort"
external solver__mkBitVectorSort : (* class Solver *) solver -> uint32_t -> sort = "caml_cvc5__Solver__mkBitVectorSort"
external solver__mkFloatingPointSort : (* class Solver *) solver -> uint32_t -> uint32_t -> sort = "caml_cvc5__Solver__mkFloatingPointSort"
external solver__mkFiniteFieldSort : (* class Solver *) solver -> string -> sort = "caml_cvc5__Solver__mkFiniteFieldSort"
external solver__mkFunctionSort : (* class Solver *) solver -> (sort array) -> sort -> sort = "caml_cvc5__Solver__mkFunctionSort"
external solver__mkPredicateSort : (* class Solver *) solver -> (sort array) -> sort = "caml_cvc5__Solver__mkPredicateSort"
external solver__mkRecordSort : (* class Solver *) solver -> ((string*sort) array) -> sort = "caml_cvc5__Solver__mkRecordSort"
external solver__mkSetSort : (* class Solver *) solver -> sort -> sort = "caml_cvc5__Solver__mkSetSort"
external solver__mkBagSort : (* class Solver *) solver -> sort -> sort = "caml_cvc5__Solver__mkBagSort"
external solver__mkSequenceSort : (* class Solver *) solver -> sort -> sort = "caml_cvc5__Solver__mkSequenceSort"
external solver__mkUninterpretedSort : (* class Solver *) solver -> (string option) -> sort = "caml_cvc5__Solver__mkUninterpretedSort"
external solver__mkTupleSort : (* class Solver *) solver -> (sort array) -> sort = "caml_cvc5__Solver__mkTupleSort"
external solver__mkBitVector__u32_u64 : (* class Solver *) solver -> uint32_t -> uint64_t -> term = "caml_cvc5__Solver__mkBitVector__overload__u32_u64"
external solver__mkBitVector__u32_s_u32 : (* class Solver *) solver -> uint32_t -> string -> uint32_t -> term = "caml_cvc5__Solver__mkBitVector__overload__u32_s_u32"
external solver__mkTrue : (* class Solver *) solver -> term = "caml_cvc5__Solver__mkTrue"
external solver__mkFalse : (* class Solver *) solver -> term = "caml_cvc5__Solver__mkFalse"
external solver__mkBoolean : (* class Solver *) solver -> bool -> term = "caml_cvc5__Solver__mkBoolean"
external solver__mkConstArray : (* class Solver *) solver -> sort -> term -> term = "caml_cvc5__Solver__mkConstArray"
external solver__mkConst : (* class Solver *) solver -> sort -> (string option) -> term = "caml_cvc5__Solver__mkConst"
external solver__mkVar : (* class Solver *) solver -> sort -> (string option) -> term = "caml_cvc5__Solver__mkVar"
external solver__simplify : (* class Solver *) solver -> term -> term = "caml_cvc5__Solver__simplify"
external solver__setOption : (* class Solver *) solver -> string -> string -> unit = "caml_cvc5__Solver__setOption"
external sort__hasSymbol : (* class Sort *) sort -> bool = "caml_cvc5__Sort__hasSymbol"
external sort__getSymbol : (* class Sort *) sort -> string = "caml_cvc5__Sort__getSymbol"
external sort__isNull : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isNull"
external sort__isBoolean : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isBoolean"
external sort__isInteger : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isInteger"
external sort__isReal : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isReal"
external sort__isInstantiated : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isInstantiated"
(*$*)

