open! Core

module Options = struct
  type t = 
    { parallel : bool
    ; model : bool
    ; proof : bool
    } [@@deriving sexp]

  let default = 
    { parallel = true
    ; model = true
    ; proof = false
    }
end

type _ t =
  { context : Z3.context
  ; solver : Z3.Solver.solver
  }

let unsafe_context_of_native (t : Z3native.ptr) : Z3.context = Obj.magic t

let c t = t.context
(* let cn t : Z3native.ptr = Obj.magic t.context *)
let s t = t.solver

module Packed = struct
  type 'i t' = 'i t
  type t = T : _ t' -> t
end

let create
    ?(options = Options.default)
    ()
  =
  let { Options.
        parallel
      ; model
      ; proof
      } = options
  in
  Z3.set_global_param "parallel.enable" (Bool.to_string parallel);
  let context =
    Z3.mk_context 
      [ "model", Bool.to_string model
      ; "proof", Bool.to_string proof 
      ]
  in
  let solver =
    Z3.Solver.mk_simple_solver context
  in
  Packed.T
    { context
    ; solver
    }

module Sort = struct
  type (_,_) t = Z3.Sort.sort

  let to_native (t : Z3.Sort.sort) : Z3native.ast = Obj.magic t

  let context (t : Z3.Sort.sort) =
    Z3native.context_of_ast (to_native t)
    |> unsafe_context_of_native
end

module Expr = struct
  type (_,_) t = Z3.Expr.expr

  let to_native (t : Z3.Expr.expr) : Z3native.ast = Obj.magic t

  let unsafe_of_native (ast : Z3native.ast) : Z3.Expr.expr = Obj.magic ast

  let context_native (t : Z3.Expr.expr) =
    Z3native.context_of_ast (to_native t)

  let context (t : Z3.Expr.expr) =
    context_native t
    |> unsafe_context_of_native

  let sort (t : Z3.Expr.expr) =
    Z3.Expr.get_sort t
end

let sort_boolean t = Z3.Boolean.mk_sort (c t)
let sort_bitvector t l = Z3.BitVector.mk_sort (c t) l

let sort_uninterpreted_function t ~domain ~codomain =
  Z3.Z3Array.mk_sort t.context domain codomain

let assert_ t expr = Z3.Solver.add (s t) [ expr ]

let var sort name = Z3.Expr.mk_const_s (Sort.context sort) name sort

let var_anon sort = Z3.Expr.mk_fresh_const (Sort.context sort) "Xanon" sort

let check_current_and_get_model t : _ Smtcaml_intf.Solver_result.t =
  match Z3.Solver.check (s t) [] with
  | UNSATISFIABLE -> Unsatisfiable
  | UNKNOWN -> Unknown (Z3.Solver.get_reason_unknown (s t))
  | SATISFIABLE ->
    let model = Option.value_exn (Z3.Solver.get_model (s t)) in
    Satisfiable model

let op f a = f (Expr.context a) a
let op2_nolist f a b = f (Expr.context a) [a; b]
let op_list f a = f (Expr.context (List.hd_exn a)) a

module Boolean = struct
  module Numeral = struct
    let true_ t = Z3.Boolean.mk_true (c t)
    let false_ t = Z3.Boolean.mk_false (c t)
    let bool t bool = Z3.Boolean.mk_val (c t) bool
  end

  let not = op Z3.Boolean.mk_not
  let eq = op Z3.Boolean.mk_eq
  let neq a b = not (eq a b)
  let ite = op Z3.Boolean.mk_ite
  let implies = op Z3.Boolean.mk_implies
  let iff = op Z3.Boolean.mk_iff

  let and_ = op2_nolist Z3.Boolean.mk_and
  let or_ = op2_nolist Z3.Boolean.mk_or

  let and_list = op_list Z3.Boolean.mk_and
  let or_list = op_list Z3.Boolean.mk_or

  let distinct = op_list Z3.Boolean.mk_distinct
end

module Bv = struct
  module Sort_internal = struct
    let length = Z3.BitVector.get_size
  end

  let length t = Sort_internal.length (Z3.Expr.get_sort t)

  module Numeral = struct
    let bit_cn cn i =
      Z3native.mk_bv_numeral cn 1 [ i ]
      |> Expr.unsafe_of_native

    (* let bit t i = bit_cn (cn t) i *)

    let int sort i =
      Z3.Expr.mk_numeral_int (Sort.context sort) i sort

    let fast_bitvector sort bv =
      assert (Sort_internal.length sort = Fast_bitvector.length bv);
      Z3.Expr.mk_numeral_string
        (Sort.context sort)
        ("#b" ^ (Fast_bitvector.Little_endian.to_string bv))
        sort

    let to_fast_bitvector t =
      if Stdlib.not (Z3.Expr.is_numeral t)
      then raise_s [%message "not a numeral"];
      let length = Sort_internal.length (Z3.Expr.get_sort t) in
      let ctx = Expr.context_native t in
      let short_string =
        Z3native.get_numeral_binary_string
          ctx
          (Expr.to_native t)
      in
      let bv = Fast_bitvector.create ~length in
      let short_string_length = String.length short_string in
      String.iteri short_string
        ~f:(fun i c ->
            match c with
            | '0' -> ()
            | '1' -> Fast_bitvector.set bv (length - ((length - short_string_length) + i + 1))
            | _ -> assert false
          )
      ;
      bv
      (*
      String.init (length - String.length short_string) ~f:(Fn.const '0')
      ^ short_string
         *)

    let zero ~length t =
      Z3.Expr.mk_numeral_int (c t) 0 (sort_bitvector t length)

    let zero_e t = Z3.Expr.mk_numeral_int (Expr.context t) 0 (Z3.Expr.get_sort t)

  end

  let extract ~low ~high e =
    Z3.BitVector.mk_extract (Expr.context e) high low e

  let extract_single ~bit e = extract ~low:bit ~high:bit e

  let concat a b =
    Z3.BitVector.mk_concat (Expr.context a) a b

  let of_bool b =
    let cn = Expr.context_native b in
    Boolean.ite b (Numeral.bit_cn cn true) (Numeral.bit_cn cn false)

  let not = op Z3.BitVector.mk_not
  let and_ = op Z3.BitVector.mk_and
  let or_ = op Z3.BitVector.mk_or
  let xor = op Z3.BitVector.mk_xor

  let add = op Z3.BitVector.mk_add
  let sub = op Z3.BitVector.mk_sub

  let is_zero e = Boolean.eq e (Numeral.zero_e e)
  let is_not_zero e = Boolean.neq e (Numeral.zero_e e)

  let is_all_ones e = Boolean.eq e (not (Numeral.zero_e e))

  let sign e =
    let length = length e in
    extract_single ~bit:(length - 1) e

  let parity a =
    let length = length a in
    (* No mk_redxor *)
    List.init length ~f:(fun i -> extract_single a ~bit:i)
    |> List.reduce_balanced_exn ~f:xor

  let is_add_overflow ~signed a b =
    let ctx = Expr.context a in
    Z3.BitVector.mk_add_no_overflow ctx a b signed
    |> Boolean.not

  let is_add_underflow a b =
    let ctx = Expr.context a in
    Z3.BitVector.mk_add_no_underflow ctx a b
    |> Boolean.not

  let is_sub_underflow ~signed a b =
    let ctx = Expr.context a in
    Z3.BitVector.mk_sub_no_underflow ctx a b signed
    |> Boolean.not

  let is_sub_overflow a b =
    let ctx = Expr.context a in
    Z3.BitVector.mk_sub_no_overflow ctx a b
    |> Boolean.not

  let shift_left ~count e =
    Z3.BitVector.mk_shl (Expr.context e) e count

  let shift_right_logical ~count e =
    Z3.BitVector.mk_lshr (Expr.context e) e count

  let shift_right_arithmetic ~count e =
    Z3.BitVector.mk_ashr (Expr.context e) e count

  let zero_extend ~extra_zeros e =
    Z3.BitVector.mk_zero_ext (Expr.context e) extra_zeros e

  let sign_extend ~extra_bits e =
    Z3.BitVector.mk_sign_ext (Expr.context e) extra_bits e

  module Sort = Sort_internal
end

module Model = struct
  type _ t = Z3.Model.model

  let eval_to_string _t model expr =
    let apply_model_completion = true in
    Z3.Model.eval model expr apply_model_completion
    |> Option.map ~f:Z3.Expr.to_string

  let eval_bitvector _t model expr =
    let apply_model_completion = true in
    Z3.Model.eval model expr apply_model_completion
    |> Option.map ~f:Bv.Numeral.to_fast_bitvector

  let eval_bool _t model expr =
    let apply_model_completion = true in
    Z3.Model.eval model expr apply_model_completion
    |> Option.map ~f:Z3.Boolean.get_bool_value
    |> Option.map ~f:(function
        | Z3enums.L_FALSE -> false
        | Z3enums.L_UNDEF -> assert false
        | Z3enums.L_TRUE -> true
      )
end

module Ufun = struct
  let apply a b = Z3.Z3Array.mk_select (Expr.context a) a b
end

module Types = struct
  type 'i instance = 'i t
  type 'i model = 'i Model.t
  type ('i, 's) sort = ('i, 's) Sort.t
  type ('i, 's) expr = ('i, 's) Expr.t
end

module Op_types = Smtcaml_intf.Make_op_types(Types)
