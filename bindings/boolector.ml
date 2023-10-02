open! Base

type btor
type sort
type node
type btor_option [@@immediate]

type solver =
  | Lingeling
  | MiniSat
  | Cms
  | Cadical
[@@deriving sexp]

type uint32_t = int
type int32_t = int

type solver_result =
  | Unknown
  | Sat
  | Unsat
[@@deriving sexp]

(*$ open! Core
  open Cppcaml_lib
  (* dune cinaps currently doesn't support adding link flags, and we need -linkall,
     this is a workaround *)
  external unit_x : unit -> unit = "caml_boolector_unit"

  let () =
    let modify s =
      Option.value ~default:s (String.chop_prefix ~prefix:"boolector_" s)
      |> filter_keyword
    in
    emit_api ()
      ~modify
  *)
external reset_time : btor -> unit = "caml____boolector_reset_time"
external reset_stats : btor -> unit = "caml____boolector_reset_stats"
external print_stats : btor -> unit = "caml____boolector_print_stats"
external push : btor -> uint32_t -> unit = "caml____boolector_push"
external pop : btor -> uint32_t -> unit = "caml____boolector_pop"
external assert_ : node -> unit = "caml____boolector_assert"
external assume : node -> unit = "caml____boolector_assume"
external failed : node -> bool = "caml____boolector_failed"
external get_failed_assumptions : btor -> node array = "caml____boolector_get_failed_assumptions"
external fixate_assumptions : btor -> unit = "caml____boolector_fixate_assumptions"
external reset_assumptions : btor -> unit = "caml____boolector_reset_assumptions"
external simplify : btor -> int32_t = "caml____boolector_simplify"
external set_sat_solver : btor -> string -> unit = "caml____boolector_set_sat_solver"
external set_opt : btor -> btor_option -> uint32_t -> unit = "caml____boolector_set_opt"
external get_opt : btor -> btor_option -> uint32_t = "caml____boolector_get_opt"
external get_opt_min : btor -> btor_option -> uint32_t = "caml____boolector_get_opt_min"
external get_opt_max : btor -> btor_option -> uint32_t = "caml____boolector_get_opt_max"
external get_opt_dflt : btor -> btor_option -> uint32_t = "caml____boolector_get_opt_dflt"
external get_opt_lng : btor -> btor_option -> string = "caml____boolector_get_opt_lng"
external get_opt_shrt : btor -> btor_option -> string = "caml____boolector_get_opt_shrt"
external get_opt_desc : btor -> btor_option -> string = "caml____boolector_get_opt_desc"
external has_opt : btor -> btor_option -> bool = "caml____boolector_has_opt"
external first_opt : btor -> btor_option = "caml____boolector_first_opt"
external next_opt : btor -> btor_option -> btor_option = "caml____boolector_next_opt"
external true_ : btor -> node = "caml____boolector_true"
external false_ : btor -> node = "caml____boolector_false"
external implies : node -> node -> node = "caml____boolector_implies"
external iff : node -> node -> node = "caml____boolector_iff"
external eq : node -> node -> node = "caml____boolector_eq"
external ne : node -> node -> node = "caml____boolector_ne"
external is_bv_const_zero : node -> bool = "caml____boolector_is_bv_const_zero"
external is_bv_const_one : node -> bool = "caml____boolector_is_bv_const_one"
external is_bv_const_ones : node -> bool = "caml____boolector_is_bv_const_ones"
external is_bv_const_max_signed : node -> bool = "caml____boolector_is_bv_const_max_signed"
external is_bv_const_min_signed : node -> bool = "caml____boolector_is_bv_const_min_signed"
external const : btor -> string -> node = "caml____boolector_const"
external constd : sort -> string -> node = "caml____boolector_constd"
external consth : sort -> string -> node = "caml____boolector_consth"
external zero : sort -> node = "caml____boolector_zero"
external ones : sort -> node = "caml____boolector_ones"
external one : sort -> node = "caml____boolector_one"
external min_signed : sort -> node = "caml____boolector_min_signed"
external max_signed : sort -> node = "caml____boolector_max_signed"
external unsigned_int : btor -> uint32_t -> sort -> node = "caml____boolector_unsigned_int"
external int : btor -> int32_t -> sort -> node = "caml____boolector_int"
external var : sort -> string -> node = "caml____boolector_var"
external array : sort -> string -> node = "caml____boolector_array"
external const_array : sort -> node -> node = "caml____boolector_const_array"
external uf : sort -> string -> node = "caml____boolector_uf"
external not : node -> node = "caml____boolector_not"
external neg : node -> node = "caml____boolector_neg"
external redor : node -> node = "caml____boolector_redor"
external redxor : node -> node = "caml____boolector_redxor"
external redand : node -> node = "caml____boolector_redand"
external slice : node -> uint32_t -> uint32_t -> node = "caml____boolector_slice"
external uext : node -> uint32_t -> node = "caml____boolector_uext"
external sext : node -> uint32_t -> node = "caml____boolector_sext"
external xor : node -> node -> node = "caml____boolector_xor"
external xnor : node -> node -> node = "caml____boolector_xnor"
external and_ : node -> node -> node = "caml____boolector_and"
external nand : node -> node -> node = "caml____boolector_nand"
external or_ : node -> node -> node = "caml____boolector_or"
external nor : node -> node -> node = "caml____boolector_nor"
external add : node -> node -> node = "caml____boolector_add"
external uaddo : node -> node -> node = "caml____boolector_uaddo"
external saddo : node -> node -> node = "caml____boolector_saddo"
external mul : node -> node -> node = "caml____boolector_mul"
external umulo : node -> node -> node = "caml____boolector_umulo"
external smulo : node -> node -> node = "caml____boolector_smulo"
external ult : node -> node -> node = "caml____boolector_ult"
external slt : node -> node -> node = "caml____boolector_slt"
external ulte : node -> node -> node = "caml____boolector_ulte"
external slte : node -> node -> node = "caml____boolector_slte"
external ugt : node -> node -> node = "caml____boolector_ugt"
external sgt : node -> node -> node = "caml____boolector_sgt"
external ugte : node -> node -> node = "caml____boolector_ugte"
external sgte : node -> node -> node = "caml____boolector_sgte"
external sll : node -> node -> node = "caml____boolector_sll"
external srl : node -> node -> node = "caml____boolector_srl"
external sra : node -> node -> node = "caml____boolector_sra"
external rol : node -> node -> node = "caml____boolector_rol"
external ror : node -> node -> node = "caml____boolector_ror"
external roli : node -> uint32_t -> node = "caml____boolector_roli"
external rori : node -> uint32_t -> node = "caml____boolector_rori"
external sub : node -> node -> node = "caml____boolector_sub"
external usubo : node -> node -> node = "caml____boolector_usubo"
external ssubo : node -> node -> node = "caml____boolector_ssubo"
external udiv : node -> node -> node = "caml____boolector_udiv"
external sdiv : node -> node -> node = "caml____boolector_sdiv"
external sdivo : node -> node -> node = "caml____boolector_sdivo"
external urem : node -> node -> node = "caml____boolector_urem"
external srem : node -> node -> node = "caml____boolector_srem"
external smod : node -> node -> node = "caml____boolector_smod"
external concat : node -> node -> node = "caml____boolector_concat"
external repeat : node -> uint32_t -> node = "caml____boolector_repeat"
external read : node -> node -> node = "caml____boolector_read"
external write : node -> node -> node -> node = "caml____boolector_write"
external cond : node -> node -> node -> node = "caml____boolector_cond"
external param : sort -> string -> node = "caml____boolector_param"
external inc : node -> node = "caml____boolector_inc"
external dec : node -> node = "caml____boolector_dec"
external get_btor : node -> btor = "caml____boolector_get_btor"
external get_node_id : node -> int32_t = "caml____boolector_get_node_id"
external get_sort : node -> sort = "caml____boolector_get_sort"
external fun_get_domain_sort : node -> sort = "caml____boolector_fun_get_domain_sort"
external fun_get_codomain_sort : node -> sort = "caml____boolector_fun_get_codomain_sort"
external match_node_by_id : btor -> int32_t -> node = "caml____boolector_match_node_by_id"
external match_node_by_symbol : btor -> string -> node = "caml____boolector_match_node_by_symbol"
external match_node : node -> node = "caml____boolector_match_node"
external get_symbol : node -> string = "caml____boolector_get_symbol"
external set_symbol : node -> string -> unit = "caml____boolector_set_symbol"
external get_width : node -> uint32_t = "caml____boolector_get_width"
external get_index_width : node -> uint32_t = "caml____boolector_get_index_width"
external get_fun_arity : node -> uint32_t = "caml____boolector_get_fun_arity"
external is_const : node -> bool = "caml____boolector_is_const"
external is_var : node -> bool = "caml____boolector_is_var"
external is_array : node -> bool = "caml____boolector_is_array"
external is_array_var : node -> bool = "caml____boolector_is_array_var"
external is_param : node -> bool = "caml____boolector_is_param"
external is_bound_param : node -> bool = "caml____boolector_is_bound_param"
external is_uf : node -> bool = "caml____boolector_is_uf"
external is_fun : node -> bool = "caml____boolector_is_fun"
external get_value : node -> node = "caml____boolector_get_value"
external bool_sort : btor -> sort = "caml____boolector_bool_sort"
external bitvec_sort : btor -> uint32_t -> sort = "caml____boolector_bitvec_sort"
external is_equal_sort : node -> node -> bool = "caml____boolector_is_equal_sort"
external is_array_sort : sort -> bool = "caml____boolector_is_array_sort"
external is_bitvec_sort : sort -> bool = "caml____boolector_is_bitvec_sort"
external is_fun_sort : sort -> bool = "caml____boolector_is_fun_sort"
external bitvec_sort_get_width : sort -> uint32_t = "caml____boolector_bitvec_sort_get_width"
external copyright : btor -> string = "caml____boolector_copyright"
external version : btor -> string = "caml____boolector_version"
external git_id : btor -> string = "caml____boolector_git_id"
external new_ : unit -> btor = "caml____boolector_new_"
external sat : btor -> solver_result = "caml_boolector_sat"
external limited_sat : btor -> int32_t -> int32_t -> solver_result = "caml_boolector_limited_sat"
external bv_assignment : node -> string = "caml_boolector_bv_assignment"
(*$*)

let set_solver btor (solver : solver) =
  Sexp.to_string ([%sexp_of: solver] solver)
  |> String.lowercase
  |> set_sat_solver btor

module BOption = struct
  type t =
    { b : (btor_option [@sexp.opaque])
    ; name_long : string
    ; name_short : string
    ; description : string
    ; min : int
    ; max : int
    ; default : int
    } [@@deriving sexp_of]

  let create btor b =
    { b
    ; name_long = get_opt_lng btor b
    ; name_short = get_opt_shrt btor b
    ; description = get_opt_desc btor b
    ; min = get_opt_min btor b
    ; max = get_opt_max btor b
    ; default = get_opt_dflt btor b
    }
end

let all_options btor =
  let first_opt = first_opt btor in
  let current = ref first_opt in
  let acc = ref [ first_opt ] in
  while
    let next = next_opt btor !current in
    if Stdlib.not (has_opt btor next)
    then false 
    else begin
      acc := next :: !acc;
      current := next;
      true
    end
  do ()
  done;
  List.map ~f:(BOption.create btor) !acc

