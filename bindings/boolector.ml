open! Base

type fun_desc =
  { return_type : string
  ; parameter_count : int
  ; parameters : string list
  } [@@deriving sexp]

type api_registry_entry =
  { wrapper_name : string
  ; name : string
  ; description : fun_desc
  } [@@deriving sexp]

external get_api_registry : unit -> api_registry_entry list = "caml_get_api_registry"

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
external get_opt : btor -> btor_option -> int = "caml_boolector_get_opt"
external set_opt : btor -> btor_option -> int -> unit = "caml_boolector_set_opt"

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

external print_stats : btor -> unit = "caml_boolector_print_stats"

external assert_ : node -> unit = "caml_boolector_assert" [@@noalloc]

type solver_result =
  | Unknown
  | Sat
  | Unsat
[@@deriving sexp]

external sat : btor -> solver_result = "caml_boolector_sat" [@@noalloc]

(*$ open! Core

  let v ?caml_name s i = s, caml_name, i

  let operators =
    [ v "true" ~caml_name:"true_val" 0
    ; v "false" ~caml_name:"false_val" 0
    ; v "implies" 2
    ; v "iff" 2
    ; v "eq" 2
    ; v "ne" 2
    ; v "xor" 2
    ; v "and" ~caml_name:"and_" 2
    ; v "or" ~caml_name:"or_" 2
    ; v "nand" 2
    ; v "nor" 2
    ; v "add" 2
    ; v "uaddo" 2
    ; v "saddo" 2
    ; v "mul" 2
    ; v "umulo" 2
    ; v "smulo" 2
    ; v "ult" 2
    ; v "slt" 2
    ; v "ulte" 2
    ; v "slte" 2
    ; v "ugt" 2
    ; v "sgt" 2
    ; v "ugte" 2
    ; v "sgte" 2
    ; v "sll" 2
    ; v "srl" 2
    ; v "sra" 2
    ; v "rol" 2
    ; v "ror" 2
    ; v "sub" 2
    ; v "usubo" 2
    ; v "ssubo" 2
    ; v "udiv" 2
    ; v "sdiv" 2
    ; v "sdivo" 2
    ; v "urem" 2
    ; v "srem" 2
    ; v "smod" 2
    ; v "concat" 2
    ; v "read" 2
    ; v "write" 2
    ; v "cond" 3
    ; v "inc" 1
    ; v "dec" 1
    ]

  let () = print_endline ""

  let () = 
    List.iter operators
      ~f:(fun (api_name, caml_name, params) ->
          let caml_name = Option.value ~default:api_name caml_name in
          let args = if params = 0 then [ "btor" ] else [] in
          let args = args @ List.init params ~f:(Fn.const "node") in
          let args = List.append args [ "node" ] in
          let args =
            String.concat args ~sep:" -> "
          in
          printf "external %s : %s = \"caml_boolector_%s\"\n" caml_name args api_name
        )
  *)
external true_val : btor -> node = "caml_boolector_true"
external false_val : btor -> node = "caml_boolector_false"
external implies : node -> node -> node = "caml_boolector_implies"
external iff : node -> node -> node = "caml_boolector_iff"
external eq : node -> node -> node = "caml_boolector_eq"
external ne : node -> node -> node = "caml_boolector_ne"
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
external slt : node -> node -> node = "caml_boolector_slt"
external ulte : node -> node -> node = "caml_boolector_ulte"
external slte : node -> node -> node = "caml_boolector_slte"
external ugt : node -> node -> node = "caml_boolector_ugt"
external sgt : node -> node -> node = "caml_boolector_sgt"
external ugte : node -> node -> node = "caml_boolector_ugte"
external sgte : node -> node -> node = "caml_boolector_sgte"
external sll : node -> node -> node = "caml_boolector_sll"
external srl : node -> node -> node = "caml_boolector_srl"
external sra : node -> node -> node = "caml_boolector_sra"
external rol : node -> node -> node = "caml_boolector_rol"
external ror : node -> node -> node = "caml_boolector_ror"
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
external write : node -> node -> node = "caml_boolector_write"
external cond : node -> node -> node -> node = "caml_boolector_cond"
external inc : node -> node = "caml_boolector_inc"
external dec : node -> node = "caml_boolector_dec"
(*$*)
