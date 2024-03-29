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


module Types = struct
  type 'i instance =
    { context : Z3.context
    ; solver : Z3.Solver.solver
    }
  type 'i model = Z3.Model.model
  type ('i, 's) sort = Z3.Sort.sort
  type ('i, 's) expr = Z3.Expr.expr
end

module Op_types = Smtcaml_intf.Make_op_types(Types)

let unsafe_context_of_native (t : Z3native.ptr) : Z3.context = Obj.magic t

let c (t : _ Types.instance) = t.context
(* let cn t : Z3native.ptr = Obj.magic t.context *)
let s (t : _ Types.instance) = t.solver

module type Expr = sig
  include Smtcaml_intf.Base_modules.Expr

  val to_native : _ t -> Z3native.ast
  val unsafe_of_native : Z3native.ast -> _ t
  val context_native : _ t -> Z3native.context
  val context : _ t -> Z3.context
end

module type Sort = sig
  include Smtcaml_intf.Base_modules.Sort

  (* val to_native : _ t -> Z3native.ast *)
  val context : _ t -> Z3.context
end

module type Util = sig
  val op : (Z3.context -> Z3.Expr.expr -> 'a) -> Z3.Expr.expr -> 'a
  val op2_nolist : (Z3.context -> Z3.Expr.expr list -> 'a) -> Z3.Expr.expr -> Z3.Expr.expr -> 'a
  val op_list : (Z3.context -> Z3.Expr.expr list -> 'a) -> Z3.Expr.expr list -> 'a
end

module rec Base : Smtcaml_intf.Backend_base
  with module Types := Types 
   and module Op_types := Op_types
   and module Options := Options
= struct
  type 'a t = 'a Types.instance


  module Packed = struct
    type 'i t' = 'i t
    type t = T : _ t' -> t
  end

  let backend_name = "z3"

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

  let var sort name = Z3.Expr.mk_const_s (Sort.context sort) name sort

  let var_anon sort = Z3.Expr.mk_fresh_const (Sort.context sort) "Xanon" sort

  let check_current_and_get_model t : _ Smtcaml_intf.Solver_result.t =
    match Z3.Solver.check (s t) [] with
    | UNSATISFIABLE -> Unsatisfiable
    | UNKNOWN -> Unknown (Z3.Solver.get_reason_unknown (s t))
    | SATISFIABLE ->
      let model = Option.value_exn (Z3.Solver.get_model (s t)) in
      Satisfiable model

end

and Util : Util
= struct
  let op f a = f (Expr.context a) a
  let op2_nolist f a b = f (Expr.context a) [a; b]
  let op_list f a = f (Expr.context (List.hd_exn a)) a
end

and Expr : Expr
  with module Types := Types
= struct

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

and Model : Smtcaml_intf.Base_modules.Model
  with module Types := Types
= struct
  type _ t = Z3.Model.model

  let eval_to_string_exn _t model expr =
    let apply_model_completion = true in
    Z3.Model.eval model expr apply_model_completion
    |> Option.value_exn
    |> Z3.Expr.to_string

  let eval_bool_exn _t model expr =
    let apply_model_completion = true in
    Z3.Model.eval model expr apply_model_completion
    |> Option.value_exn
    |> Z3.Boolean.get_bool_value
    |> function
    | Z3enums.L_FALSE -> false
    | Z3enums.L_UNDEF -> assert false
    | Z3enums.L_TRUE -> true
end

and Sort : Sort
  with module Types := Types
= struct
  type (_,_) t = Z3.Sort.sort

  let to_native (t : Z3.Sort.sort) : Z3native.ast = Obj.magic t

  let context (t : Z3.Sort.sort) =
    Z3native.context_of_ast (to_native t)
    |> unsafe_context_of_native
end

and Boolean_t : Smtcaml_intf.Boolean
  with module Types := Types
   and module Op_types := Op_types
= struct
  open Util

  let sort_boolean t = Z3.Boolean.mk_sort (c t)

  let assert_ t expr = Z3.Solver.add (s t) [ expr ]

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

  let assert_not t expr = Z3.Solver.add (s t) [ Boolean.not expr ]
end

and Bitvector_t : Smtcaml_intf.Bitvector
  with module Types := Types
   and module Op_types := Op_types
= struct
  open Util
  module Boolean = T.Boolean

  let sort_bitvector t l = Z3.BitVector.mk_sort (c t) l

  module Bv = struct
    module Set = Smtcaml_utils.Set_ops.Make(T)

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

      let int_e e i =
        int (Z3.Expr.get_sort e) i

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

      let zero ~length t =
        Z3.Expr.mk_numeral_int (c t) 0 (sort_bitvector t length)

      let zero_e t = Z3.Expr.mk_numeral_int (Expr.context t) 0 (Z3.Expr.get_sort t)

    end

    module Model = struct
      let eval_exn _t model expr =
        let apply_model_completion = true in
        Z3.Model.eval model expr apply_model_completion
        |> Option.value_exn
        |> Numeral.to_fast_bitvector
    end

    module Signed = struct
      let (==) = Boolean.eq
      let (<>) = Boolean.neq
      let (<) = op Z3.BitVector.mk_slt
      let (<=) = op Z3.BitVector.mk_sle
      let (>) = op Z3.BitVector.mk_sgt
      let (>=) = op Z3.BitVector.mk_sge
      let (+) = Bitvector_t.Bv.add
      let (-) = Bitvector_t.Bv.sub
      let (~-) = op Z3.BitVector.mk_neg
      let ( * ) = op Z3.BitVector.mk_mul
      let (/) = op Z3.BitVector.mk_sdiv
      let (mod) = op Z3.BitVector.mk_srem
    end

    module Unsigned = struct
      let (==) = Boolean.eq
      let (<>) = Boolean.neq
      let (<) = op Z3.BitVector.mk_ult
      let (<=) = op Z3.BitVector.mk_ule
      let (>) = op Z3.BitVector.mk_ugt
      let (>=) = op Z3.BitVector.mk_uge
      let (+) = Bitvector_t.Bv.add
      let (-) = Bitvector_t.Bv.sub
      let ( * ) = op Z3.BitVector.mk_mul
      let (/) = op Z3.BitVector.mk_udiv
      let (mod) = op Z3.BitVector.mk_urem
    end

    let extract ~low ~high e =
      Z3.BitVector.mk_extract (Expr.context e) high low e

    let extract_single ~bit e = extract ~low:bit ~high:bit e

    let concat a b =
      Z3.BitVector.mk_concat (Expr.context a) a b

    let concat_list l =
      let op = Z3.BitVector.mk_concat (Expr.context (List.hd_exn l)) in
      List.reduce_balanced_exn l ~f:op

    let repeat ~count e =
      Z3.BitVector.mk_repeat (Expr.context e) count e

    let broadcast_single_bit sort bit =
      let target_size = Bitvector_t.Bv.Sort.length sort in
      assert (length bit = 1);
      repeat ~count:target_size bit

    let of_bool b =
      let cn = Expr.context_native b in
      Boolean.ite b (Numeral.bit_cn cn true) (Numeral.bit_cn cn false)

    let not = op Z3.BitVector.mk_not
    let and_ = op Z3.BitVector.mk_and
    let or_ = op Z3.BitVector.mk_or
    let xor = op Z3.BitVector.mk_xor

    let and_list = List.reduce_balanced_exn ~f:and_
    let or_list = List.reduce_balanced_exn ~f:or_
    let xor_list = List.reduce_balanced_exn ~f:xor

    let add = op Z3.BitVector.mk_add
    let sub = op Z3.BitVector.mk_sub

    let inc e = add (Numeral.int_e e 1) e
    let dec e = sub e (Numeral.int_e e 1)

    let is_zero e = Boolean.eq e (Numeral.zero_e e)
    let is_not_zero e = Boolean.neq e (Numeral.zero_e e)

    let is_all_ones e = Boolean.eq e (not (Numeral.zero_e e))

    include Smtcaml_utils.Power_of_two.Make(T)

    let sign e =
      let length = length e in
      extract_single ~bit:(length - 1) e

    let parity a =
      let length = length a in
      (* No mk_redxor *)
      List.init length ~f:(fun i -> extract_single a ~bit:i)
      |> List.reduce_balanced_exn ~f:xor

    include Smtcaml_utils.Popcount.Make(T)

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
end

and Uf_t : Smtcaml_intf.Uninterpreted_function
  with module Types := Types
   and module Op_types := Op_types
= struct

  let sort_uninterpreted_function (t : _ T.t) ~domain ~codomain =
    Z3.Z3Array.mk_sort t.context domain codomain

  module Ufun = struct
    module Model = struct
      (* not tail-recursive *)
      let rec eval_to_list_exn instance model expr convert_domain convert_codomain =
        let apply_model_completion = true in
        let ev =
          Z3.Model.eval model expr apply_model_completion 
          |> Option.value_exn
        in
        let fd = Z3.Expr.get_func_decl ev in
        match Z3.FuncDecl.get_decl_kind fd with
        | OP_CONST_ARRAY -> 
          let else_val = List.hd_exn (Z3.Expr.get_args ev) in
          let else_val = convert_codomain instance model else_val in
          { Smtcaml_intf.Ufun_interp.
            values = []
          ; else_val = Some else_val
          }
        | OP_AS_ARRAY ->
          (* CR smuenzel: Need to find an example to test this *)
          raise_s
            [%message "cannot handle this interp"
                ~_:(Z3.FuncDecl.to_string fd : string)
            ]
        | OP_STORE ->
          begin match Z3.Expr.get_args ev with
            | [ store_array; store_domain; store_codomain ] ->
              let inner =
                eval_to_list_exn instance model store_array convert_domain convert_codomain
              in
              let v =
                convert_domain instance model store_domain
              , convert_codomain instance model store_codomain
              in
              { inner with
                values = v::inner.values
              }
            | _ ->
              assert false
          end
        | _ ->
          raise_s
            [%message "cannot handle this interp"
                ~_:(Z3.FuncDecl.to_string fd : string)
            ]
          (*
        let open Poly in


        Printf.eprintf "%i\n" (Z3enums.int_of_decl_kind (Z3.FuncDecl.get_decl_kind fd));
        Printf.eprintf "%s\n" (Z3.FuncDecl.to_string fd);
        assert (Z3.FuncDecl.get_decl_kind fd = Z3enums.OP_AS_ARRAY);
        assert (Z3.FuncDecl.get_num_parameters fd = 1);
        match Z3.FuncDecl.get_parameters fd with
        | array_interp :: _ ->
          let array_interp = Z3.FuncDecl.Parameter.get_func_decl array_interp in
          let interp = Option.value_exn (Z3.Model.get_func_interp model array_interp) in
          let entries = Z3.Model.FuncInterp.get_entries interp in
          List.map entries
            ~f:(fun x ->
                let args = Z3.Model.FuncInterp.FuncEntry.get_args x in
                let return = Z3.Model.FuncInterp.FuncEntry.get_value x in
                let arg = List.hd_exn args in
                convert_domain instance model arg
              , convert_codomain instance model return
              )

        | _ -> assert false
             *)
    end

    let apply a b = Z3.Z3Array.mk_select (Expr.context a) a b
  end

end

and T : Smtcaml_intf.Interface_definitions.Bitvector_uf
  with module Types = Types
   and module Op_types = Op_types
   and module Options = Options
= struct
  include Base
  include Boolean_t
  include Bitvector_t
  include Uf_t

  module Types = Types
  module Op_types = Op_types
  module Options = Options
  module Expr = Expr
  module Sort = Sort
  module Model = Model
end

include (T : module type of struct include T end
  with module Types := T.Types
   and module Op_types := T.Op_types
   and module Options := T.Options
   and module Sort := T.Sort
   and module Expr := T.Expr
   and module Model := T.Model
 )




