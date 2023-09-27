open! Core

module Sort_kind = struct
  type bv = [ `Bv ]
  type bool = [ `Bool ]
  type ('a, 'b) array = [ `Array of 'a * 'b ]

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

  type ('i, 's) unary = ('i, 's) Types.expr -> ('i, 's) Types.expr
  type ('i, 's) binary = ('i, 's) Types.expr -> ('i, 's) unary
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
  end

  module Model : sig
    type 'i t' := 'i t
    type 'i t = 'i Types.model

    val eval_to_string : 'i t' -> 'i t -> ('i, 's) Expr.t -> string option
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
    val not : ('i, m_sort) Op_types.unary
    val and_  : ('i, m_sort) Op_types.binary
    val or_  : ('i, m_sort) Op_types.binary
    val xor  : ('i, m_sort) Op_types.binary

    (* val to_bool : ('i, m_sort) Types.expr -> ('i, Sort_kind.bool) Types.expr *)
    val of_bool : ('i, Sort_kind.bool) Types.expr -> ('i, m_sort) Types.expr
  end
end

module type Boolean = sig
  module Types : Types
  module Op_types : Op_types with module Types := Types

  type m_sort := Sort_kind.bool

  val sort_boolean : 'i Types.instance -> ('i, m_sort) Types.sort

  val assert_ : ('i, m_sort) Types.expr -> unit

  module Boolean : sig
    val eq : ('i, 's) Types.expr -> ('i, 's) Types.expr -> ('i, m_sort) Types.expr
    val neq : ('i, 's) Types.expr -> ('i, 's) Types.expr -> ('i, m_sort) Types.expr
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
