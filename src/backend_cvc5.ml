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

  let eval_to_string solver () term =
    Some (C.term__toString (C.solver__getValue__t solver term))
end

let create ?(options=()) () =
  let () = options in
  let t = C.new_Solver () in
  C.solver__setOption t "produce-models" "true";
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

let sort_boolean t = C.solver__getBooleanSort t
let assert_ expr = C.solver__assertFormula expr

let get_term_context (t : C.term) : C.solver = Obj.magic t

module Boolean = struct
  let eq e0 e1 = C.solver__mkTerm__kind (get_term_context e0) EQUAL [| e0; e1 |]
  let neq e0 e1 = C.solver__mkTerm__kind (get_term_context e0) DISTINCT [| e0; e1 |]
end

module Types = struct
  type 'i instance = 'i t
  type _ model = unit
  type ('i, 's) sort = ('i, 's) Sort.t
  type ('i, 's) expr = ('i, 's) Expr.t
end

module Op_types = Smtcaml_intf.Make_op_types(Types)
