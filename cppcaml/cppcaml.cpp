#include <cppcaml.h>
using namespace CppCaml;

value ApiFunctionDescription::to_value(){
  CAMLparam0();
  CAMLlocal5(v_ret, v_return_type, v_parameter_count, v_parameters, v_it);
  CAMLlocal1(v_next);
  v_return_type = caml_copy_string(this->return_type);
  v_parameter_count = Val_long(this->parameter_count);
  v_parameters = list_to_caml(caml_copy_string,this->parameters);
  v_ret = caml_alloc_small(3,0);
  Field(v_ret,0) = v_return_type;
  Field(v_ret,1) = v_parameter_count;
  Field(v_ret,2) = v_parameters;
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

value api_registry_entry_to_list(ApiRegistryEntry*start, ApiRegistryEntry*stop){
  uint64_t* idx = (uint64_t*)start;
  uint64_t* sstop = (uint64_t*)stop;
  while(idx < sstop && *idx != marker){
    idx++;
  };
  if(idx >= sstop) return Val_long(0);
  else{
    auto entry = (ApiRegistryEntry*)idx;
    CAMLparam0();
    CAMLlocal1(v_ret);
    v_ret = caml_alloc(2, 0);
    Store_field(v_ret,0,entry->to_value());
    Store_field(v_ret,1,api_registry_entry_to_list(entry+1, stop));
    CAMLreturn(v_ret);
  }
}

apireturn caml_get_api_registry(value){
  extern ApiRegistryEntry __start_caml_api_registry;
  extern ApiRegistryEntry __stop_caml_api_registry;
  ApiRegistryEntry * start = &__start_caml_api_registry;
  ApiRegistryEntry * stop = &__stop_caml_api_registry;
  return api_registry_entry_to_list(start,stop);
}

