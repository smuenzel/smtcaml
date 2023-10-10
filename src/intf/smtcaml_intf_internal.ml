open! Core

module Sort_kind = struct
  type bv = [ `Bv ]
  type bool = [ `Bool ]
  type ('a, 'b) array = [ `Array of 'a * 'b ]

  type 'a ufun = [ `Ufun of 'a ]

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

module type Ordering = sig
  module Types : Types
  module Op_types : Op_types with module Types := Types
  val (==) : ('i, 's) Op_types.binary_test
  val (<>) : ('i, 's) Op_types.binary_test
  val (>) : ('i, 's) Op_types.binary_test
  val (>=) : ('i, 's) Op_types.binary_test
  val (<) : ('i, 's) Op_types.binary_test
  val (<=) : ('i, 's) Op_types.binary_test
end

module type Arithmetic_ops = sig
  module Types : Types
  module Op_types : Op_types with module Types := Types
  val (+) : ('i, 's) Op_types.binary
  val (-) : ('i, 's) Op_types.binary
  val ( * ) : ('i, 's) Op_types.binary
  val ( / ) : ('i, 's) Op_types.binary
  val ( mod ) : ('i, 's) Op_types.binary
end

module type Arithmetic_ops_signed = sig
  include Arithmetic_ops
  val (~-) : ('i, 's) Op_types.unary
end

module Make_op_types (Types : Types) : Op_types with module Types := Types = struct
  module rec T : Op_types with module Types := Types = T
  include T
end

module Base_modules = struct
  module type Sort = sig
    module Types : Types

    type ('i, 's) t = ('i, 's) Types.sort
  end

  module type Expr = sig
    module Types : Types

    type ('i, 's) t = ('i, 's) Types.expr

    val sort : ('i, 's) t -> ('i, 's) Types.sort
  end

  module type Model = sig
    module Types : Types

    type 'i t = 'i Types.model

    val eval_to_string : 'i Types.instance -> 'i t -> ('i, 's) Types.expr -> string option

    val eval_bool : 'i Types.instance -> 'i t -> ('i, Sort_kind.bool) Types.expr -> bool option
  end

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

  val backend_name : string

  val create : ?options:Options.t -> unit -> Packed.t

  val var : ('i,'s) Types.sort -> string -> ('i, 's) Types.expr
  val var_anon : ('i,'s) Types.sort -> ('i, 's) Types.expr

  val check_current_and_get_model : 'i t -> 'i Types.model Solver_result.t
end

module type Backend = sig
  include Backend_base

  module Sort : Base_modules.Sort with module Types := Types
  module Expr : Base_modules.Expr with module Types := Types
  module Model : Base_modules.Model with module Types := Types
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

    module Model : sig
      val eval : 'i Types.instance -> 'i Types.model -> ('i, Sort_kind.bv) Types.expr -> Fast_bitvector.t option
    end

    val length : ('i, m_sort) Types.expr -> int

    module Numeral : sig
      val int : ('i, m_sort) Types.sort -> int -> ('i, m_sort) Types.expr
      val int_e : ('i, m_sort) Types.expr -> int -> ('i, m_sort) Types.expr
      val fast_bitvector : ('i, m_sort) Types.sort -> Fast_bitvector.t -> ('i, m_sort) Types.expr 
      val zero : length:int -> 'i Types.instance -> ('i, m_sort) Types.expr
      val zero_e : ('i, m_sort) Op_types.unary
    end

    module Signed : sig
      include Ordering
        with module Types := Types
         and module Op_types := Op_types

      include Arithmetic_ops_signed
        with module Types := Types
         and module Op_types := Op_types
    end

    module Unsigned : sig
      include Ordering
        with module Types := Types
         and module Op_types := Op_types

      include Arithmetic_ops
        with module Types := Types
         and module Op_types := Op_types
    end

    val extract : low:int -> high:int -> ('i, m_sort) Op_types.unary
    val extract_single : bit:int -> ('i, m_sort) Op_types.unary
    val concat : ('i, m_sort) Op_types.binary
    val concat_list : ('i, m_sort) Op_types.redlist
    val repeat : count:int -> ('i, m_sort) Op_types.unary
    val broadcast_single_bit : ('i, m_sort) Types.sort -> ('i, m_sort) Op_types.unary
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
    val is_power_of_two_or_zero : ('i, m_sort) Op_types.unary_test
    val is_power_of_two : ('i, m_sort) Op_types.unary_test
    val sign : ('i, m_sort) Op_types.unary
    val parity : ('i, m_sort) Op_types.unary

    val popcount : ?result_bit_size:int -> ('i, m_sort) Op_types.unary

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

    module Set : sig
      val const_empty : 'i Types.instance -> int -> ('i, m_sort) Types.expr

      val union : ('i, m_sort) Op_types.binary
      val inter : ('i, m_sort) Op_types.binary
      val complement : ('i, m_sort) Op_types.unary
      val diff : ('i, m_sort) Op_types.binary
      val symmdiff : ('i, m_sort) Op_types.binary

      val is_empty : ('i, m_sort) Op_types.unary_test
      val is_subset : ('i, m_sort) Types.expr -> of_:('i, m_sort) Types.expr  -> ('i, Sort_kind.bool) Types.expr 
      val has_max_one_member : ('i, m_sort) Op_types.unary_test
      val has_single_member : ('i, m_sort) Op_types.unary_test
    end
  end
end

module type Boolean = sig
  module Types : Types
  module Op_types : Op_types with module Types := Types

  type m_sort := Sort_kind.bool

  val sort_boolean : 'i Types.instance -> ('i, m_sort) Types.sort

  val assert_ : 'i Types.instance -> ('i, m_sort) Types.expr -> unit
  val assert_not : 'i Types.instance -> ('i, m_sort) Types.expr -> unit

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
    -> ('i, ('d -> 'cd) Sort_kind.ufun) Types.sort

  module Ufun : sig
    val apply
      : ('i, ('d -> 'cd) Sort_kind.ufun) Types.expr
      -> ('i, 'd) Types.expr
      -> ('i, 'cd) Types.expr
  end
end
