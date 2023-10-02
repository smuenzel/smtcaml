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
end

module Model = struct
  type _ t = unit

  let eval_to_string instance _model expr =
    Some (B.Solver.get_value instance expr |> B.Term.to_string)
end

let create ?(options = Options.default) () =
  Packed.T (B.Solver.create options)

let var sort symbol =
  B.mk_const ~symbol sort

let check_current_and_get_model t : _ Smtcaml_intf.Solver_result.t =
  match B.Solver.check_sat t with
  | B.Result.Sat -> Satisfiable ()
  | B.Result.Unsat -> Unsatisfiable
  | B.Result.Unknown -> Unknown "unknown"

module Types = struct
  type 'i instance = 'i t
  type _ model = unit
  type ('i, 's) sort = ('i, 's) Sort.t
  type ('i, 's) expr = ('i, 's) Expr.t
end

module Op_types = Smtcaml_intf.Make_op_types(Types)
