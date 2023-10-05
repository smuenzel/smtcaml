
module Sort_kind = Smtcaml_intf.Sort_kind

module type P = sig
  include Smtcaml_intf.Backend_base

  type m_sort := Sort_kind.bv

  val zero_extend : extra_zeros:int -> ('i, m_sort) Op_types.unary

  val add : ('i, m_sort) Op_types.binary
  val sub : ('i, m_sort) Op_types.binary

  val is_not_zero : ('i, m_sort) Op_types.unary_test

  val sign : ('i, m_sort) Op_types.unary
end

module Make(P : P) = struct
  open P

  let add_overflow_unsigned a b =
    let a' = zero_extend ~extra_zeros:1 a in
    let b' = zero_extend ~extra_zeros:1 b in
    let sum = add a' b' in
    is_not_zero (sign sum)
end
