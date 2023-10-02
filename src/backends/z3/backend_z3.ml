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
end

let sort_boolean t = Z3.Boolean.mk_sort (c t)
let sort_bitvector t l = Z3.BitVector.mk_sort (c t) l

let assert_ t expr = Z3.Solver.add (s t) [ expr ]

let var sort name = Z3.Expr.mk_const_s (Sort.context sort) name sort

let check_current_and_get_model t : _ Smtcaml_intf.Solver_result.t =
  match Z3.Solver.check (s t) [] with
  | UNSATISFIABLE -> Unsatisfiable
  | UNKNOWN -> Unknown (Z3.Solver.get_reason_unknown (s t))
  | SATISFIABLE ->
    let model = Option.value_exn (Z3.Solver.get_model (s t)) in
    Satisfiable model

let op f a = f (Expr.context a) a

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

end

module Bv = struct
  module Sort_internal = struct
    let length = Z3.BitVector.get_size
  end

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
  end

  let of_bool b =
    let cn = Expr.context_native b in
    Boolean.ite b (Numeral.bit_cn cn true) (Numeral.bit_cn cn false)

  let not = op Z3.BitVector.mk_not
  let and_ = op Z3.BitVector.mk_and
  let or_ = op Z3.BitVector.mk_or
  let xor = op Z3.BitVector.mk_xor

  let add = op Z3.BitVector.mk_add

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
end


module Types = struct
  type 'i instance = 'i t
  type 'i model = 'i Model.t
  type ('i, 's) sort = ('i, 's) Sort.t
  type ('i, 's) expr = ('i, 's) Expr.t
end

module Op_types = Smtcaml_intf.Make_op_types(Types)
