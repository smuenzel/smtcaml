

module Make(P : Smtcaml_intf.Interface_definitions.Bitvector_basic) = struct
  open P
  open Bv

  let const_empty t length = Numeral.zero ~length t

  let union = or_
  let inter = and_
  let complement = not
  let diff a b = inter a (complement b)
  let symmdiff = xor

  let is_empty = is_zero
  let is_subset a ~of_ = is_empty (diff a of_)
  let has_max_one_member = is_power_of_two_or_zero
  let has_single_member = is_power_of_two
end
