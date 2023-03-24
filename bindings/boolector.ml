

type btor
type sort
type node

type solver =
  | Lingeling
  | MiniSat
  | CryptoMiniSat
  | Cadical
[@@deriving sexp]

external make : unit -> btor = "caml_boolector_new"
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
