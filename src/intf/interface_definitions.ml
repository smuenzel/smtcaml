
module Smtcaml_intf = Smtcaml_intf_internal

module type Bitvector_basic = sig
  include Smtcaml_intf.Backend_base
  include Smtcaml_intf.Bitvector with module Types := Types
  include Smtcaml_intf.Boolean with module Types := Types
end
