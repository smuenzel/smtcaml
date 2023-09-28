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

let function_type
    { return_type
    ; parameter_count
    ; parameters
    }
  =
  let parameters =
    if parameter_count = 0
    then [ "unit" ]
    else parameters
  in
  String.concat ~sep:" -> " (parameters @ [ return_type])

let emit_entry
    ~modify
    { wrapper_name
    ; name
    ; description
    }
  =
  Stdlib.Printf.printf
    {|external %s : %s = "%s"|}
    (String.uncapitalize (modify name))
    (function_type description)
    wrapper_name
  ;
  Stdlib.print_newline ()

let emit_api ?registry ?(filter=Fn.const true) ?(modify=Fn.id) () =
  let registry =
    match registry with
    | None -> get_api_registry ()
    | Some registry -> registry
  in
  Stdlib.print_newline ();
  List.iter (List.rev registry)
    ~f:(fun entry ->
        if filter entry.name
        then emit_entry ~modify entry
      )

let filter_keyword = function
  | "new" -> "new_"
  | "assert" -> "assert_"
  | "or" -> "or_"
  | "and" -> "and_"
  | "true" -> "true_"
  | "false" -> "false_"
  | s -> s
