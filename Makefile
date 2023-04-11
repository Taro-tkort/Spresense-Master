############################################################################
# apps/examples/helloxx/Makefile
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.  The
# ASF licenses this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
############################################################################

include $(APPDIR)/Make.defs
include $(SDKDIR)/Make.defs

# helloxx built-in application info

PROGNAME = $(CONFIG_EXAMPLES_IMPULSEXX_PROGNAME)
PRIORITY = $(CONFIG_EXAMPLES_IMPULSEXX_PRIORITY)
STACKSIZE = $(CONFIG_EXAMPLES_IMPULSEXX_STACKSIZE)
MODULE = $(CONFIG_EXAMPLES_IMPULSEXX)

# Hello, World! C++ Example

MAINSRC = impulsexx_main.cxx


########audio example additions########
# Audio Example paths

AUDIODIR = $(SDKDIR)$(DELIM)modules$(DELIM)audio

# Audio Example flags

CXXFLAGS += ${shell $(INCDIR) $(INCDIROPT) "$(CC)" "$(AUDIODIR)"}
ifeq ($(CONFIG_EXAMPLES_IMPULSEXX_USEPOSTPROC),y)
CXXFLAGS += ${shell $(INCDIR) $(INCDIROPT) "$(CC)" "worker$(DELIM)userproc$(DELIM)include"}
endif

CXXFLAGS += -D_POSIX
CXXFLAGS += -DUSE_MEMMGR_FENCE
CXXFLAGS += -DATTENTION_USE_FILENAME_LINE

include $(APPDIR)/Application.mk

ifeq ($(CONFIG_EXAMPLES_IMPULSEXX_USEPOSTPROC),y)

build_worker:
	$(Q) $(MAKE) -C worker TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV)

$(OBJS): build_worker

clean_worker:
	@$(MAKE) -C worker TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV) clean

clean:: clean_worker

endif
