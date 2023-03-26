open! Core

module Sort_kind = struct
  type bv = [ `Bv ]
  type bool = [ `Bool ]
  type ('a, 'b) array = [ `Array of 'a * 'b ]

end

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
