open! Base

type solver
type sort
type term
type sat_result

type uint32_t = int
type int32_t = int
type uint64_t = int

(*$ open! Core
  open Cppcaml_lib
  (* dune cinaps currently doesn't support adding link flags, and we need -linkall,
     this is a workaround *)
  external unit_x : unit -> unit = "caml_cvc5_unit"
    *)(*$*)

(*$
  let () =
    emit_enums ()
  *)
type kind =
  | INTERNAL_KIND
  | UNDEFINED_KIND
  | NULL_TERM
  | UNINTERPRETED_SORT_VALUE
  | EQUAL
  | DISTINCT
  | CONSTANT
  | VARIABLE
  | SEXPR
  | LAMBDA
  | WITNESS
  | CONST_BOOLEAN
  | NOT
  | AND
  | IMPLIES
  | OR
  | XOR
  | ITE
  | APPLY_UF
  | CARDINALITY_CONSTRAINT
  | HO_APPLY
  | ADD
  | MULT
  | IAND
  | POW2
  | SUB
  | NEG
  | DIVISION
  | INTS_DIVISION
  | INTS_MODULUS
  | ABS
  | POW
  | EXPONENTIAL
  | SINE
  | COSINE
  | TANGENT
  | COSECANT
  | SECANT
  | COTANGENT
  | ARCSINE
  | ARCCOSINE
  | ARCTANGENT
  | ARCCOSECANT
  | ARCSECANT
  | ARCCOTANGENT
  | SQRT
  | DIVISIBLE
  | CONST_RATIONAL
  | CONST_INTEGER
  | LT
  | LEQ
  | GT
  | GEQ
  | IS_INTEGER
  | TO_INTEGER
  | TO_REAL
  | PI
  | CONST_BITVECTOR
  | BITVECTOR_CONCAT
  | BITVECTOR_AND
  | BITVECTOR_OR
  | BITVECTOR_XOR
  | BITVECTOR_NOT
  | BITVECTOR_NAND
  | BITVECTOR_NOR
  | BITVECTOR_XNOR
  | BITVECTOR_COMP
  | BITVECTOR_MULT
  | BITVECTOR_ADD
  | BITVECTOR_SUB
  | BITVECTOR_NEG
  | BITVECTOR_UDIV
  | BITVECTOR_UREM
  | BITVECTOR_SDIV
  | BITVECTOR_SREM
  | BITVECTOR_SMOD
  | BITVECTOR_SHL
  | BITVECTOR_LSHR
  | BITVECTOR_ASHR
  | BITVECTOR_ULT
  | BITVECTOR_ULE
  | BITVECTOR_UGT
  | BITVECTOR_UGE
  | BITVECTOR_SLT
  | BITVECTOR_SLE
  | BITVECTOR_SGT
  | BITVECTOR_SGE
  | BITVECTOR_ULTBV
  | BITVECTOR_SLTBV
  | BITVECTOR_ITE
  | BITVECTOR_REDOR
  | BITVECTOR_REDAND
  | BITVECTOR_UADDO
  | BITVECTOR_SADDO
  | BITVECTOR_UMULO
  | BITVECTOR_SMULO
  | BITVECTOR_USUBO
  | BITVECTOR_SSUBO
  | BITVECTOR_SDIVO
  | BITVECTOR_EXTRACT
  | BITVECTOR_REPEAT
  | BITVECTOR_ZERO_EXTEND
  | BITVECTOR_SIGN_EXTEND
  | BITVECTOR_ROTATE_LEFT
  | BITVECTOR_ROTATE_RIGHT
  | INT_TO_BITVECTOR
  | BITVECTOR_TO_NAT
  | CONST_FINITE_FIELD
  | FINITE_FIELD_NEG
  | FINITE_FIELD_ADD
  | FINITE_FIELD_MULT
  | CONST_FLOATINGPOINT
  | CONST_ROUNDINGMODE
  | FLOATINGPOINT_FP
  | FLOATINGPOINT_EQ
  | FLOATINGPOINT_ABS
  | FLOATINGPOINT_NEG
  | FLOATINGPOINT_ADD
  | FLOATINGPOINT_SUB
  | FLOATINGPOINT_MULT
  | FLOATINGPOINT_DIV
  | FLOATINGPOINT_FMA
  | FLOATINGPOINT_SQRT
  | FLOATINGPOINT_REM
  | FLOATINGPOINT_RTI
  | FLOATINGPOINT_MIN
  | FLOATINGPOINT_MAX
  | FLOATINGPOINT_LEQ
  | FLOATINGPOINT_LT
  | FLOATINGPOINT_GEQ
  | FLOATINGPOINT_GT
  | FLOATINGPOINT_IS_NORMAL
  | FLOATINGPOINT_IS_SUBNORMAL
  | FLOATINGPOINT_IS_ZERO
  | FLOATINGPOINT_IS_INF
  | FLOATINGPOINT_IS_NAN
  | FLOATINGPOINT_IS_NEG
  | FLOATINGPOINT_IS_POS
  | FLOATINGPOINT_TO_FP_FROM_IEEE_BV
  | FLOATINGPOINT_TO_FP_FROM_FP
  | FLOATINGPOINT_TO_FP_FROM_REAL
  | FLOATINGPOINT_TO_FP_FROM_SBV
  | FLOATINGPOINT_TO_FP_FROM_UBV
  | FLOATINGPOINT_TO_UBV
  | FLOATINGPOINT_TO_SBV
  | FLOATINGPOINT_TO_REAL
  | SELECT
  | STORE
  | CONST_ARRAY
  | EQ_RANGE
  | APPLY_CONSTRUCTOR
  | APPLY_SELECTOR
  | APPLY_TESTER
  | APPLY_UPDATER
  | MATCH
  | MATCH_CASE
  | MATCH_BIND_CASE
  | TUPLE_PROJECT
  | SEP_NIL
  | SEP_EMP
  | SEP_PTO
  | SEP_STAR
  | SEP_WAND
  | SET_EMPTY
  | SET_UNION
  | SET_INTER
  | SET_MINUS
  | SET_SUBSET
  | SET_MEMBER
  | SET_SINGLETON
  | SET_INSERT
  | SET_CARD
  | SET_COMPLEMENT
  | SET_UNIVERSE
  | SET_COMPREHENSION
  | SET_CHOOSE
  | SET_IS_SINGLETON
  | SET_MAP
  | SET_FILTER
  | SET_FOLD
  | RELATION_JOIN
  | RELATION_PRODUCT
  | RELATION_TRANSPOSE
  | RELATION_TCLOSURE
  | RELATION_JOIN_IMAGE
  | RELATION_IDEN
  | RELATION_GROUP
  | RELATION_AGGREGATE
  | RELATION_PROJECT
  | BAG_EMPTY
  | BAG_UNION_MAX
  | BAG_UNION_DISJOINT
  | BAG_INTER_MIN
  | BAG_DIFFERENCE_SUBTRACT
  | BAG_DIFFERENCE_REMOVE
  | BAG_SUBBAG
  | BAG_COUNT
  | BAG_MEMBER
  | BAG_DUPLICATE_REMOVAL
  | BAG_MAKE
  | BAG_CARD
  | BAG_CHOOSE
  | BAG_IS_SINGLETON
  | BAG_FROM_SET
  | BAG_TO_SET
  | BAG_MAP
  | BAG_FILTER
  | BAG_FOLD
  | BAG_PARTITION
  | TABLE_PRODUCT
  | TABLE_PROJECT
  | TABLE_AGGREGATE
  | TABLE_JOIN
  | TABLE_GROUP
  | STRING_CONCAT
  | STRING_IN_REGEXP
  | STRING_LENGTH
  | STRING_SUBSTR
  | STRING_UPDATE
  | STRING_CHARAT
  | STRING_CONTAINS
  | STRING_INDEXOF
  | STRING_INDEXOF_RE
  | STRING_REPLACE
  | STRING_REPLACE_ALL
  | STRING_REPLACE_RE
  | STRING_REPLACE_RE_ALL
  | STRING_TO_LOWER
  | STRING_TO_UPPER
  | STRING_REV
  | STRING_TO_CODE
  | STRING_FROM_CODE
  | STRING_LT
  | STRING_LEQ
  | STRING_PREFIX
  | STRING_SUFFIX
  | STRING_IS_DIGIT
  | STRING_FROM_INT
  | STRING_TO_INT
  | CONST_STRING
  | STRING_TO_REGEXP
  | REGEXP_CONCAT
  | REGEXP_UNION
  | REGEXP_INTER
  | REGEXP_DIFF
  | REGEXP_STAR
  | REGEXP_PLUS
  | REGEXP_OPT
  | REGEXP_RANGE
  | REGEXP_REPEAT
  | REGEXP_LOOP
  | REGEXP_NONE
  | REGEXP_ALL
  | REGEXP_ALLCHAR
  | REGEXP_COMPLEMENT
  | SEQ_CONCAT
  | SEQ_LENGTH
  | SEQ_EXTRACT
  | SEQ_UPDATE
  | SEQ_AT
  | SEQ_CONTAINS
  | SEQ_INDEXOF
  | SEQ_REPLACE
  | SEQ_REPLACE_ALL
  | SEQ_REV
  | SEQ_PREFIX
  | SEQ_SUFFIX
  | CONST_SEQUENCE
  | SEQ_UNIT
  | SEQ_NTH
  | FORALL
  | EXISTS
  | VARIABLE_LIST
  | INST_PATTERN
  | INST_NO_PATTERN
  | INST_POOL
  | INST_ADD_TO_POOL
  | SKOLEM_ADD_TO_POOL
  | INST_ATTRIBUTE
  | INST_PATTERN_LIST
  | LAST_KIND
[@@deriving sexp]

type sortKind =
  | INTERNAL_SORT_KIND
  | UNDEFINED_SORT_KIND
  | NULL_SORT
  | ABSTRACT_SORT
  | ARRAY_SORT
  | BAG_SORT
  | BOOLEAN_SORT
  | BITVECTOR_SORT
  | DATATYPE_SORT
  | FINITE_FIELD_SORT
  | FLOATINGPOINT_SORT
  | FUNCTION_SORT
  | INTEGER_SORT
  | REAL_SORT
  | REGLAN_SORT
  | ROUNDINGMODE_SORT
  | SEQUENCE_SORT
  | SET_SORT
  | STRING_SORT
  | TUPLE_SORT
  | UNINTERPRETED_SORT
  | LAST_SORT_KIND
[@@deriving sexp]

type unknownExplanation =
  | REQUIRES_FULL_CHECK
  | INCOMPLETE
  | TIMEOUT
  | RESOURCEOUT
  | MEMOUT
  | INTERRUPTED
  | UNSUPPORTED
  | OTHER
  | REQUIRES_CHECK_AGAIN
  | UNKNOWN_REASON
[@@deriving sexp]

(*$*)

(*$
  let () =
    let modify s =
      Option.value ~default:s (String.chop_prefix ~prefix:"cvc5_" s)
      |> filter_keyword
    in
    emit_api ()
      ~modify
  *)
external make_Solver : unit -> solver = "caml_cvc5__Solver__operator_new"
external solver__getBooleanSort : (* class Solver *) solver -> sort = "caml_cvc5__Solver__getBooleanSort"
external solver__getIntegerSort : (* class Solver *) solver -> sort = "caml_cvc5__Solver__getIntegerSort"
external solver__getRealSort : (* class Solver *) solver -> sort = "caml_cvc5__Solver__getRealSort"
external solver__getRegExpSort : (* class Solver *) solver -> sort = "caml_cvc5__Solver__getRegExpSort"
external solver__getRoundingModeSort : (* class Solver *) solver -> sort = "caml_cvc5__Solver__getRoundingModeSort"
external solver__getStringSort : (* class Solver *) solver -> sort = "caml_cvc5__Solver__getStringSort"
external solver__mkArraySort : (* class Solver *) solver -> sort -> sort -> sort = "caml_cvc5__Solver__mkArraySort"
external solver__mkBitVectorSort : (* class Solver *) solver -> uint32_t -> sort = "caml_cvc5__Solver__mkBitVectorSort"
external solver__mkFloatingPointSort : (* class Solver *) solver -> uint32_t -> uint32_t -> sort = "caml_cvc5__Solver__mkFloatingPointSort"
external solver__mkFiniteFieldSort : (* class Solver *) solver -> string -> sort = "caml_cvc5__Solver__mkFiniteFieldSort"
external solver__mkFunctionSort : (* class Solver *) solver -> (sort array) -> sort -> sort = "caml_cvc5__Solver__mkFunctionSort"
external solver__mkPredicateSort : (* class Solver *) solver -> (sort array) -> sort = "caml_cvc5__Solver__mkPredicateSort"
external solver__mkRecordSort : (* class Solver *) solver -> ((string*sort) array) -> sort = "caml_cvc5__Solver__mkRecordSort"
external solver__mkSetSort : (* class Solver *) solver -> sort -> sort = "caml_cvc5__Solver__mkSetSort"
external solver__mkBagSort : (* class Solver *) solver -> sort -> sort = "caml_cvc5__Solver__mkBagSort"
external solver__mkSequenceSort : (* class Solver *) solver -> sort -> sort = "caml_cvc5__Solver__mkSequenceSort"
external solver__mkUninterpretedSort : (* class Solver *) solver -> (string option) -> sort = "caml_cvc5__Solver__mkUninterpretedSort"
external solver__mkTupleSort : (* class Solver *) solver -> (sort array) -> sort = "caml_cvc5__Solver__mkTupleSort"
external solver__mkBitVector__u32_u64 : (* class Solver *) solver -> uint32_t -> uint64_t -> term = "caml_cvc5__Solver__mkBitVector__overload__u32_u64"
external solver__mkBitVector__u32_s_u32 : (* class Solver *) solver -> uint32_t -> string -> uint32_t -> term = "caml_cvc5__Solver__mkBitVector__overload__u32_s_u32"
external solver__mkTrue : (* class Solver *) solver -> term = "caml_cvc5__Solver__mkTrue"
external solver__mkFalse : (* class Solver *) solver -> term = "caml_cvc5__Solver__mkFalse"
external solver__mkBoolean : (* class Solver *) solver -> bool -> term = "caml_cvc5__Solver__mkBoolean"
external solver__mkConstArray : (* class Solver *) solver -> sort -> term -> term = "caml_cvc5__Solver__mkConstArray"
external solver__mkConst : (* class Solver *) solver -> sort -> (string option) -> term = "caml_cvc5__Solver__mkConst"
external solver__mkVar : (* class Solver *) solver -> sort -> (string option) -> term = "caml_cvc5__Solver__mkVar"
external solver__simplify : (* class Solver *) solver -> term -> term = "caml_cvc5__Solver__simplify"
external solver__assertFormula : (* class Solver *) solver -> term -> unit = "caml_cvc5__Solver__assertFormula"
external solver__checkSat : (* class Solver *) solver -> sat_result = "caml_cvc5__Solver__checkSat"
external solver__setOption : (* class Solver *) solver -> string -> string -> unit = "caml_cvc5__Solver__setOption"
external sort__getKind : (* class Sort *) sort -> sortKind = "caml_cvc5__Sort__getKind"
external sort__hasSymbol : (* class Sort *) sort -> bool = "caml_cvc5__Sort__hasSymbol"
external sort__getSymbol : (* class Sort *) sort -> string = "caml_cvc5__Sort__getSymbol"
external sort__isNull : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isNull"
external sort__isBoolean : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isBoolean"
external sort__isInteger : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isInteger"
external sort__isReal : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isReal"
external sort__isInstantiated : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isInstantiated"
external result__isNull : (* class Result *) sat_result -> bool = "caml_cvc5__Result__isNull"
external result__isSat : (* class Result *) sat_result -> bool = "caml_cvc5__Result__isSat"
external result__isUnsat : (* class Result *) sat_result -> bool = "caml_cvc5__Result__isUnsat"
external result__isUnknown : (* class Result *) sat_result -> bool = "caml_cvc5__Result__isUnknown"
external result__toString : (* class Result *) sat_result -> string = "caml_cvc5__Result__toString"
external result__getUnknownExplanation : (* class Result *) sat_result -> unknownExplanation = "caml_cvc5__Result__getUnknownExplanation"
(*$*)


