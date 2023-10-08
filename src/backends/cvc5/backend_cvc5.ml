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

  let eval_bool solver () term =
    Some (C.term__getBooleanValue (C.solver__getValue__t solver term))
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

let sort_uninterpreted_function t ~domain ~codomain =
  C.solver__mkFunctionSort t [| domain |] codomain

let assert_ _ expr = C.solver__assertFormula expr

let get_term_context (t : C.term) : C.solver = Obj.magic t
let get_sort_context (t : C.sort) : C.solver = Obj.magic t

let term_op op ar = C.solver__mkTerm__kind (get_term_context ar.(0)) op ar

let term_complex_op op op_params ar =
  let t = get_term_context ar.(0) in
  let op = C.solver__mkOp__kv t op op_params in
  C.solver__mkTerm__op t op ar

let op1 o e0 = term_op o [| e0 |]
let op2 o e0 e1 = term_op o [| e0; e1 |]
let op3 o e0 e1 e2 = term_op o [| e0; e1; e2 |]

let op_list o elist = term_op o (Array.of_list elist)

module Boolean = struct
  module Numeral = struct
    let bool t b = C.solver__mkBoolean t b
    let true_ t = bool t true
    let false_ t = bool t false
  end

  let not = op1 NOT
  let and_ = op2 AND
  let or_ = op2 OR
  let and_list = op_list AND
  let or_list = op_list OR
  let eq = op2 EQUAL
  let neq = op2 DISTINCT
  let ite = op3 ITE
  let implies = op2 IMPLIES
  let xor = op2 XOR
  let iff a b = not (xor a b) 

  let distinct list = term_op DISTINCT (Array.of_list list)
end

module Bv = struct
  module Sort = struct
    let length sort = C.sort__getBitVectorSize sort
  end

  let length e0 = Sort.length (C.term__getSort e0)

  module Numeral = struct
    let int sort i =
      C.solver__mkBitVector__u32_u64
        (get_sort_context sort)
        (C.sort__getBitVectorSize sort)
        (Int64.of_int i)

    let fast_bitvector sort bv =
      assert (Sort.length sort = Fast_bitvector.length bv);
      C.solver__mkBitVector__u32_s_u32
        (get_sort_context sort)
        (Fast_bitvector.length bv)
        (Fast_bitvector.Little_endian.to_string bv)
        2

    let zero ~length t = C.solver__mkBitVector__u32_u64 t length 0L

    let zero_e e0 =
      C.solver__mkBitVector__u32_u64 (get_term_context e0) (length e0) 0L
  end

  let extract ~low ~high e =
    term_complex_op BITVECTOR_EXTRACT [| high; low |] [| e |]

  let extract_single ~bit e = extract ~low:bit ~high:bit e

  let concat = op2 BITVECTOR_CONCAT

  let zero_extend ~extra_zeros e =
    term_complex_op BITVECTOR_ZERO_EXTEND [| extra_zeros |] [| e |]

  let sign_extend ~extra_bits e =
    term_complex_op BITVECTOR_SIGN_EXTEND [| extra_bits |] [| e |]

  let of_bool e0 =
    let t = get_term_context e0 in
    Boolean.ite e0
      (C.solver__mkBitVector__u32_u64 t 1 1L)
      (C.solver__mkBitVector__u32_u64 t 1 0L)

  let not = op1 BITVECTOR_NOT
  let and_ = op2 BITVECTOR_AND
  let or_ = op2 BITVECTOR_OR
  let xor = op2 BITVECTOR_XOR

  let add = op2 BITVECTOR_ADD
  let sub = op2 BITVECTOR_SUB

  let is_zero e = Boolean.eq (Numeral.zero_e e) e
  let is_not_zero e = Boolean.neq (Numeral.zero_e e) e
  let is_all_ones e = Boolean.eq (not (Numeral.zero_e e)) e 

  let sign e =
    let length = length e in
    extract_single ~bit:(length - 1) e

  let parity a =
    let length = length a in
    (* No mk_redxor *)
    List.init length ~f:(fun i -> extract_single a ~bit:i)
    |> List.reduce_balanced_exn ~f:xor

  let is_add_overflow ~signed e0 e1 =
    match signed with
    | true -> op2 BITVECTOR_SADDO e0 e1
    | false -> op2 BITVECTOR_UADDO e0 e1

  let is_add_underflow _ _ = assert false
  let is_sub_overflow _ _ = assert false

  let is_sub_underflow ~signed e0 e1 =
    match signed with
    | true -> op2 BITVECTOR_SSUBO e0 e1
    | false -> op2 BITVECTOR_USUBO e0 e1

  let shift_left ~count e =
    op2 BITVECTOR_SHL e count

  let shift_right_logical ~count e =
    op2 BITVECTOR_LSHR e count

  let shift_right_arithmetic ~count e =
    op2 BITVECTOR_ASHR e count
end

module Ufun = struct
  let apply a b = op2 APPLY_UF a b
end

module Types = struct
  type 'i instance = 'i t
  type _ model = unit
  type ('i, 's) sort = ('i, 's) Sort.t
  type ('i, 's) expr = ('i, 's) Expr.t
end

module Op_types = Smtcaml_intf.Make_op_types(Types)
