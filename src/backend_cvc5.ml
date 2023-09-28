open! Core

module C = Smtcaml_bindings.Cvc5

module Options = struct
  type t = unit [@@deriving sexp]
end

type _ t = C.solver
                                 
module Packed = struct
  type 'i t' = 'i t
  type t = T : _ t' -> t
end

module Sort = struct
  type (_,_) t = C.sort
end

module Expr = struct
  type (_,_) t = C.term
end

module Model = struct
  type _ t = unit

  let eval_to_string _ _ _ = None
end

let create ?(options=()) () =
  let () = options in
  let t = C.make_Solver () in
  Packed.T t

let var sort name =
  C.solver__mkConst sort (Some name)

let check_current_and_get_model t : _ Smtcaml_intf.Solver_result.t =
  let result = C.solver__checkSat t in
  if C.result__isSat result
  then Satisfiable ()
  else if C.result__isUnsat result
  then Unsatisfiable
  else if C.result__isUnknown result
  then Unknown (Sexp.to_string (C.sexp_of_unknownExplanation (C.result__getUnknownExplanation result)))
  else if C.result__isNull result
  then Unknown "Null"
  else Unknown "Unknown"

module Types = struct
  type 'i instance = 'i t
  type _ model = unit
  type ('i, 's) sort = ('i, 's) Sort.t
  type ('i, 's) expr = ('i, 's) Expr.t
end

module Op_types = Smtcaml_intf.Make_op_types(Types)
