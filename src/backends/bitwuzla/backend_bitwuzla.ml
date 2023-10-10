open! Core

module B = Bitwuzla_cxx

module Options = struct
  type t = B.Options.t

  module Bv_solver = struct
    type t = B.Options.bv_solver =
      | Bitblast
      | Preprop
      | Prop
    [@@deriving sexp, enumerate]
  end

  module Sat_solver = struct
    type t = B.Options.sat_solver =
      | Cadical
      | Cms
      | Kissat
    [@@deriving sexp, enumerate]
  end

  module Prop_path_sel = struct
    type t = B.Options.prop_path_sel =
      | Essential
      | Random
    [@@deriving sexp, enumerate]
  end

  module Key = struct
    type 'k t = 'k B.Options.key =
      | Log_level : int t
      | Produce_models : bool t
      | Produce_unsat_assumptions : bool t
      | Produce_unsat_cores : bool t
      | Seed : int t
      | Verbosity : int t
      | Time_limit_per : int t
      | Bv_solver : B.Options.bv_solver t
      | Rewrite_level : int t
      | Sat_solver : B.Options.sat_solver t
      | Prop_const_bits : bool t
      | Prop_ineq_bounds : bool t
      | Prop_nprops : int t
      | Prop_nupdates : int t
      | Prop_opt_lt_concat_sext : bool t
      | Prop_path_sel : B.Options.prop_path_sel t
      | Prop_prob_pick_rand_input : int t
      | Prop_prob_pick_inv_value : int t
      | Prop_sext : bool t
      | Prop_normalize : bool t
      | Preprocess : bool t
      | Pp_contr_ands : bool t
      | Pp_elim_extracts : bool t
      | Pp_embedded : bool t
      | Pp_flatten_and : bool t
      | Pp_normalize : bool t
      | Pp_normalize_share_aware : bool t
      | Pp_skeleton_preproc : bool t
      | Pp_variable_subst : bool t
      | Pp_variable_subst_norm_eq : bool t
      | Pp_variable_subst_norm_diseq : bool t
      | Pp_variable_subst_norm_bv_ineq : bool t
      | Dbg_rw_node_thresh : int t
      | Dbg_pp_node_thresh : int t
      | Check_model : bool t
      | Check_unsat_core : bool t

    let sexpable (type a) (t : a t) : (module Sexpable with type t = a) =
      match t with
      | Log_level -> (module Int)
      | Produce_models -> (module Bool)
      | Produce_unsat_assumptions -> (module Bool)
      | Produce_unsat_cores -> (module Bool)
      | Seed -> (module Int)
      | Verbosity -> (module Int)
      | Time_limit_per -> (module Int)
      | Bv_solver -> (module Bv_solver)
      | Rewrite_level -> (module Int)
      | Sat_solver -> (module Sat_solver)
      | Prop_const_bits -> (module Bool)
      | Prop_ineq_bounds -> (module Bool)
      | Prop_nprops -> (module Int)
      | Prop_nupdates -> (module Int)
      | Prop_opt_lt_concat_sext -> (module Bool)
      | Prop_path_sel -> (module Prop_path_sel)
      | Prop_prob_pick_rand_input -> (module Int)
      | Prop_prob_pick_inv_value -> (module Int)
      | Prop_sext -> (module Bool)
      | Prop_normalize -> (module Bool)
      | Preprocess -> (module Bool)
      | Pp_contr_ands -> (module Bool)
      | Pp_elim_extracts -> (module Bool)
      | Pp_embedded -> (module Bool)
      | Pp_flatten_and -> (module Bool)
      | Pp_normalize -> (module Bool)
      | Pp_normalize_share_aware -> (module Bool)
      | Pp_skeleton_preproc -> (module Bool)
      | Pp_variable_subst -> (module Bool)
      | Pp_variable_subst_norm_eq -> (module Bool)
      | Pp_variable_subst_norm_diseq -> (module Bool)
      | Pp_variable_subst_norm_bv_ineq -> (module Bool)
      | Dbg_rw_node_thresh -> (module Int)
      | Dbg_pp_node_thresh -> (module Int)
      | Check_model -> (module Bool)
      | Check_unsat_core -> (module Bool)

    module rec Packed : sig
      type 'a t' := 'a t
      type t = T : _ t' -> t
    end = Packed

    module Plain = struct
      type 'a t' = 'a t

      type t =
        | Log_level
        | Produce_models
        | Produce_unsat_assumptions
        | Produce_unsat_cores
        | Seed
        | Verbosity
        | Time_limit_per
        | Bv_solver
        | Rewrite_level
        | Sat_solver
        | Prop_const_bits
        | Prop_ineq_bounds
        | Prop_nprops
        | Prop_nupdates
        | Prop_opt_lt_concat_sext
        | Prop_path_sel
        | Prop_prob_pick_rand_input
        | Prop_prob_pick_inv_value
        | Prop_sext
        | Prop_normalize
        | Preprocess
        | Pp_contr_ands
        | Pp_elim_extracts
        | Pp_embedded
        | Pp_flatten_and
        | Pp_normalize
        | Pp_normalize_share_aware
        | Pp_skeleton_preproc
        | Pp_variable_subst
        | Pp_variable_subst_norm_eq
        | Pp_variable_subst_norm_diseq
        | Pp_variable_subst_norm_bv_ineq
        | Dbg_rw_node_thresh
        | Dbg_pp_node_thresh
        | Check_model
        | Check_unsat_core
      [@@deriving sexp, enumerate]

      let of_gadt (type k) (t' : k t') =
        match t' with
        | Log_level -> Log_level
        | Produce_models -> Produce_models
        | Produce_unsat_assumptions -> Produce_unsat_assumptions
        | Produce_unsat_cores -> Produce_unsat_cores
        | Seed -> Seed
        | Verbosity -> Verbosity
        | Time_limit_per -> Time_limit_per
        | Bv_solver -> Bv_solver
        | Rewrite_level -> Rewrite_level
        | Sat_solver -> Sat_solver
        | Prop_const_bits -> Prop_const_bits
        | Prop_ineq_bounds -> Prop_ineq_bounds
        | Prop_nprops -> Prop_nprops
        | Prop_nupdates -> Prop_nupdates
        | Prop_opt_lt_concat_sext -> Prop_opt_lt_concat_sext
        | Prop_path_sel -> Prop_path_sel
        | Prop_prob_pick_rand_input -> Prop_prob_pick_rand_input
        | Prop_prob_pick_inv_value -> Prop_prob_pick_inv_value
        | Prop_sext -> Prop_sext
        | Prop_normalize -> Prop_normalize
        | Preprocess -> Preprocess
        | Pp_contr_ands -> Pp_contr_ands
        | Pp_elim_extracts -> Pp_elim_extracts
        | Pp_embedded -> Pp_embedded
        | Pp_flatten_and -> Pp_flatten_and
        | Pp_normalize -> Pp_normalize
        | Pp_normalize_share_aware -> Pp_normalize_share_aware
        | Pp_skeleton_preproc -> Pp_skeleton_preproc
        | Pp_variable_subst -> Pp_variable_subst
        | Pp_variable_subst_norm_eq -> Pp_variable_subst_norm_eq
        | Pp_variable_subst_norm_diseq -> Pp_variable_subst_norm_diseq
        | Pp_variable_subst_norm_bv_ineq -> Pp_variable_subst_norm_bv_ineq
        | Dbg_rw_node_thresh -> Dbg_rw_node_thresh
        | Dbg_pp_node_thresh -> Dbg_pp_node_thresh
        | Check_model -> Check_model
        | Check_unsat_core -> Check_unsat_core

      let to_gadt = function
        | Log_level -> Packed.T Log_level
        | Produce_models -> Packed.T Produce_models
        | Produce_unsat_assumptions -> Packed.T Produce_unsat_assumptions
        | Produce_unsat_cores -> Packed.T Produce_unsat_cores
        | Seed -> Packed.T Seed
        | Verbosity -> Packed.T Verbosity
        | Time_limit_per -> Packed.T Time_limit_per
        | Bv_solver -> Packed.T Bv_solver
        | Rewrite_level -> Packed.T Rewrite_level
        | Sat_solver -> Packed.T Sat_solver
        | Prop_const_bits -> Packed.T Prop_const_bits
        | Prop_ineq_bounds -> Packed.T Prop_ineq_bounds
        | Prop_nprops -> Packed.T Prop_nprops
        | Prop_nupdates -> Packed.T Prop_nupdates
        | Prop_opt_lt_concat_sext -> Packed.T Prop_opt_lt_concat_sext
        | Prop_path_sel -> Packed.T Prop_path_sel
        | Prop_prob_pick_rand_input -> Packed.T Prop_prob_pick_rand_input
        | Prop_prob_pick_inv_value -> Packed.T Prop_prob_pick_inv_value
        | Prop_sext -> Packed.T Prop_sext
        | Prop_normalize -> Packed.T Prop_normalize
        | Preprocess -> Packed.T Preprocess
        | Pp_contr_ands -> Packed.T Pp_contr_ands
        | Pp_elim_extracts -> Packed.T Pp_elim_extracts
        | Pp_embedded -> Packed.T Pp_embedded
        | Pp_flatten_and -> Packed.T Pp_flatten_and
        | Pp_normalize -> Packed.T Pp_normalize
        | Pp_normalize_share_aware -> Packed.T Pp_normalize_share_aware
        | Pp_skeleton_preproc -> Packed.T Pp_skeleton_preproc
        | Pp_variable_subst -> Packed.T Pp_variable_subst
        | Pp_variable_subst_norm_eq -> Packed.T Pp_variable_subst_norm_eq
        | Pp_variable_subst_norm_diseq -> Packed.T Pp_variable_subst_norm_diseq
        | Pp_variable_subst_norm_bv_ineq -> Packed.T Pp_variable_subst_norm_bv_ineq
        | Dbg_rw_node_thresh -> Packed.T Dbg_rw_node_thresh
        | Dbg_pp_node_thresh -> Packed.T Dbg_pp_node_thresh
        | Check_model -> Packed.T Check_model
        | Check_unsat_core -> Packed.T Check_unsat_core
    end
  end

  module KV = struct
    type t = K : 'a Key.t * 'a -> t

    let sexp_of_t = function
      | K (key, v) ->
        let (module S) = Key.sexpable key in
        [%sexp_of: Key.Plain.t * S.t] ((Key.Plain.of_gadt key), v)

    let t_of_sexp sexp =
      let key_plain, value_sexp = [%of_sexp: Key.Plain.t * Sexp.t] sexp in
      let T key = Key.Plain.to_gadt key_plain in
      let (module S) = Key.sexpable key in
      let value = S.t_of_sexp value_sexp in
      K (key, value)
  end

  let default =
    let t = B.Options.default () in
    B.Options.set t B.Options.Produce_models true;
    t

  let sexp_of_t t =
    List.map Key.Plain.all
      ~f:(fun key ->
          let T key = Key.Plain.to_gadt key in
          let value = B.Options.get t key in
          KV.K (key, value)
        )
    |> [%sexp_of: KV.t list]

  let t_of_sexp sexp =
    [%of_sexp: KV.t list] sexp
    |> List.fold ~init:(B.Options.default ())
      ~f:(fun acc (KV.K (key, value)) ->
          B.Options.set acc key value;
          acc
        )
end

module Types = struct
  type 'i instance = B.Solver.t
  type _ model = unit
  type ('i, 's) sort = B.Sort.t
  type ('i, 's) expr = B.Term.t
end

module Op_types = Smtcaml_intf.Make_op_types(Types)

module rec Base : Smtcaml_intf.Backend
  with module Types := Types 
   and module Op_types := Op_types
   and module Options := Options
= struct

  type _ t = B.Solver.t

  module Packed = struct
    type 'i t' = 'i t
    type t = T : _ t' -> t
  end

  module Sort = struct
    type (_,_) t = B.Sort.t
  end

  module Expr = struct
    type (_,_) t = B.Term.t

    let sort t =
      B.Term.sort t
  end

  module Model = struct
    type _ t = unit

    let eval_to_string instance _model expr =
      Some (B.Solver.get_value instance expr |> B.Term.to_string)

    let eval_bool instance _m expr =
      Some (B.Solver.get_value instance expr |> B.Term.is_bv_value_one)
  end

  let create ?(options = Options.default) () =
    Packed.T (B.Solver.create options)

  let var sort symbol =
    B.mk_const ~symbol sort

  let var_anon sort =
    B.mk_const ?symbol:None sort

  let check_current_and_get_model t : _ Smtcaml_intf.Solver_result.t =
    match B.Solver.check_sat t with
    | B.Result.Sat -> Satisfiable ()
    | B.Result.Unsat -> Unsatisfiable
    | B.Result.Unknown -> Unknown "unknown"
end

and Boolean_t : Smtcaml_intf.Boolean
  with module Types := Types
   and module Op_types := Op_types
= struct

  let sort_boolean _ = B.mk_bool_sort ()

  let assert_ t e = B.Solver.assert_formula t e

  module Boolean = struct
    module Numeral = struct
      let true_ _ = B.mk_true ()
      let false_ _ = B.mk_false ()

      let bool t b = if b then true_ t else false_ t

    end

    let not = B.mk_term1 Not
    let and_ = B.mk_term2 And
    let or_ = B.mk_term2 Or
    let and_list list = B.mk_term And (Array.of_list list)
    let or_list list = B.mk_term Or (Array.of_list list)

    let eq = B.mk_term2 Equal
    let neq a b = not (eq a b)
    let distinct vs = B.mk_term Distinct (Array.of_list vs)

    let ite = B.mk_term3 Ite

    let iff = B.mk_term2 Iff
    let implies = B.mk_term2 Implies

  end

  let assert_not t e = B.Solver.assert_formula t (Boolean.not e)
end

and Bitvector_t : Smtcaml_intf.Bitvector
  with module Types := Types
   and module Op_types := Op_types
= struct
  open T

  let sort_bitvector _ i = B.mk_bv_sort i

  module Bv = struct
    module Set = Smtcaml_utils.Set_ops.Make(T)

    module Sort = struct
      let length s = B.Sort.bv_size s
    end

    module Model = struct
      let eval i m e =
        Model.eval_to_string i m e
        |> Option.map ~f:(String.chop_prefix_exn ~prefix:"#b")
        |> Option.map ~f:Fast_bitvector.Little_endian.of_string
    end

    let length e = Sort.length (B.Term.sort e)

    module Numeral = struct
      let int s i = B.mk_bv_value_int s i

      let int_e e i = int (Expr.sort e) i

      let fast_bitvector s fbv =
        B.mk_bv_value s (Fast_bitvector.Little_endian.to_string fbv) 2

      let zero ~length t = B.mk_bv_zero (sort_bitvector t length)
      let zero_e e = B.mk_bv_zero (B.Term.sort e)
    end

    module Signed = struct
      let (==) = Boolean.eq
      let (<>) = Boolean.neq
      let (<) = B.mk_term2 Bv_slt
      let (<=) = B.mk_term2 Bv_sle
      let (>) = B.mk_term2 Bv_sgt
      let (>=) = B.mk_term2 Bv_sge
      let (+) = Bitvector_t.Bv.add
      let (-) = Bitvector_t.Bv.sub
      let (~-) = B.mk_term1 Bv_neg
      let ( * ) = B.mk_term2 Bv_mul
      let ( / ) = B.mk_term2 Bv_sdiv
      let ( mod ) = B.mk_term2 Bv_srem
    end

    module Unsigned = struct
      let (==) = Boolean.eq
      let (<>) = Boolean.neq
      let (<) = B.mk_term2 Bv_ult
      let (<=) = B.mk_term2 Bv_ule
      let (>) = B.mk_term2 Bv_ugt
      let (>=) = B.mk_term2 Bv_uge
      let (+) = Bitvector_t.Bv.add
      let (-) = Bitvector_t.Bv.sub
      let ( * ) = B.mk_term2 Bv_mul
      let ( / ) = B.mk_term2 Bv_udiv
      let ( mod ) = B.mk_term2 Bv_urem
    end

    let extract ~low ~high e =
      B.mk_term1_indexed2 Bv_extract e high low

    let extract_single ~bit e = extract ~low:bit ~high:bit e

    let concat = B.mk_term2 Bv_concat
    let concat_list list = B.mk_term Bv_concat (Array.of_list list)

    let zero_extend ~extra_zeros e = B.mk_term1_indexed1 Bv_zero_extend e extra_zeros
    let sign_extend ~extra_bits e = B.mk_term1_indexed1 Bv_sign_extend e extra_bits

    let not = B.mk_term1 Bv_not
    let and_ = B.mk_term2 Bv_and
    let or_ = B.mk_term2 Bv_or
    let xor = B.mk_term2 Bv_xor

    let add = B.mk_term2 Bv_add
    let sub = B.mk_term2 Bv_sub

    let is_zero e = Boolean.eq e (Numeral.zero_e e)
    let is_not_zero e = Boolean.neq e (Numeral.zero_e e)

    let is_all_ones e = Boolean.eq e (B.mk_bv_ones (B.Term.sort e))

    include Smtcaml_utils.Power_of_two.Make(T)

    let sign e =
      let length = length e in
      extract_single ~bit:(length - 1) e

    let parity e = B.mk_term1 Bv_redxor e

    include Smtcaml_utils.Add_sub_over_under_flow.Make(T)

    let shift_left ~count e = B.mk_term2 Bv_shl e count
    let shift_right_logical ~count e = B.mk_term2 Bv_shr e count
    let shift_right_arithmetic ~count e = B.mk_term2 Bv_ashr e count

    let of_bool b =
      let s = B.mk_bv_sort 1 in
      Boolean.ite b (B.mk_bv_one s) (B.mk_bv_zero s)
  end
end

and Uf_t : Smtcaml_intf.Uninterpreted_function
  with module Types := Types
= struct
  let sort_uninterpreted_function _t ~domain ~codomain =
    B.mk_fun_sort [| domain |] codomain

  module Ufun = struct
    let apply a b = B.mk_term2 Apply a b
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
end

include (T : module type of struct include T end
  with module Types := T.Types
   and module Op_types := T.Op_types
   and module Options := T.Options
 )

