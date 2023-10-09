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

module Types = struct
  type 'i instance = B.btor
  type _ model = unit
  type ('i, 's) sort = B.sort
  type ('i, 's) expr = B.node
end

module Op_types = Smtcaml_intf.Make_op_types(Types)

let get_sort_context (sort : B.sort) : B.btor = Obj.magic sort
let get_expr_context (expr : B.node) : B.btor = Obj.magic expr

module rec Base : Smtcaml_intf.Backend_base
  with module Types := Types 
   and module Op_types := Op_types
   and module Options := Options
= struct

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

    let sort = B.get_sort
  end

  module Model = struct
    type _ t = unit

    let eval_to_string _instance _model expr =
      Some (B.bv_assignment expr)

    let eval_bool _instance _model expr =
      Some (B.is_bv_const_one expr)
  end

  let create ?(options=Options.default) () =
    let t = B.new_ () in
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

  let var sort name =
    if B.is_bitvec_sort sort
    then B.var sort name
    else if B.is_fun_sort sort
    then B.uf_opt sort (Some name)
    else assert false

  let var_anon sort =
    if B.is_bitvec_sort sort
    then B.var_opt sort None
    else if B.is_fun_sort sort
    then B.uf_opt sort None
    else assert false

  let check_current_and_get_model t : _ Smtcaml_intf.Solver_result.t =
    match B.satR t with
    | B.Unknown -> Unknown "unknown"
    | B.Sat -> Satisfiable ()
    | B.Unsat -> Unsatisfiable
end

and Boolean_t : Smtcaml_intf.Boolean
  with module Types := Types
   and module Op_types := Op_types
= struct

  let sort_boolean t =
    B.bool_sort t

  let assert_ _ expr =
    B.assert_ expr

  module Boolean = struct
    module Numeral = struct
      let true_ t = B.true_ t
      let false_ t = B.false_ t
      let bool t b = if b then true_ t else false_ t
    end

    let and_ = B.and_
    let or_ = B.or_
    let and_list = List.reduce_balanced_exn ~f:and_
    let or_list = List.reduce_balanced_exn ~f:or_

    let eq = B.eq
    let neq = B.ne
    let ite = B.cond
    let implies = B.implies
    let iff = B.iff

    let not = B.not

    let distinct l =
      List.Cartesian_product.map2 l l ~f:(fun a b ->
          if phys_equal a b
          then None
          else Some (neq a b)
        )
      |> List.reduce_balanced_exn ~f:(fun a b ->
          match a, b with
          | Some a, Some b -> Some (and_ a b)
          | None, (Some _ as a)
          | (Some _ as a), None -> a
          | None, None -> None
        )
      |> Option.value_exn
  end

end

and Bitvector_t : Smtcaml_intf.Bitvector
  with module Types := Types
   and module Op_types := Op_types
= struct
  open T

  let sort_bitvector t length =
    B.bitvec_sort t length

  module Bv = struct
    module Sort = struct
      let length sort = B.bitvec_sort_get_width sort
    end

    module Model = struct
      let eval instance model expr =
        Model.eval_to_string instance model expr
        |> Option.map ~f:Fast_bitvector.Little_endian.of_string
    end

    let length e = Sort.length (Expr.sort e)

    module Numeral = struct
      let int sort i = B.int (get_sort_context sort) i sort

      let fast_bitvector sort bv =
        assert (Sort.length sort = Fast_bitvector.length bv);
        B.const
          (get_sort_context sort)
          (Fast_bitvector.Little_endian.to_string bv)

      let zero ~length t = B.zero (sort_bitvector t length)

      let zero_e e = B.zero (Expr.sort e)
    end

    let extract ~low ~high e = B.slice e high low

    let extract_single ~bit e = extract ~low:bit ~high:bit e

    let concat e0 e1 = B.concat e0 e1

    let zero_extend ~extra_zeros e = B.uext e extra_zeros

    let sign_extend ~extra_bits e = B.sext e extra_bits

    let sign e =
      let length = length e in
      extract_single ~bit:(length - 1) e

    let not = B.not
    let and_ = B.and_
    let or_ = B.or_
    let xor = B.xor

    let add = B.add
    let sub = B.sub

    let is_zero e =
      B.eq (Numeral.zero_e e) e

    let is_not_zero e =
      B.ne (Numeral.zero_e e) e

    let is_all_ones e =
      let sort = Expr.sort e in
      B.eq (B.not (B.zero sort)) e

    let parity e = B.redxor e

    include Smtcaml_utils.Add_sub_over_under_flow.Make(T)

    let shift_left ~count e = B.sll e count
    let shift_right_logical ~count e = B.srl e count
    let shift_right_arithmetic ~count e = B.sra e count

    let of_bool b = b
  end

end

and Uf_t : Smtcaml_intf.Uninterpreted_function
  with module Types := Types
= struct
  let sort_uninterpreted_function t ~domain ~codomain =
    B.fun_sort_vector t [| domain |] codomain

  module Ufun = struct
    let apply a b = B.apply_vector (get_expr_context b) [| b |] a
  end
end

and T : Smtcaml_intf.Interface_definitions.Bitvector_uf
  with module Types = Types
   and module Op_types = Op_types
   and module Options = Options
= struct
  include Base
  include Boolean_t
  include Bitvector_t
  include Uf_t

  module Types = Types
  module Op_types = Op_types
  module Options = Options
end

include (T : module type of struct include T end
  with module Types := T.Types
   and module Op_types := T.Op_types
   and module Options := T.Options
 )

