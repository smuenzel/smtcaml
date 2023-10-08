
module Sort_kind = Smtcaml_intf.Sort_kind

(*
module type P = sig
  include Smtcaml_intf.Backend_base

  module Bv : sig
    type m_sort := Sort_kind.bv

    val zero_extend : extra_zeros:int -> ('i, m_sort) Op_types.unary

    val add : ('i, m_sort) Op_types.binary
    val sub : ('i, m_sort) Op_types.binary

    val is_not_zero : ('i, m_sort) Op_types.unary_test

    val sign : ('i, m_sort) Op_types.unary
  end
end
   *)

module Make(P : Smtcaml_intf.Interface_definitions.Bitvector_basic) = struct
  open P
  open Bv

  let is_add_overflow_unsigned a b =
    let a' = zero_extend ~extra_zeros:1 a in
    let b' = zero_extend ~extra_zeros:1 b in
    let sum = add a' b' in
    is_not_zero (sign sum)

  let is_add_overflow_signed a b =
    let sa = sign a in
    let sb = sign b in
    let sum = add a b in
    Boolean.and_list
      [ is_zero sa
      ; is_zero sb
      ; is_not_zero (sign sum)
      ]

  let is_add_overflow ~signed =
    if signed
    then is_add_overflow_signed
    else is_add_overflow_unsigned

  let is_add_underflow a b =
    let sa = sign a in
    let sb = sign b in
    let sum = add a b in
    Boolean.and_list
      [ is_not_zero sa
      ; is_not_zero sb
      ; is_zero (sign sum)
      ]
end
