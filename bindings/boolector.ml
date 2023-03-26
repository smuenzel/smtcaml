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

external make : unit -> btor = "caml_boolector_new"

external set_solver : btor -> string -> unit = "caml_boolector_set_sat_solver" [@@noalloc]

let set_solver btor (solver : solver) =
  Sexp.to_string ([%sexp_of: solver] solver)
  |> String.lowercase
  |> set_solver btor 

external first_opt : btor -> btor_option = "caml_boolector_first_opt" [@@noalloc]
external next_opt : btor -> btor_option -> btor_option = "caml_boolector_next_opt" [@@noalloc]
external has_opt : btor -> btor_option -> bool = "caml_boolector_has_opt" [@@noalloc]
external opt_name_long : btor -> btor_option -> string = "caml_boolector_get_opt_lng"
external opt_desc : btor -> btor_option -> string = "caml_boolector_get_opt_desc"
external opt_min : btor -> btor_option -> int = "caml_boolector_get_opt_min"
external opt_max : btor -> btor_option -> int = "caml_boolector_get_opt_max"
external opt_default : btor -> btor_option -> int = "caml_boolector_get_opt_dflt"

module BOption = struct
  type t =
    { b : (btor_option [@sexp.opaque])
    ; name_long : string
    ; description : string
    ; min : int
    ; max : int
    ; default : int
    } [@@deriving sexp_of]

  let create btor b =
    { b
    ; name_long = opt_name_long btor b
    ; description = opt_desc btor b
    ; min = opt_min btor b
    ; max = opt_max btor b
    ; default = opt_default btor b
    }
end

let all_options btor =
  let first_opt = first_opt btor in
  let current = ref first_opt in
  let acc = ref [ first_opt ] in
  while
    let next = next_opt btor !current in
    if not (has_opt btor next)
    then begin
      Stdlib.Printf.printf "%i %i\n" (Stdlib.Obj.magic next) (List.length !acc);
      false
    end
    else begin
      acc := next :: !acc;
      current := next;
      true
    end
  do ()
  done;
  List.map ~f:(BOption.create btor) !acc

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
external sat : btor -> solver_result = "caml_boolector_sat"
external get_btor : node -> btor = "caml_boolector_get_btor"
external new_ : unit -> btor = "caml_boolector_new"
external set_opt : btor -> btor_option -> uint32_t -> unit = "caml_boolector_set_opt"
external get_opt : btor -> btor_option -> uint32_t = "caml_boolector_get_opt"
external get_opt_dflt : btor -> btor_option -> uint32_t = "caml_boolector_get_opt_dflt"
external get_opt_max : btor -> btor_option -> uint32_t = "caml_boolector_get_opt_max"
external get_opt_min : btor -> btor_option -> uint32_t = "caml_boolector_get_opt_min"
external get_opt_desc : btor -> btor_option -> string = "caml_boolector_get_opt_desc"
external get_opt_lng : btor -> btor_option -> string = "caml_boolector_get_opt_lng"
external next_opt : btor -> btor_option -> btor_option = "caml_boolector_next_opt"
external has_opt : btor -> btor_option -> bool = "caml_boolector_has_opt"
external first_opt : btor -> btor_option = "caml_boolector_first_opt"
external set_sat_solver : btor -> string -> unit = "caml_boolector_set_sat_solver"
external assert_ : node -> unit = "caml_boolector_assert"
external print_stats : btor -> unit = "caml_boolector_print_stats"
external array_sort : sort -> sort -> sort = "caml_boolector_array_sort"
external bitvec_sort : btor -> uint32_t -> sort = "caml_boolector_bitvec_sort"
external var : sort -> string -> node = "caml_boolector_var"
external bool_sort : btor -> sort = "caml_boolector_bool_sort"
external dec : node -> node = "caml_boolector_dec"
external inc : node -> node = "caml_boolector_inc"
external cond : node -> node -> node -> node = "caml_boolector_cond"
external write : node -> node -> node -> node = "caml_boolector_write"
external read : node -> node -> node = "caml_boolector_read"
external concat : node -> node -> node = "caml_boolector_concat"
external smod : node -> node -> node = "caml_boolector_smod"
external srem : node -> node -> node = "caml_boolector_srem"
external urem : node -> node -> node = "caml_boolector_urem"
external sdivo : node -> node -> node = "caml_boolector_sdivo"
external sdiv : node -> node -> node = "caml_boolector_sdiv"
external udiv : node -> node -> node = "caml_boolector_udiv"
external ssubo : node -> node -> node = "caml_boolector_ssubo"
external usubo : node -> node -> node = "caml_boolector_usubo"
external sub : node -> node -> node = "caml_boolector_sub"
external rori : node -> uint32_t -> node = "caml_boolector_rori"
external roli : node -> uint32_t -> node = "caml_boolector_roli"
external ror : node -> node -> node = "caml_boolector_ror"
external rol : node -> node -> node = "caml_boolector_rol"
external sra : node -> node -> node = "caml_boolector_sra"
external srl : node -> node -> node = "caml_boolector_srl"
external sll : node -> node -> node = "caml_boolector_sll"
external sgte : node -> node -> node = "caml_boolector_sgte"
external ugte : node -> node -> node = "caml_boolector_ugte"
external sgt : node -> node -> node = "caml_boolector_sgt"
external ugt : node -> node -> node = "caml_boolector_ugt"
external slt : node -> node -> node = "caml_boolector_slt"
external slte : node -> node -> node = "caml_boolector_slte"
external ulte : node -> node -> node = "caml_boolector_ulte"
external ult : node -> node -> node = "caml_boolector_ult"
external smulo : node -> node -> node = "caml_boolector_smulo"
external umulo : node -> node -> node = "caml_boolector_umulo"
external mul : node -> node -> node = "caml_boolector_mul"
external saddo : node -> node -> node = "caml_boolector_saddo"
external uaddo : node -> node -> node = "caml_boolector_uaddo"
external add : node -> node -> node = "caml_boolector_add"
external nor : node -> node -> node = "caml_boolector_nor"
external nand : node -> node -> node = "caml_boolector_nand"
external or_ : node -> node -> node = "caml_boolector_or"
external and_ : node -> node -> node = "caml_boolector_and"
external xor : node -> node -> node = "caml_boolector_xor"
external sext : node -> uint32_t -> node = "caml_boolector_sext"
external uext : node -> uint32_t -> node = "caml_boolector_uext"
external slice : node -> uint32_t -> uint32_t -> node = "caml_boolector_slice"
external redand : node -> node = "caml_boolector_redand"
external redxor : node -> node = "caml_boolector_redxor"
external redor : node -> node = "caml_boolector_redor"
external neg : node -> node = "caml_boolector_neg"
external not : node -> node = "caml_boolector_not"
external ne : node -> node -> node = "caml_boolector_ne"
external eq : node -> node -> node = "caml_boolector_eq"
external iff : node -> node -> node = "caml_boolector_iff"
external implies : node -> node -> node = "caml_boolector_implies"
external true_ : btor -> node = "caml_boolector_true"
external false_ : btor -> node = "caml_boolector_false"
external get_sort : node -> sort = "caml_boolector_get_sort"
(*$*)
