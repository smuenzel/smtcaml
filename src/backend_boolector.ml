open! Core

module B = Smtcaml_bindings.Boolector

module Options = struct
  type t = 
    { solver : B.solver
    } [@@deriving sexp, fields]

  let default =
    { solver = B.Cadical
    }
end

type _ t = B.btor

module Packed = struct
  type 'i t' = 'i t
  type t = T : _ t' -> t
end

module Sort = struct
  type (_,_) t = B.sort
end

module Expr = struct
  type (_,_) t = B.node
end

module Model = struct
  type _ t = unit

  let eval_to_string _instance _model expr =
    Some (B.bv_assignment expr)
end

let create ?(options=Options.default) () =
  let t = B.make () in
  Options.Fields.Direct.iter options
    ~solver:(fun _ _ s -> B.set_solver t s)
  ;
  let all_options = B.all_options t in
  List.iter all_options
    ~f:(fun opt ->
        match opt.name_long with
        (* CR smuenzel: fragile! *)
        | "model-gen" -> B.set_opt t opt.b 2
        | _ -> ()
      );
  Packed.T t

let sort_bitvector t length =
  B.bitvec_sort t length

let sort_boolean t =
  B.bool_sort t

let var sort name =
  B.var sort name

let assert_ expr =
  B.assert_ expr

let check_current_and_get_model t : _ Smtcaml_intf.Solver_result.t =
  match B.sat t with
  | B.Unknown -> Unknown "unknown"
  | B.Sat -> Satisfiable ()
  | B.Unsat -> Unsatisfiable

module Bv = struct
  let not = B.not
  let and_ = B.and_
  let or_ = B.or_
  let xor = B.xor

  let of_bool b = b
end

module Boolean = struct
  let eq = B.eq
  let neq = B.ne
end

module Types = struct
  type 'i instance = 'i t
  type _ model = unit
  type ('i, 's) sort = ('i, 's) Sort.t
  type ('i, 's) expr = ('i, 's) Expr.t
end

module Op_types = Smtcaml_intf.Make_op_types(Types)
