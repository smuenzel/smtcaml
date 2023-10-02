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

  let eval_bitvector solver model term =
    eval_to_string solver model term
    |> Option.map ~f:(String.chop_prefix_exn ~prefix:"#b")
    |> Option.map ~f:Fast_bitvector.Little_endian.of_string
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
let sort_bitvector t l = C.solver__mkBitVectorSort t l

let assert_ _ expr = C.solver__assertFormula expr

let get_term_context (t : C.term) : C.solver = Obj.magic t
let get_sort_context (t : C.sort) : C.solver = Obj.magic t

let term_op op ar = C.solver__mkTerm__kind (get_term_context ar.(0)) op ar

let op1 o e0 = term_op o [| e0 |]
let op2 o e0 e1 = term_op o [| e0; e1 |]
let op3 o e0 e1 e2 = term_op o [| e0; e1; e2 |]

module Boolean = struct
  module Numeral = struct
    let bool t b = C.solver__mkBoolean t b
    let true_ t = bool t true
    let false_ t = bool t false
  end

  let eq = op2 EQUAL
  let neq = op2 DISTINCT
  let ite = op3 ITE
end

module Bv = struct
  module Sort = struct
    let length sort = C.sort__getBitVectorSize sort
  end

  module Numeral = struct
    let int sort i =
      C.solver__mkBitVector__u32_u64
        (get_sort_context sort)
        (C.sort__getBitVectorSize sort)
        i

    let fast_bitvector sort bv =
      assert (Sort.length sort = Fast_bitvector.length bv);
      C.solver__mkBitVector__u32_s_u32
        (get_sort_context sort)
        (Fast_bitvector.length bv)
        (Fast_bitvector.Little_endian.to_string bv)
        2
  end

  let of_bool e0 =
    let t = get_term_context e0 in
    Boolean.ite e0
      (C.solver__mkBitVector__u32_u64 t 1 1)
      (C.solver__mkBitVector__u32_u64 t 1 0)

  let not = op1 BITVECTOR_NOT
  let and_ = op2 BITVECTOR_AND
  let or_ = op2 BITVECTOR_OR
  let xor = op2 BITVECTOR_XOR

  let add = op2 BITVECTOR_ADD
end

module Types = struct
  type 'i instance = 'i t
  type _ model = unit
  type ('i, 's) sort = ('i, 's) Sort.t
  type ('i, 's) expr = ('i, 's) Expr.t
end

module Op_types = Smtcaml_intf.Make_op_types(Types)