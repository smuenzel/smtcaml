
module Make(P : Smtcaml_intf.Interface_definitions.Bitvector_basic) = struct
  open P
  open Bv

  let is_power_of_two_or_zero e =
    and_ e (sub e (Numeral.int_e e 1))
    |> is_zero

  let is_power_of_two e =
    Boolean.and_
      (is_power_of_two_or_zero e)
      (is_not_zero e)
end
