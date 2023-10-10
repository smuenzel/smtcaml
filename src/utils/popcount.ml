open Core

module Make(P : Smtcaml_intf.Interface_definitions.Bitvector_basic) = struct
  open P
  open Bv

  let popcount ?result_bit_size e =
    let size = length e in
    let result_bit_size = Option.value ~default:size result_bit_size in
    let acc_bit_size = Int.ceil_log2 size in
    let extra_zeros = result_bit_size - acc_bit_size in
    let final_extend = if extra_zeros = 0 then Fn.id else zero_extend ~extra_zeros in
    assert (acc_bit_size <= result_bit_size);
    List.init size
      ~f:(fun bit ->
          extract_single ~bit e
          |> zero_extend ~extra_zeros:(acc_bit_size - 1)
        )
    |> List.reduce_balanced_exn ~f:add
    |> final_extend
end
