

type btor

external make : unit -> btor = "caml_boolector_new"
external print_stats : btor -> unit = "caml_boolector_print_stats"
