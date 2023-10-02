#include <boolector/boolector.h>
#include <cppcaml.h>

using CppCaml::Custom_value;
using CppCaml::ContainerOps;

struct solver_result;

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
DECL_API_TYPE(solver_result,solver_result);

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
#define API2(APIF) API2__(_,boolector_##APIF)
#define API3(APIF) API3__(_,boolector_##APIF)
#define API1I(APIF) API1_IMPLIED__(_,boolector_##APIF);
#define API2I(APIF) API2_IMPLIED__(_,boolector_##APIF);
#define API3I(APIF) API3_IMPLIED__(_,boolector_##APIF);

API1I(get_sort)
API1(false)
API1(true)
API2I(implies)
API2I(iff)
API2I(eq)
API2I(ne)
API1I(not)
API1I(neg)
API1I(redor)
API1I(redxor)
API1I(redand)
API3I(slice)
API2I(uext)
API2I(sext)
API2I(xor)
API2I(and)
API2I(or)
API2I(nand)
API2I(nor)
API2I(add)
API2I(uaddo)
API2I(saddo)
API2I(mul)
API2I(umulo)
API2I(smulo)
API2I(ult)
API2I(ulte)
API2I(slte)
API2I(slt)
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
API2I(read)
API3I(write)
API3I(cond)
API1I(inc)
API1I(dec)

API3(int);

API1(bool_sort)
API1I(bitvec_sort_get_width)
API2(const)

API1I(zero)

API2I(var)

API2(bitvec_sort);

API2I(array_sort);

API1(print_stats);

API2(push);
API2(pop);
API1I(assert);
API1I(assume);
API1I(failed);

API1(get_failed_assumptions);
API1(fixate_assumptions);
API1(reset_assumptions);

API2(set_sat_solver);

API1(first_opt);

API2(has_opt);
API2(next_opt);
API2(get_opt_lng);
API2(get_opt_shrt);
API2(get_opt_desc);
API2(get_opt_min);
API2(get_opt_max);
API2(get_opt_dflt);
API2(get_opt);
API3(set_opt);

API3(unsigned_int)

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

apireturn caml_boolector_sat(value v_btor){
  return sat_generic(boolector_sat, v_btor);
}
REGISTER_API_CUSTOM(boolector_sat,caml_boolector_sat,solver_result,Btor*);

apireturn caml_boolector_limited_sat(value v_btor, value lod_limit, value sat_limit){
  return sat_generic(boolector_limited_sat,v_btor,lod_limit,sat_limit);
}
REGISTER_API_CUSTOM(boolector_limited_sat,caml_boolector_limited_sat,solver_result,Btor*,int32_t,int32_t);

void boolector_free_bv_assignment_(Btor*&b,const char*&s){
  boolector_free_bv_assignment(b,s);
}

apireturn caml_boolector_bv_assignment(value v_p0){
  return CppCaml::call_api_implied_first(boolector_free_bv_assignment_,boolector_bv_assignment,v_p0);
}
REGISTER_API_CUSTOM(boolector_bv_assignment,caml_boolector_bv_assignment,const char *, BoolectorNode*);


apireturn caml_boolector_unit(value){
  return Val_unit;
}
