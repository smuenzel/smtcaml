#include <boolector/boolector.h>
#include <cppcaml.h>

using CppCaml::Custom_value;
using CppCaml::ContainerOps;

DECL_API_TYPE(uint32_t,uint32_t);
DECL_API_TYPE(int32_t,int32_t);
DECL_API_TYPE(bool,bool);
DECL_API_TYPE(BoolectorNode*,node);
DECL_API_TYPE(BoolectorNode**,node array);
DECL_API_TYPE(BoolectorSort,sort);
DECL_API_TYPE(Btor*,btor);
DECL_API_TYPE(const char*,string);
DECL_API_TYPE(BtorOption,btor_option);
DECL_API_TYPE(void,unit);

typedef std::remove_pointer<BoolectorSort>::type BoolectorSortRaw;

////////////////////////////////////////////////////////////////////////////////////////

template<> struct CppCaml::CamlConversionProperties<BoolectorNode*>{
  static constexpr auto representation_kind = CppCaml::CamlRepresentationKind::ContainerWithContext;
  static constexpr bool allow_const = true;
  typedef Btor Context;
  static void delete_T(Context*context, BoolectorNode*t){
    boolector_release(context,t);
  }
};

template<> struct CppCaml::CamlConversionProperties<BoolectorSortRaw*>{
  static constexpr auto representation_kind = CppCaml::CamlRepresentationKind::ContainerWithContext;
  typedef Btor Context;
  static void delete_T(Context*context, BoolectorSortRaw*t){
    boolector_release_sort(context,t);
  }
};

static_assert(CppCaml::CamlBidirectional<BoolectorNode*>);

template<> struct CppCaml::CamlConversionProperties<Btor*>{
  static constexpr auto representation_kind = CppCaml::CamlRepresentationKind::ContainerSharedPointer;
  static void delete_T(Btor*b){
    boolector_delete(b);
  }
};

template<> struct CppCaml::CamlConversion<BtorOption> {
  typedef BtorOption RepresentationType;
  static const auto allocates = CppCaml::CamlAllocates::No_allocation;

  static inline value to_value(BtorOption x) { return Val_long(x); }
  static inline RepresentationType of_value(value v) { return (BtorOption)Long_val(v); }
  static inline BtorOption get_underlying(RepresentationType r) { return r; }
};
static_assert(CppCaml::CamlBidirectional<BtorOption>);

template<> struct CppCaml::CamlConversion<BoolectorNode**> {
  typedef std::vector<CppCaml::CamlConversion<BoolectorNode*>::RepresentationType> RepresentationType;
  typedef Btor Context;
  static const auto allocates = CppCaml::CamlAllocates::Allocation;

  static inline value to_value(std::shared_ptr<Btor>&context, BoolectorNode** n) {
    CAMLparam0();
    CAMLlocal1(ar);
    size_t length = 0;
    auto it = n;
    while(*it) { it ++; length ++; };
    ar = caml_alloc(length, 0);
    for(size_t i = 0; i < length; i ++){
      Field(ar,i) = CppCaml::CamlConversion<BoolectorNode*>::to_value(context, n[i]);
    };
    return ar;
  }
};

////////////////////////////////////////////////////////////////////////////////////////


#define API0(APIF) API0__(_,boolector_##APIF)
#define API1(APIF) API1__(_,boolector_##APIF)
#define API1_BLOCKING(APIF) API1_BLOCKING__(_,boolector_##APIF)
#define API2(APIF) API2__(_,boolector_##APIF)
#define API3(APIF) API3__(_,boolector_##APIF)
#define API3_BLOCKING(APIF) API3_BLOCKING__(_,boolector_##APIF)
#define API4(APIF) API3__(_,boolector_##APIF)
#define API1I(APIF) API1_IMPLIED__(_,boolector_##APIF);
#define API2I(APIF) API2_IMPLIED__(_,boolector_##APIF);
#define API3I(APIF) API3_IMPLIED__(_,boolector_##APIF);
#define API4I(APIF) API3_IMPLIED__(_,boolector_##APIF);

////////////////////////////////////////////////////////////////////////////////////////

enum class BtorResult {
  Unknown = BTOR_RESULT_UNKNOWN
, Sat = BTOR_RESULT_SAT
, Unsat = BTOR_RESULT_UNSAT
};

#define ENUM_BtorResult(F) \
  F(BtorResult, Unknown) \
  F(BtorResult, Sat) \
  F(BtorResult, Unsat)

MAKE_ENUM_IMMEDIATE_PROPERTIES(BtorResult,ENUM_BtorResult,Unknown)

////////////////////////////////////////////////////////////////////////////////////////

//API(new)
//API(clone)
//API(delete)
//API(set_term)
//API(terminate)
//API(set_abort)
//API(set_msg_prefix)
//API(get_refs)
API1(reset_time)
API1(reset_stats)
API1(print_stats)
//API(set_trapi)
//API(get_trapi)
API2(push)
API2(pop)
API1I(assert)
API1I(assume)
API1I(failed)
API1(get_failed_assumptions)
API1(fixate_assumptions)
API1(reset_assumptions)
//API(sat)
//API(limited_sat)
API1(simplify)
API2(set_sat_solver)
API3(set_opt)
API2(get_opt)
API2(get_opt_min)
API2(get_opt_max)
API2(get_opt_dflt)
API2(get_opt_lng)
API2(get_opt_shrt)
API2(get_opt_desc)
API2(has_opt)
API1(first_opt)
API2(next_opt)
//API(copy)
//API(release)
//API(release_all)
API1(true)
API1(false)
API2I(implies)
API2I(iff)
API2I(eq)
API2I(ne)
API1I(is_bv_const_zero)
API1I(is_bv_const_one)
API1I(is_bv_const_ones)
API1I(is_bv_const_max_signed)
API1I(is_bv_const_min_signed)
API2(const)
API2I(constd)
API2I(consth)
API1I(zero)
API1I(ones)
API1I(one)
API1I(min_signed)
API1I(max_signed)
API3(unsigned_int)
API3(int)
API2I(var)
API2I(array)
API2I(const_array)
API2I(uf)
API1I(not)
API1I(neg)
API1I(redor)
API1I(redxor)
API1I(redand)
API3I(slice)
API2I(uext)
API2I(sext)
API2I(xor)
API2I(xnor)
API2I(and)
API2I(nand)
API2I(or)
API2I(nor)
API2I(add)
API2I(uaddo)
API2I(saddo)
API2I(mul)
API2I(umulo)
API2I(smulo)
API2I(ult)
API2I(slt)
API2I(ulte)
API2I(slte)
API2I(ugt)
API2I(sgt)
API2I(ugte)
API2I(sgte)
API2I(sll)
API2I(srl)
API2I(sra)
API2I(rol)
API2I(ror)
API2I(roli)
API2I(rori)
API2I(sub)
API2I(usubo)
API2I(ssubo)
API2I(udiv)
API2I(sdiv)
API2I(sdivo)
API2I(urem)
API2I(srem)
API2I(smod)
API2I(concat)
API2I(repeat)
API2I(read)
API3I(write)
API3I(cond)
API2I(param)
//API4(fun)
//API3(apply)
API1I(inc)
API1I(dec)
//API(forall)
//API(exists)
API1(get_btor)
API1I(get_node_id)
API1I(get_sort)
API1I(fun_get_domain_sort)
API1I(fun_get_codomain_sort)
API2(match_node_by_id)
API2(match_node_by_symbol)
API1I(match_node)
API1I(get_symbol)
API2I(set_symbol)
API1I(get_width)
API1I(get_index_width)
//API1I(get_bits)
//API1(free_bits)
API1I(get_fun_arity)
API1I(is_const)
API1I(is_var)
API1I(is_array)
API1I(is_array_var)
API1I(is_param)
API1I(is_bound_param)
API1I(is_uf)
API1I(is_fun)
//API1(fun_sort_check)
API1I(get_value)
//API1I(bv_assignment)
//API(free_bv_assignment)
//API(array_assignment)
//API(free_array_assignment)
//API(uf_assignment)
//API(free_uf_assignment)
//API(print_model)
API1(bool_sort)
API2(bitvec_sort)
//API3I(fun_sort)
//API(array_sort)
//API(copy_sort)
//API(release_sort)
API2I(is_equal_sort)
API1I(is_array_sort)
API1I(is_bitvec_sort)
API1I(is_fun_sort)
API1I(bitvec_sort_get_width)
//API(parse)
//API(parse_btor)
//API(dump_btor_node)
//API(dump_btor)
//API(dump_aiger_ascii)
//API(dump_aiger_binary)
API1(copyright)
API1(version)
API1(git_id)

////////////////////////////////////////////////////////////////////////////////////////

static void abort_callback(const char* msg){
  if(Caml_state == NULL) caml_acquire_runtime_system();
  caml_failwith(msg);
}

auto boolector_new_(){
  boolector_set_abort(&abort_callback);
  return boolector_new();
}
API0(new_)

template<typename... Ps, size_t... Is>
static inline int32_t call_sat_generic(int32_t (*inner_sat)(Btor*,Ps...), Btor*btor, std::tuple<Ps...> args, std::index_sequence<Is...> is){
  return inner_sat(btor,std::get<Is>(args)...);
}

template<typename... Ps>
static value sat_generic(int32_t (*inner_sat)(Btor*,Ps...), value v_btor, typename CppCaml::first_type<value,Ps>::type... v_ps){
  /* acquire ownership before blocking section */
  auto s_btor = Custom_value<CppCaml::SharedPointerContainer<Btor>>(v_btor).t;
  auto index_sequence = std::index_sequence_for<Ps...>();
  std::tuple p_ar{ CppCaml::CamlConversion<Ps>::get_underlying(CppCaml::CamlConversion<Ps>::of_value(v_ps))... };
  caml_enter_blocking_section();
  auto sat = call_sat_generic(inner_sat, s_btor.get(), p_ar, index_sequence);
  caml_leave_blocking_section();
  switch(sat){
    case BTOR_RESULT_SAT: return Val_int(1);
    case BTOR_RESULT_UNSAT: return Val_int(2);
    default: return Val_int(0);
  }
}

BtorResult boolector_satR (Btor *btor){
  return (BtorResult)boolector_sat(btor);
}
API1_BLOCKING(satR)

BtorResult boolector_limited_satR (Btor *btor, int32_t lod_limit, int32_t sat_limit){
  return (BtorResult)boolector_limited_sat(btor, lod_limit, sat_limit);
}
API3_BLOCKING(limited_satR)

void boolector_free_bv_assignment_(Btor*&b,const char*&s){
  boolector_free_bv_assignment(b,s);
}

apireturn caml_boolector_bv_assignment(value v_p0){
  return CppCaml::call_api_implied_first(boolector_free_bv_assignment_,boolector_bv_assignment,v_p0);
}
REGISTER_API_CUSTOM(boolector_bv_assignment,caml_boolector_bv_assignment,const char *, BoolectorNode*);

inline BoolectorNode* boolector_fun_vector(Btor*btor, std::vector<BoolectorNode*> params, BoolectorNode*body){
  return boolector_fun(btor,params.data(), params.size(), body);
}
API3(fun_vector)

//CR smuenzel: we should do something to prevent copying of vectors
// probably make it a reference and change 'invoke_result' to not remove the reference
inline BoolectorSort boolector_fun_sort_vector(Btor*btor, std::vector<BoolectorSort> domain, BoolectorSort codomain){
  return boolector_fun_sort(btor,domain.data(), domain.size(), codomain);
}
API3(fun_sort_vector)

inline BoolectorNode* boolector_apply_vector(Btor*btor, std::vector<BoolectorNode*> args, BoolectorNode* n_fun){
  return boolector_apply(btor, args.data(), args.size(), n_fun);
}
API4(apply_vector)

inline BoolectorNode* boolector_uf_opt(Btor*btor, BoolectorSort sort, std::optional<const char *> symbol){
  if(symbol)
    return boolector_uf(btor,sort,symbol.value());
  else
    return boolector_uf(btor,sort,0);
}

API2I(uf_opt)

apireturn caml_boolector_unit(value){
  return Val_unit;
}
