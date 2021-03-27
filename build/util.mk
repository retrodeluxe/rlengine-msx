# Utility definitions
#
bold := $(shell tput bold)
sgr0 := $(shell tput sgr0)
red := $(shell tput setaf 1)
green := $(shell tput setaf 2)
yellow := $(shell tput setaf 3)
blue := $(shell tput setaf 4)
magenta := $(shell tput setaf 5)
cyan := $(shell tput setaf 6)
white := $(shell tput setaf 7)

define print_res
		@printf "$(bold)$(green)[res ] $(1) $(sgr0): $(2)\n"
endef

define print_cc
		@printf "$(bold)$(blue)[cc  ] $(1) $(sgr0): $(2)\n"
endef

define print_ld
		@printf "$(bold)$(magenta)[ld  ] $(1) $(sgr0): $(2)\n"
endef

define print_ar
		@printf "$(bold)$(blue)[ar  ] $(1) $(sgr0): $(2)\n"
endef

define print_pack
		@printf "$(bold)$(yellow)[pack] $(1) $(sgr0): $(2)\n"
endef

define print_host_cc
		@printf "$(bold)$(cyan)[gcc ] $(1) $(sgr0): $(2)\n"
endef

# recursive wildcard
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
