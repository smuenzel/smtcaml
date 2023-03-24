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

external make : unit -> btor = "caml_boolector_new"

external set_solver : btor -> string -> unit = "caml_boolector_set_solver" [@@noalloc]

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

external print_stats : btor -> unit = "caml_boolector_print_stats"

external assert_ : node -> unit = "caml_boolector_assert" [@@noalloc]

type solver_result =
  | Unknown
  | Sat
  | Unsat
[@@deriving sexp]

external sat : btor -> solver_result = "caml_boolector_sat" [@@noalloc]

(*$ open! Core

  let operators =
    [ "true", Some "true_val", 0
    ; "false", Some "false_val", 0
    ; "implies", None, 2
    ; "iff", None, 2
    ; "eq", None, 2
    ; "ne", None, 2
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
(*$*)
