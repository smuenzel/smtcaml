open! Core

type fun_desc =
  { return_type : string
  ; parameter_count : int
  ; parameters : string list
  ; class_name : string option
  ; may_allocate : bool
  } [@@deriving sexp]

type api_registry_entry =
  { wrapper_name : string
  ; name : string
  ; description : fun_desc
  } [@@deriving sexp]

type api_enum_entry = 
  { enum_name : string
  ; member_name : string
  ; value : int
  } [@@deriving sexp]

external get_api_registry : unit -> api_registry_entry list = "caml_get_api_registry"
external get_api_enums : unit -> api_enum_entry list = "caml_get_api_enums"

let function_type
    { return_type
    ; parameter_count
    ; parameters
    ; class_name
    ; may_allocate = _
    }
  =
  let parameters =
    if parameter_count = 0
    then [ "unit" ]
    else parameters
  in
  let parameters = List.map ~f:String.uncapitalize parameters in
  let return_type = String.uncapitalize return_type in
  let r = String.concat ~sep:" -> " (parameters @ [ return_type]) in
  match class_name with
  | None -> r
  | Some class_name -> Printf.sprintf "(* class %s *) %s" class_name r

let emit_entry
    ~modify
    { wrapper_name
    ; name
    ; description
    }
  =
  let noalloc =
    (* CR smuenzel: this does not take into account potential callbacks
       (e.g. boolector has exeception callbacks).
       Also, we don't know if the function will release the ocaml lock.
       So disable for now. *)
    if true || description.may_allocate
    then ""
    else " [@@ocaml.noalloc]"
  in
  if description.parameter_count > 5
  then begin
    Stdlib.Printf.printf
      {|external %s : %s = "%s_bytecode" "%s"%s|}
      (String.uncapitalize (modify name))
      (function_type description)
      wrapper_name
      wrapper_name
      noalloc
  end else begin
    Stdlib.Printf.printf
      {|external %s : %s = "%s"%s|}
      (String.uncapitalize (modify name))
      (function_type description)
      wrapper_name
      noalloc
  end;
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

let all_enums () =
  let et = String.Table.create () in
  get_api_enums ()
  |> List.iter ~f:(fun entry ->
      Hashtbl.add_multi et ~key:entry.enum_name ~data:(entry.value, entry.member_name)
    );
  Hashtbl.map et
    ~f:(fun list ->
        List.sort list ~compare:(fun (i0,_) (i1,_) -> Int.compare i0 i1)
        |> List.mapi
          ~f:(fun i (i', en) ->
              assert (i = i');
              en
            )
      )

let emit_enum ~name ~entries =
  Stdlib.Printf.printf "type %s =\n" (String.uncapitalize name);
  List.iter entries
    ~f:(fun e ->
        Stdlib.Printf.printf "  | %s\n" e
      );
  Stdlib.Printf.printf "[@@deriving sexp]\n\n";
  ()

let emit_enums () =
  Stdlib.print_newline ();
  let enums = all_enums () in
  Hashtbl.iteri enums ~f:(fun ~key:name ~data:entries -> emit_enum ~name ~entries)

