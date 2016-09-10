# Root Makefile
export TOP := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
include build/main.mk