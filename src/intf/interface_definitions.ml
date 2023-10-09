
module Smtcaml_intf = Smtcaml_intf_internal

module type Bitvector_basic = sig
  include Smtcaml_intf.Backend
  include Smtcaml_intf.Bitvector with module Types := Types
  include Smtcaml_intf.Boolean with module Types := Types
end

module type Bitvector_uf = sig
  include Smtcaml_intf.Backend
  include Smtcaml_intf.Bitvector with module Types := Types
  include Smtcaml_intf.Boolean with module Types := Types
  include Smtcaml_intf.Uninterpreted_function with module Types := Types
end
