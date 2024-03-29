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

(* We can't use the eval functions when evaluating a function, since boolector does
   the string conversion *)
let eval_string_hack expr ~f_normal ~f_string =
  let oexpr = Obj.repr expr in
  if Obj.is_block oexpr
  then begin
    if Obj.string_tag = (Obj.tag oexpr)
    then f_string (Obj.obj oexpr : string)
    else f_normal expr
  end
  else f_normal expr

module rec Base : Smtcaml_intf.Backend
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

    let eval_to_string_exn _instance _model expr =
      eval_string_hack expr
        ~f_normal:B.bv_assignment
        ~f_string:Fn.id

    let eval_bool_exn _instance _model expr =
      eval_string_hack expr
        ~f_normal:B.is_bv_const_one
        ~f_string:(function
            | "0" -> false
            | "1" -> true
            | _ -> assert false)
  end

  let backend_name = "boolector"

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

  let assert_ _ expr = B.assert_ expr

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

  let assert_not _ expr = B.assert_ (Boolean.not expr)

end

and Bitvector_t : Smtcaml_intf.Bitvector
  with module Types := Types
   and module Op_types := Op_types
= struct
  open T

  let sort_bitvector t length =
    B.bitvec_sort t length

  module Bv = struct
    module Set = Smtcaml_utils.Set_ops.Make(T)

    module Sort = struct
      let length sort = B.bitvec_sort_get_width sort
    end

    module Model = struct
      let eval_exn instance model expr =
        Model.eval_to_string_exn instance model expr
        |> Fast_bitvector.Little_endian.of_string
    end

    let length e = Sort.length (Expr.sort e)

    module Numeral = struct
      let int sort i = B.int (get_sort_context sort) i sort

      let int_e e i = int (Expr.sort e) i

      let fast_bitvector sort bv =
        assert (Sort.length sort = Fast_bitvector.length bv);
        B.const
          (get_sort_context sort)
          (Fast_bitvector.Little_endian.to_string bv)

      let zero ~length t = B.zero (sort_bitvector t length)

      let zero_e e = B.zero (Expr.sort e)
    end

    module Signed = struct
      let (==) = Boolean.eq
      let (<>) = Boolean.neq
      let (<) = B.slt
      let (<=) = B.slte
      let (>) = B.sgt
      let (>=) = B.sgte
      let (+) = Bitvector_t.Bv.add
      let (-) = Bitvector_t.Bv.sub
      let (~-) = B.neg
      let ( * ) = B.mul
      let (/) = B.sdiv
      let (mod) = B.srem
    end

    module Unsigned = struct
      let (==) = Boolean.eq
      let (<>) = Boolean.neq
      let (<) = B.ult
      let (<=) = B.ulte
      let (>) = B.ugt
      let (>=) = B.ugte
      let (+) = Bitvector_t.Bv.add
      let (-) = Bitvector_t.Bv.sub
      let ( * ) = B.mul
      let (/) = B.udiv
      let (mod) = B.urem
    end

    let extract ~low ~high e = B.slice e high low

    let extract_single ~bit e = extract ~low:bit ~high:bit e

    let concat e0 e1 = B.concat e0 e1

    let concat_list l = List.reduce_balanced_exn l ~f:B.concat

    let repeat ~count e = B.repeat e count

    let broadcast_single_bit sort bit =
      let target_size = Bitvector_t.Bv.Sort.length sort in
      assert (length bit = 1);
      repeat ~count:target_size bit

    let zero_extend ~extra_zeros e = B.uext e extra_zeros

    let sign_extend ~extra_bits e = B.sext e extra_bits

    let sign e =
      let length = length e in
      extract_single ~bit:(length - 1) e

    let not = B.not
    let and_ = B.and_
    let or_ = B.or_
    let xor = B.xor

    let and_list = List.reduce_balanced_exn ~f:and_
    let or_list = List.reduce_balanced_exn ~f:or_
    let xor_list = List.reduce_balanced_exn ~f:xor

    let add = B.add
    let sub = B.sub

    let inc = B.inc
    let dec = B.dec

    let is_zero e =
      B.eq (Numeral.zero_e e) e

    let is_not_zero e =
      B.ne (Numeral.zero_e e) e

    let is_all_ones e =
      let sort = Expr.sort e in
      B.eq (B.not (B.zero sort)) e

    include Smtcaml_utils.Power_of_two.Make(T)

    let parity e = B.redxor e

    include Smtcaml_utils.Popcount.Make(T)

    include Smtcaml_utils.Add_sub_over_under_flow.Make(T)

    let shift_left ~count e = B.sll e count
    let shift_right_logical ~count e = B.srl e count
    let shift_right_arithmetic ~count e = B.sra e count

    let of_bool b = b
  end

end

and Uf_t : Smtcaml_intf.Uninterpreted_function
  with module Types := Types
   and module Op_types := Op_types
= struct
  let sort_uninterpreted_function t ~domain ~codomain =
    B.fun_sort_vector t [| domain |] codomain

  module Ufun = struct
    module Model = struct
      (* This is a very bad hack because we're not allowed to do the correct
         conversion. Maybe the "convert" functions should not be functions, but
         GADTs instead, so that we can make this safe *)
      let eval_to_list_exn instance model expr convert_domain convert_codomain =
        let array = B.uf_assignment_vector expr in
        let else_ = ref None in
        let acc = ref [] in
        Array.iter array
          ~f:(fun (left, right) ->
              match left with
              | "*" -> else_ := Some (convert_codomain instance model (Obj.magic right))
              | _ ->
                acc :=
                  ( convert_domain instance model (Obj.magic left)
                  , convert_codomain instance model (Obj.magic right)
                  ) :: !acc
            );
        { Smtcaml_intf.Ufun_interp.
          values = !acc
        ; else_val = !else_
        }
    end

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

