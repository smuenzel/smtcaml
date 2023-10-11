open! Core

module Sort_kind = Smtcaml_intf.Sort_kind

module Make(P : Smtcaml_intf.Interface_definitions.Bitvector_basic) = struct
  open P
  open Bv

  type 'i t =
    { input_count : int
    ; input_selector : ('i, Sort_kind.bv) Types.expr
    ; output : ('i, Sort_kind.bv) Types.expr
    ; constraints : ('i, Sort_kind.bool) Types.expr
    }

  let create instance inputs =
    let first = List.hd_exn inputs in
    let input_length = Bv.length first in
    assert (List.for_all inputs ~f:(fun a -> input_length = (Bv.length a)));
    let input_count = List.length inputs in
    let selector_sort = sort_bitvector instance input_count in
    let input_selector = var_anon selector_sort in
    let constraints =
      Set.has_single_member input_selector
    in
    let input_sort = Expr.sort first in
    let output =
      List.mapi inputs
        ~f:(fun i input ->
            and_
              (broadcast_single_bit
                 input_sort
                 (extract_single ~bit:i input_selector)
              )
              input
          )
      |> or_list
    in
    { input_count
    ; input_selector
    ; output
    ; constraints
    }
end
