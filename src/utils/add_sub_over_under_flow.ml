
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

  let is_sub_overflow a b =
    let diff = sub a b in
    Boolean.and_list
      [ is_zero (sign a)
      ; is_not_zero (sign b)
      ; is_not_zero (sign diff)
      ]

  let is_sub_underflow_unsigned a b =
    let a' = zero_extend ~extra_zeros:1 a in
    let b' = zero_extend ~extra_zeros:1 b in
    let diff = sub a' b' in
    is_not_zero (sign diff)

  let is_sub_underflow_signed a b =
    let sa = sign a in
    let sb = sign b in
    let diff = sub a b in
    Boolean.and_list
      [ is_not_zero sa
      ; is_zero sb
      ; is_zero (sign diff)
      ]

  let is_sub_underflow ~signed =
    match signed with
    | false -> is_sub_underflow_unsigned
    | true -> is_sub_underflow_signed
end
