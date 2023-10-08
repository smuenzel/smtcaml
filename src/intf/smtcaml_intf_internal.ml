open! Core

module Sort_kind = struct
  type bv = [ `Bv ]
  type bool = [ `Bool ]
  type ('a, 'b) array = [ `Array of 'a * 'b ]

  type ('a, 'b) ufun = [ `Ufun of 'a * 'b ]

end

module Solver_result = struct
  type 'a t =
    | Unsatisfiable
    | Unknown of string
    | Satisfiable of 'a
  [@@deriving sexp]
end

module type Types = sig
  type 'instance instance
  type ('instance, 'sort) sort
  type ('instance, 'sort) expr
  type 'instance model
end

module type Op_types = sig
  module Types : Types

  type ('i, 's) redlist = ('i, 's) Types.expr list -> ('i, 's) Types.expr

  type ('i, 's) unary = ('i, 's) Types.expr -> ('i, 's) Types.expr
  type ('i, 's) binary = ('i, 's) Types.expr -> ('i, 's) unary

  type ('i, 's) unary_test = ('i, 's) Types.expr -> ('i, Sort_kind.bool) Types.expr
  type ('i, 's) binary_test = ('i, 's) Types.expr -> ('i, 's) unary_test
end

module Make_op_types (Types : Types) : Op_types with module Types := Types = struct
  module rec T : Op_types with module Types := Types = T
  include T
end

module type Backend_base = sig
  module Types : Types

  module Op_types : Op_types with module Types := Types

  module Options : sig
    type t [@@deriving sexp]
  end

  type 'instance t = 'instance Types.instance

  module Packed : sig
    type t = T : _ Types.instance -> t
  end

  module Sort : sig
    type ('i, 's) t = ('i, 's) Types.sort
  end

  module Expr : sig
    type ('i, 's) t = ('i, 's) Types.expr

    val sort : ('i, 's) t -> ('i, 's) Sort.t
  end

  module Model : sig
    type 'i t' := 'i t
    type 'i t = 'i Types.model

    val eval_to_string : 'i t' -> 'i t -> ('i, 's) Expr.t -> string option

    val eval_bitvector : 'i t' -> 'i t -> ('i, Sort_kind.bv) Expr.t -> Fast_bitvector.t option

    val eval_bool : 'i t' -> 'i t -> ('i, Sort_kind.bool) Expr.t -> bool option
  end

  val create : ?options:Options.t -> unit -> Packed.t

  val var : ('i,'s) Sort.t -> string -> ('i, 's) Expr.t

  val check_current_and_get_model : 'i t -> 'i Model.t Solver_result.t
end

module type Bitvector = sig
  module Types : Types
  module Op_types : Op_types with module Types := Types

  type m_sort := Sort_kind.bv

  val sort_bitvector : 'i Types.instance -> int -> ('i, m_sort) Types.sort

  module Bv : sig
    module Sort : sig
      val length : ('i, m_sort) Types.sort -> int
    end

    val length : ('i, m_sort) Types.expr -> int

    module Numeral : sig
      val int : ('i, m_sort) Types.sort -> int -> ('i, m_sort) Types.expr
      val fast_bitvector : ('i, m_sort) Types.sort -> Fast_bitvector.t -> ('i, m_sort) Types.expr 
      val zero : length:int -> 'i Types.instance -> ('i, m_sort) Types.expr
      val zero_e : ('i, m_sort) Op_types.unary
    end

    val extract : low:int -> high:int -> ('i, m_sort) Op_types.unary
    val extract_single : bit:int -> ('i, m_sort) Op_types.unary
    val concat : ('i, m_sort) Op_types.binary
    val zero_extend : extra_zeros:int -> ('i, m_sort) Op_types.unary
    val sign_extend : extra_bits:int -> ('i, m_sort) Op_types.unary

    val not : ('i, m_sort) Op_types.unary
    val and_  : ('i, m_sort) Op_types.binary
    val or_  : ('i, m_sort) Op_types.binary
    val xor  : ('i, m_sort) Op_types.binary

    val add : ('i, m_sort) Op_types.binary
    val sub : ('i, m_sort) Op_types.binary

    val is_zero : ('i, m_sort) Op_types.unary_test
    val is_not_zero : ('i, m_sort) Op_types.unary_test
    val is_all_ones : ('i, m_sort) Op_types.unary_test
    val sign : ('i, m_sort) Op_types.unary
    val parity : ('i, m_sort) Op_types.unary

    (* a + b > max *)
    val is_add_overflow : signed:bool -> ('i, m_sort) Op_types.binary_test
    (* sign(a) = sign(b) = -1, sign(a+b) <> -1*)
    val is_add_underflow : ('i, m_sort) Op_types.binary_test

    (* a - b > max *)
    val is_sub_overflow : ('i, m_sort) Op_types.binary_test
    (* sign(a) = -1, sign(b) <> -1, sign(a-b) <> -1 *)
    val is_sub_underflow : signed:bool -> ('i, m_sort) Op_types.binary_test

    val shift_left : count:('i, m_sort) Types.expr -> ('i, m_sort) Op_types.unary 
    val shift_right_logical : count:('i, m_sort) Types.expr -> ('i, m_sort) Op_types.unary 
    val shift_right_arithmetic : count:('i, m_sort) Types.expr -> ('i, m_sort) Op_types.unary 

    val of_bool : ('i, Sort_kind.bool) Types.expr -> ('i, m_sort) Types.expr
  end
end

module type Boolean = sig
  module Types : Types
  module Op_types : Op_types with module Types := Types

  type m_sort := Sort_kind.bool

  val sort_boolean : 'i Types.instance -> ('i, m_sort) Types.sort

  val assert_ : 'i Types.instance -> ('i, m_sort) Types.expr -> unit

  module Boolean : sig
    module Numeral : sig
      val true_ : 'i Types.instance -> ('i, m_sort) Types.expr
      val false_ : 'i Types.instance -> ('i, m_sort) Types.expr
      val bool : 'i Types.instance -> bool -> ('i, m_sort) Types.expr
    end

    val distinct : ('i, 's) Types.expr list -> ('i, m_sort) Types.expr

    val eq : ('i, 's) Types.expr -> ('i, 's) Types.expr -> ('i, m_sort) Types.expr
    val neq : ('i, 's) Types.expr -> ('i, 's) Types.expr -> ('i, m_sort) Types.expr
    val ite : ('i, m_sort) Types.expr ->  ('i, 's) Op_types.binary

    val not : ('i, m_sort) Op_types.unary
    val and_ : ('i, m_sort) Op_types.binary
    val or_ : ('i, m_sort) Op_types.binary

    val and_list : ('i, m_sort) Op_types.redlist
    val or_list : ('i, m_sort) Op_types.redlist

    val implies : ('i, m_sort) Op_types.binary
    val iff : ('i, m_sort) Op_types.binary
  end
end

module type Uninterpreted_function = sig
  module Types : Types

  val sort_uninterpreted_function 
    : 'i Types.instance
    -> domain:('i,'d) Types.sort
    -> codomain:('i, 'cd) Types.sort
    -> ('i, ('d, 'cd) Sort_kind.ufun) Types.sort

  module Ufun : sig
    val apply
      : ('i, ('d, 'cd) Sort_kind.ufun) Types.expr
      -> ('i, 'd) Types.expr
      -> ('i, 'cd) Types.expr
  end
end

(*

module Backend = struct
  type boolector = [ `Boolector ]

  type _ t =
    | Boolector : boolector t

end

module type Smt_instance = sig
  type ('backend, 'e) t

  type 'backend packed = T : ('backend, _) t -> 'backend packed
end

module type Specific_instance = sig

  type 'e t
  type packed = T : _ t -> packed

  val create : unit -> packed

  val same_witness : 'e1 t -> 'e2 t -> ('e1, 'e2) Type_equal.t option

  module Sort : sig
    type 'e instance := 'e t
    type ('e, 'sort) t

    val instance : ('e, _) t -> 'e instance
  end

  module Expr : sig
    type 'e instance := 'e t
    type ('e, 'sort) t

    val instance : ('e, _) t -> 'e instance

    val eq : ('e, 'sort) t -> ('e, 'sort) t -> ('e, Sort_kind.bool) t
    val neq : ('e, 'sort) t -> ('e, 'sort) t -> ('e, Sort_kind.bool) t

    val ite : ('e, Sort_kind.bool) t -> ('e, 'sort) t -> ('e, 'sort) t -> ('e, 'sort) t
  end

  val push : _ t -> unit
  val pop : ?levels:int -> _ t -> unit

  val assert_ : 'e t -> ('e, Sort_kind.bool) Expr.t -> unit
  val assert_not : 'e t -> ('e, Sort_kind.bool) Expr.t -> unit
end

module type Bool_ops = sig
  type 'e instance
  type ('e, 'sort) expr
  type 'e t = ('e, Sort_kind.bool) expr

  val constant : 'e instance -> bool -> 'e t
  val false_val : 'e instance -> 'e t
  val true_val : 'e instance -> 'e t

  type 'e op_2 := 'e t -> 'e t -> 'e t
  val not : 'e t -> 'e t
  val (lor) : 'e op_2
  val (land) : 'e op_2
  val (lxor) : 'e op_2

  val iff : 'e op_2
  val implies : 'e op_2
end

module type Bv_ops = sig

end
   *)
