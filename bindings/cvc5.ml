open! Base

type solver
type sort
type term
type sat_result
type op
type datatype
type datatype_decl
type datatype_constructor_decl

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

type roundingMode =
  | ROUND_NEAREST_TIES_TO_EVEN
  | ROUND_TOWARD_POSITIVE
  | ROUND_TOWARD_NEGATIVE
  | ROUND_TOWARD_ZERO
  | ROUND_NEAREST_TIES_TO_AWAY
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

type learnedLitType =
  | PREPROCESS_SOLVED
  | PREPROCESS
  | INPUT
  | SOLVABLE
  | CONSTANT_PROP
  | INTERNAL
  | UNKNOWN
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

type findSynthTarget =
  | ENUM
  | REWRITE
  | REWRITE_UNSOUND
  | REWRITE_INPUT
  | QUERY
[@@deriving sexp]

type proofComponent =
  | RAW_PREPROCESS
  | PREPROCESS
  | SAT
  | THEORY_LEMMAS
  | FULL
[@@deriving sexp]

type blockModelsMode =
  | LITERALS
  | VALUES
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
external new_Solver : unit -> solver = "caml_cvc5__new_Solver"
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
external solver__mkDatatypeSort : (* class Solver *) solver -> datatype_decl -> sort = "caml_cvc5__Solver__mkDatatypeSort"
external solver__mkDatatypeSorts : (* class Solver *) solver -> (datatype_decl array) -> (sort array) = "caml_cvc5__Solver__mkDatatypeSorts"
external solver__mkFunctionSort : (* class Solver *) solver -> (sort array) -> sort -> sort = "caml_cvc5__Solver__mkFunctionSort"
external solver__mkParamSort : (* class Solver *) solver -> (string option) -> sort = "caml_cvc5__Solver__mkParamSort"
external solver__mkPredicateSort : (* class Solver *) solver -> (sort array) -> sort = "caml_cvc5__Solver__mkPredicateSort"
external solver__mkRecordSort : (* class Solver *) solver -> ((string*sort) array) -> sort = "caml_cvc5__Solver__mkRecordSort"
external solver__mkSetSort : (* class Solver *) solver -> sort -> sort = "caml_cvc5__Solver__mkSetSort"
external solver__mkBagSort : (* class Solver *) solver -> sort -> sort = "caml_cvc5__Solver__mkBagSort"
external solver__mkSequenceSort : (* class Solver *) solver -> sort -> sort = "caml_cvc5__Solver__mkSequenceSort"
external solver__mkAbstractSort : (* class Solver *) solver -> sortKind -> sort = "caml_cvc5__Solver__mkAbstractSort"
external solver__mkUninterpretedSort : (* class Solver *) solver -> (string option) -> sort = "caml_cvc5__Solver__mkUninterpretedSort"
external solver__mkUnresolvedDatatypeSort : (* class Solver *) solver -> string -> int64 -> sort = "caml_cvc5__Solver__mkUnresolvedDatatypeSort"
external solver__mkUninterpretedSortConstructorSort : (* class Solver *) solver -> int64 -> (string option) -> sort = "caml_cvc5__Solver__mkUninterpretedSortConstructorSort"
external solver__mkTupleSort : (* class Solver *) solver -> (sort array) -> sort = "caml_cvc5__Solver__mkTupleSort"
external solver__mkTerm__kind : (* class Solver *) solver -> kind -> (term array) -> term = "caml_cvc5__Solver__mkTerm__overload__kind"
external solver__mkTerm__op : (* class Solver *) solver -> op -> (term array) -> term = "caml_cvc5__Solver__mkTerm__overload__op"
external solver__mkTuple : (* class Solver *) solver -> (term array) -> term = "caml_cvc5__Solver__mkTuple"
external solver__simplify : (* class Solver *) solver -> term -> term = "caml_cvc5__Solver__simplify"
external solver__assertFormula : (* class Solver *) term -> unit = "caml_cvc5__Solver__assertFormula"
external solver__checkSat : (* class Solver *) solver -> sat_result = "caml_cvc5__Solver__checkSat"
external solver__checkSatAssuming__t : (* class Solver *) solver -> term -> sat_result = "caml_cvc5__Solver__checkSatAssuming__overload__t"
external solver__checkSatAssuming__tv : (* class Solver *) solver -> (term array) -> sat_result = "caml_cvc5__Solver__checkSatAssuming__overload__tv"
external solver__declareDatatype : (* class Solver *) solver -> string -> (datatype_constructor_decl array) -> sort = "caml_cvc5__Solver__declareDatatype"
external solver__declareFun : (* class Solver *) solver -> string -> (sort array) -> sort -> bool -> term = "caml_cvc5__Solver__declareFun"
external solver__declareSort : (* class Solver *) solver -> string -> uint32_t -> bool -> sort = "caml_cvc5__Solver__declareSort"
external solver__defineFun : (* class Solver *) solver -> string -> (term array) -> sort -> term -> bool -> term = "caml_cvc5__Solver__defineFun_bytecode" "caml_cvc5__Solver__defineFun"
external solver__defineFunRec__symbol : (* class Solver *) solver -> string -> (term array) -> sort -> term -> bool -> term = "caml_cvc5__Solver__defineFunRec__overload__symbol_bytecode" "caml_cvc5__Solver__defineFunRec__overload__symbol"
external solver__defineFunRec__term : (* class Solver *) solver -> term -> (term array) -> term -> bool -> term = "caml_cvc5__Solver__defineFunRec__overload__term"
external solver__mkBitVector__u32_u64 : (* class Solver *) solver -> uint32_t -> int64 -> term = "caml_cvc5__Solver__mkBitVector__overload__u32_u64"
external solver__mkBitVector__u32_s_u32 : (* class Solver *) solver -> uint32_t -> string -> uint32_t -> term = "caml_cvc5__Solver__mkBitVector__overload__u32_s_u32"
external solver__mkTrue : (* class Solver *) solver -> term = "caml_cvc5__Solver__mkTrue"
external solver__mkFalse : (* class Solver *) solver -> term = "caml_cvc5__Solver__mkFalse"
external solver__mkBoolean : (* class Solver *) solver -> bool -> term = "caml_cvc5__Solver__mkBoolean"
external solver__mkConstArray : (* class Solver *) solver -> sort -> term -> term = "caml_cvc5__Solver__mkConstArray"
external solver__mkConst : (* class Solver *) sort -> (string option) -> term = "caml_cvc5__Solver__mkConst"
external solver__mkVar : (* class Solver *) solver -> sort -> (string option) -> term = "caml_cvc5__Solver__mkVar"
external solver__getValue__t : (* class Solver *) solver -> term -> term = "caml_cvc5__Solver__getValue__overload__t"
external solver__getValue__vt : (* class Solver *) solver -> (term array) -> (term array) = "caml_cvc5__Solver__getValue__overload__vt"
external solver__setOption : (* class Solver *) solver -> string -> string -> unit = "caml_cvc5__Solver__setOption"
external solver__getOption : (* class Solver *) solver -> string -> string = "caml_cvc5__Solver__getOption"
external solver__mkOp__kv : (* class Solver *) solver -> kind -> (uint32_t array) -> op = "caml_cvc5__Solver__mkOp__overload__kv"
external sort__getKind : (* class Sort *) sort -> sortKind = "caml_cvc5__Sort__getKind"
external sort__hasSymbol : (* class Sort *) sort -> bool = "caml_cvc5__Sort__hasSymbol"
external sort__getSymbol : (* class Sort *) sort -> string = "caml_cvc5__Sort__getSymbol"
external sort__isNull : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isNull"
external sort__isBoolean : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isBoolean"
external sort__isInteger : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isInteger"
external sort__isReal : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isReal"
external sort__isRegExp : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isRegExp"
external sort__isRoundingMode : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isRoundingMode"
external sort__isBitVector : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isBitVector"
external sort__isFloatingPoint : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isFloatingPoint"
external sort__isDatatype : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isDatatype"
external sort__isDatatypeConstructor : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isDatatypeConstructor"
external sort__isDatatypeSelector : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isDatatypeSelector"
external sort__isDatatypeUpdater : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isDatatypeUpdater"
external sort__isFunction : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isFunction"
external sort__isPredicate : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isPredicate"
external sort__isTuple : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isTuple"
external sort__isRecord : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isRecord"
external sort__isArray : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isArray"
external sort__isFiniteField : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isFiniteField"
external sort__isSet : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isSet"
external sort__isBag : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isBag"
external sort__isSequence : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isSequence"
external sort__isAbstract : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isAbstract"
external sort__isUninterpretedSort : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isUninterpretedSort"
external sort__isUninterpretedSortConstructor : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isUninterpretedSortConstructor"
external sort__getUninterpretedSortConstructor : (* class Sort *) sort -> sort = "caml_cvc5__Sort__getUninterpretedSortConstructor"
external sort__getDatatype : (* class Sort *) sort -> datatype = "caml_cvc5__Sort__getDatatype"
external sort__isInstantiated : (* class Sort *) sort -> bool = "caml_cvc5__Sort__isInstantiated"
external sort__instantiate : (* class Sort *) sort -> (sort array) -> sort = "caml_cvc5__Sort__instantiate"
external sort__getInstantiatedParameters : (* class Sort *) sort -> (sort array) = "caml_cvc5__Sort__getInstantiatedParameters"
external sort__substitute__ss : (* class Sort *) sort -> sort -> sort -> sort = "caml_cvc5__Sort__substitute__overload__ss"
external sort__substitute__svsv : (* class Sort *) sort -> (sort array) -> (sort array) -> sort = "caml_cvc5__Sort__substitute__overload__svsv"
external sort__toString : (* class Sort *) sort -> string = "caml_cvc5__Sort__toString"
external sort__getDatatypeConstructorArity : (* class Sort *) sort -> int64 = "caml_cvc5__Sort__getDatatypeConstructorArity"
external sort__getDatatypeConstructorDomainSorts : (* class Sort *) sort -> (sort array) = "caml_cvc5__Sort__getDatatypeConstructorDomainSorts"
external sort__getDatatypeConstructorCodomainSort : (* class Sort *) sort -> sort = "caml_cvc5__Sort__getDatatypeConstructorCodomainSort"
external sort__getDatatypeSelectorDomainSort : (* class Sort *) sort -> sort = "caml_cvc5__Sort__getDatatypeSelectorDomainSort"
external sort__getDatatypeSelectorCodomainSort : (* class Sort *) sort -> sort = "caml_cvc5__Sort__getDatatypeSelectorCodomainSort"
external sort__getDatatypeTesterDomainSort : (* class Sort *) sort -> sort = "caml_cvc5__Sort__getDatatypeTesterDomainSort"
external sort__getDatatypeTesterCodomainSort : (* class Sort *) sort -> sort = "caml_cvc5__Sort__getDatatypeTesterCodomainSort"
external sort__getFunctionArity : (* class Sort *) sort -> int64 = "caml_cvc5__Sort__getFunctionArity"
external sort__getFunctionDomainSorts : (* class Sort *) sort -> (sort array) = "caml_cvc5__Sort__getFunctionDomainSorts"
external sort__getFunctionCodomainSort : (* class Sort *) sort -> sort = "caml_cvc5__Sort__getFunctionCodomainSort"
external sort__getArrayIndexSort : (* class Sort *) sort -> sort = "caml_cvc5__Sort__getArrayIndexSort"
external sort__getArrayElementSort : (* class Sort *) sort -> sort = "caml_cvc5__Sort__getArrayElementSort"
external sort__getSetElementSort : (* class Sort *) sort -> sort = "caml_cvc5__Sort__getSetElementSort"
external sort__getBagElementSort : (* class Sort *) sort -> sort = "caml_cvc5__Sort__getBagElementSort"
external sort__getSequenceElementSort : (* class Sort *) sort -> sort = "caml_cvc5__Sort__getSequenceElementSort"
external sort__getAbstractedKind : (* class Sort *) sort -> sortKind = "caml_cvc5__Sort__getAbstractedKind"
external sort__getUninterpretedSortConstructorArity : (* class Sort *) sort -> int64 = "caml_cvc5__Sort__getUninterpretedSortConstructorArity"
external sort__getBitVectorSize : (* class Sort *) sort -> uint32_t = "caml_cvc5__Sort__getBitVectorSize"
external sort__getFiniteFieldSize : (* class Sort *) sort -> string = "caml_cvc5__Sort__getFiniteFieldSize"
external sort__getFloatingPointExponentSize : (* class Sort *) sort -> uint32_t = "caml_cvc5__Sort__getFloatingPointExponentSize"
external sort__getFloatingPointSignificandSize : (* class Sort *) sort -> uint32_t = "caml_cvc5__Sort__getFloatingPointSignificandSize"
external sort__getDatatypeArity : (* class Sort *) sort -> int64 = "caml_cvc5__Sort__getDatatypeArity"
external sort__getTupleLength : (* class Sort *) sort -> int64 = "caml_cvc5__Sort__getTupleLength"
external sort__getTupleSorts : (* class Sort *) sort -> (sort array) = "caml_cvc5__Sort__getTupleSorts"
external result__isNull : (* class Result *) sat_result -> bool = "caml_cvc5__Result__isNull"
external result__isSat : (* class Result *) sat_result -> bool = "caml_cvc5__Result__isSat"
external result__isUnsat : (* class Result *) sat_result -> bool = "caml_cvc5__Result__isUnsat"
external result__isUnknown : (* class Result *) sat_result -> bool = "caml_cvc5__Result__isUnknown"
external result__toString : (* class Result *) sat_result -> string = "caml_cvc5__Result__toString"
external result__getUnknownExplanation : (* class Result *) sat_result -> unknownExplanation = "caml_cvc5__Result__getUnknownExplanation"
external term__getNumChildren : (* class Term *) term -> int64 = "caml_cvc5__Term__getNumChildren"
external term__getId : (* class Term *) term -> int64 = "caml_cvc5__Term__getId"
external term__getKind : (* class Term *) term -> kind = "caml_cvc5__Term__getKind"
external term__getSort : (* class Term *) term -> sort = "caml_cvc5__Term__getSort"
external term__toString : (* class Term *) term -> string = "caml_cvc5__Term__toString"
external term__substitute__tt : (* class Term *) term -> term -> term -> term = "caml_cvc5__Term__substitute__overload__tt"
external term__substitute__tvtv : (* class Term *) term -> (term array) -> (term array) -> term = "caml_cvc5__Term__substitute__overload__tvtv"
external term__hasOp : (* class Term *) term -> bool = "caml_cvc5__Term__hasOp"
external term__getOp : (* class Term *) term -> op = "caml_cvc5__Term__getOp"
external term__hasSymbol : (* class Term *) term -> bool = "caml_cvc5__Term__hasSymbol"
external term__getSymbol : (* class Term *) term -> string = "caml_cvc5__Term__getSymbol"
external term__isNull : (* class Term *) term -> bool = "caml_cvc5__Term__isNull"
external term__notTerm : (* class Term *) term -> term = "caml_cvc5__Term__notTerm"
external term__andTerm : (* class Term *) term -> term -> term = "caml_cvc5__Term__andTerm"
external term__orTerm : (* class Term *) term -> term -> term = "caml_cvc5__Term__orTerm"
external term__xorTerm : (* class Term *) term -> term -> term = "caml_cvc5__Term__xorTerm"
external term__eqTerm : (* class Term *) term -> term -> term = "caml_cvc5__Term__eqTerm"
external term__impTerm : (* class Term *) term -> term -> term = "caml_cvc5__Term__impTerm"
external term__iteTerm : (* class Term *) term -> term -> term -> term = "caml_cvc5__Term__iteTerm"
external term__getRealOrIntegerValueSign : (* class Term *) term -> int32_t = "caml_cvc5__Term__getRealOrIntegerValueSign"
external term__isInt32Value : (* class Term *) term -> bool = "caml_cvc5__Term__isInt32Value"
external term__getInt32Value : (* class Term *) term -> int32_t = "caml_cvc5__Term__getInt32Value"
external term__isUInt32Value : (* class Term *) term -> bool = "caml_cvc5__Term__isUInt32Value"
external term__getUInt32Value : (* class Term *) term -> uint32_t = "caml_cvc5__Term__getUInt32Value"
external term__isInt64Value : (* class Term *) term -> bool = "caml_cvc5__Term__isInt64Value"
external term__getInt64Value : (* class Term *) term -> int64 = "caml_cvc5__Term__getInt64Value"
external term__isUInt64Value : (* class Term *) term -> bool = "caml_cvc5__Term__isUInt64Value"
external term__getUInt64Value : (* class Term *) term -> int64 = "caml_cvc5__Term__getUInt64Value"
external term__isIntegerValue : (* class Term *) term -> bool = "caml_cvc5__Term__isIntegerValue"
external term__getIntegerValue : (* class Term *) term -> string = "caml_cvc5__Term__getIntegerValue"
external term__isStringValue : (* class Term *) term -> bool = "caml_cvc5__Term__isStringValue"
external term__isBooleanValue : (* class Term *) term -> bool = "caml_cvc5__Term__isBooleanValue"
external term__getBooleanValue : (* class Term *) term -> bool = "caml_cvc5__Term__getBooleanValue"
external term__isBitVectorValue : (* class Term *) term -> bool = "caml_cvc5__Term__isBitVectorValue"
external term__getBitVectorValue : (* class Term *) term -> uint32_t -> string = "caml_cvc5__Term__getBitVectorValue"
(*$*)


