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

external make : unit -> btor = "caml_boolector_new"

external set_solver : btor -> string -> unit = "caml_boolector_set_sat_solver" [@@noalloc]

let set_solver btor (solver : solver) =
  Sexp.to_string ([%sexp_of: solver] solver)
  |> String.lowercase
  |> set_solver btor

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
external get_sort : node -> sort = "caml_boolector_get_sort"
external false_ : btor -> node = "caml____boolector_false"
external true_ : btor -> node = "caml____boolector_true"
external implies : node -> node -> node = "caml_boolector_implies"
external iff : node -> node -> node = "caml_boolector_iff"
external eq : node -> node -> node = "caml_boolector_eq"
external ne : node -> node -> node = "caml_boolector_ne"
external not : node -> node = "caml_boolector_not"
external neg : node -> node = "caml_boolector_neg"
external redor : node -> node = "caml_boolector_redor"
external redxor : node -> node = "caml_boolector_redxor"
external redand : node -> node = "caml_boolector_redand"
external slice : node -> uint32_t -> uint32_t -> node = "caml_boolector_slice"
external uext : node -> uint32_t -> node = "caml_boolector_uext"
external sext : node -> uint32_t -> node = "caml_boolector_sext"
external xor : node -> node -> node = "caml_boolector_xor"
external and_ : node -> node -> node = "caml_boolector_and"
external or_ : node -> node -> node = "caml_boolector_or"
external nand : node -> node -> node = "caml_boolector_nand"
external nor : node -> node -> node = "caml_boolector_nor"
external add : node -> node -> node = "caml_boolector_add"
external uaddo : node -> node -> node = "caml_boolector_uaddo"
external saddo : node -> node -> node = "caml_boolector_saddo"
external mul : node -> node -> node = "caml_boolector_mul"
external umulo : node -> node -> node = "caml_boolector_umulo"
external smulo : node -> node -> node = "caml_boolector_smulo"
external ult : node -> node -> node = "caml_boolector_ult"
external ulte : node -> node -> node = "caml_boolector_ulte"
external slte : node -> node -> node = "caml_boolector_slte"
external slt : node -> node -> node = "caml_boolector_slt"
external ugt : node -> node -> node = "caml_boolector_ugt"
external sgt : node -> node -> node = "caml_boolector_sgt"
external ugte : node -> node -> node = "caml_boolector_ugte"
external sgte : node -> node -> node = "caml_boolector_sgte"
external sll : node -> node -> node = "caml_boolector_sll"
external srl : node -> node -> node = "caml_boolector_srl"
external sra : node -> node -> node = "caml_boolector_sra"
external rol : node -> node -> node = "caml_boolector_rol"
external ror : node -> node -> node = "caml_boolector_ror"
external roli : node -> uint32_t -> node = "caml_boolector_roli"
external rori : node -> uint32_t -> node = "caml_boolector_rori"
external sub : node -> node -> node = "caml_boolector_sub"
external usubo : node -> node -> node = "caml_boolector_usubo"
external ssubo : node -> node -> node = "caml_boolector_ssubo"
external udiv : node -> node -> node = "caml_boolector_udiv"
external sdiv : node -> node -> node = "caml_boolector_sdiv"
external sdivo : node -> node -> node = "caml_boolector_sdivo"
external urem : node -> node -> node = "caml_boolector_urem"
external srem : node -> node -> node = "caml_boolector_srem"
external smod : node -> node -> node = "caml_boolector_smod"
external concat : node -> node -> node = "caml_boolector_concat"
external read : node -> node -> node = "caml_boolector_read"
external write : node -> node -> node -> node = "caml_boolector_write"
external cond : node -> node -> node -> node = "caml_boolector_cond"
external inc : node -> node = "caml_boolector_inc"
external dec : node -> node = "caml_boolector_dec"
external bool_sort : btor -> sort = "caml____boolector_bool_sort"
external var : sort -> string -> node = "caml_boolector_var"
external bitvec_sort : btor -> uint32_t -> sort = "caml_boolector_bitvec_sort"
external array_sort : sort -> sort -> sort = "caml_boolector_array_sort"
external print_stats : btor -> unit = "caml____boolector_print_stats"
external push : btor -> uint32_t -> unit = "caml_boolector_push"
external pop : btor -> uint32_t -> unit = "caml_boolector_pop"
external assert_ : node -> unit = "caml_boolector_assert"
external assume : node -> unit = "caml_boolector_assume"
external failed : node -> bool = "caml_boolector_failed"
external get_failed_assumptions : btor -> node array = "caml____boolector_get_failed_assumptions"
external fixate_assumptions : btor -> unit = "caml____boolector_fixate_assumptions"
external reset_assumptions : btor -> unit = "caml____boolector_reset_assumptions"
external set_sat_solver : btor -> string -> unit = "caml_boolector_set_sat_solver"
external first_opt : btor -> btor_option = "caml____boolector_first_opt"
external has_opt : btor -> btor_option -> bool = "caml_boolector_has_opt"
external next_opt : btor -> btor_option -> btor_option = "caml_boolector_next_opt"
external get_opt_lng : btor -> btor_option -> string = "caml_boolector_get_opt_lng"
external get_opt_shrt : btor -> btor_option -> string = "caml_boolector_get_opt_shrt"
external get_opt_desc : btor -> btor_option -> string = "caml_boolector_get_opt_desc"
external get_opt_min : btor -> btor_option -> uint32_t = "caml_boolector_get_opt_min"
external get_opt_max : btor -> btor_option -> uint32_t = "caml_boolector_get_opt_max"
external get_opt_dflt : btor -> btor_option -> uint32_t = "caml_boolector_get_opt_dflt"
external get_opt : btor -> btor_option -> uint32_t = "caml_boolector_get_opt"
external set_opt : btor -> btor_option -> uint32_t -> unit = "caml_boolector_set_opt"
external new_ : unit -> btor = "caml_boolector_new"
external get_btor : node -> btor = "caml_boolector_get_btor"
external sat : btor -> solver_result = "caml_boolector_sat"
external limited_sat : btor -> int32_t -> int32_t -> solver_result = "caml_boolector_limited_sat"
external bv_assignment : node -> string = "caml_boolector_bv_assignment"
(*$*)

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

