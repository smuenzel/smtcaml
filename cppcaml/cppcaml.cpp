#include <cppcaml.h>
using namespace CppCaml;

value ApiFunctionDescription::to_value(){
  CAMLparam0();
  CAMLlocal5(v_ret, v_return_type, v_parameter_count, v_parameters, v_it);
  CAMLlocal3(v_class_name, v_next, v_allocates);
  v_return_type = caml_copy_string(this->return_type);
  v_parameter_count = Val_long(this->parameter_count);
  v_allocates = Val_bool(this->may_allocate);
  v_parameters = list_to_caml(caml_copy_string,this->parameters);
  if(this->class_name == 0){
    v_class_name = Val_none;
  }else{
    v_class_name = caml_alloc_small(1,0);
    Field(v_class_name,0) = caml_copy_string(this->class_name);
  };
  v_ret = caml_alloc_small(5,0);
  Field(v_ret,0) = v_return_type;
  Field(v_ret,1) = v_parameter_count;
  Field(v_ret,2) = v_parameters;
  Field(v_ret,3) = v_class_name;
  Field(v_ret,4) = v_allocates;
  CAMLreturn(v_ret);
}

value ApiRegistryEntry::to_value(){
  CAMLparam0();
  CAMLlocal4(v_ret,v_wrapper_name, v_name, v_description);
  v_wrapper_name = caml_copy_string(this->wrapper_name);
  v_name = caml_copy_string(this->name);
  v_description = this->description.to_value();
  v_ret = caml_alloc_small(3, 0);
  Field(v_ret,0) = v_wrapper_name;
  Field(v_ret,1) = v_name;
  Field(v_ret,2) = v_description;
  CAMLreturn(v_ret);
}

value ApiEnumEntry::to_value(){
  CAMLparam0();
  CAMLlocal4(v_ret,v_enumName, v_memberName, v_memberValue);
  v_enumName = caml_copy_string(this->enumName);
  v_memberName = caml_copy_string(this->memberName);
  v_memberValue = this->memberValue;
  v_ret = caml_alloc_small(3, 0);
  Field(v_ret,0) = v_enumName;
  Field(v_ret,1) = v_memberName;
  Field(v_ret,2) = v_memberValue;
  CAMLreturn(v_ret);
}

template<typename T>
value section_to_list(T*start,T*stop){
  CAMLparam0();
  CAMLlocal3(v_ret, v_cur, v_prev);
  v_ret = Val_none;
  v_cur = Val_none;
  v_prev = Val_none;
  while(1){
    uint64_t* idx = (uint64_t*)start;
    uint64_t* sstop = (uint64_t*)stop;
    while(idx < sstop && *idx != marker){
      idx++;
    };
    start=(T*) idx;
    if(start >= stop) {
      if(v_prev == Val_none)
        CAMLreturn(Val_long(0));
      else{
        Store_field(v_prev,1,Val_none);
        CAMLreturn(v_ret);
      }
    } else {
      auto entry = (T*)start;
      v_cur = caml_alloc(2, 0);
      if(v_prev == Val_none){
        v_ret = v_cur;
      } else {
        Store_field(v_prev,1,v_cur);
      };
      Store_field(v_cur,0,entry->to_value());
      v_prev = v_cur;
      start = entry+1;
    }
  }
}

apireturn caml_get_api_registry(value){
  extern ApiRegistryEntry __start_caml_api_registry;
  extern ApiRegistryEntry __stop_caml_api_registry;
  return section_to_list(&__start_caml_api_registry, &__stop_caml_api_registry);
}

static inline constexpr auto __caml_api_enum_var__Dummy
__attribute((used, section("caml_api_enum"))) = CppCaml::ApiEnumEntry();

apireturn caml_get_api_enums(value){
  extern ApiEnumEntry __start_caml_api_enum;
  extern ApiEnumEntry __stop_caml_api_enum;
  return section_to_list(&__start_caml_api_enum,&__stop_caml_api_enum);
}


