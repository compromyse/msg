LUA_VERSION := 5.4.7
LUA_BIN ?= $(CURDIR)/bin

lua: bin

.ONESHELL:
bin:
	curl -L -R -O https://www.lua.org/ftp/lua-$(LUA_VERSION).tar.gz
	tar zxf lua-$(LUA_VERSION).tar.gz
	###
	cd lua-$(LUA_VERSION)
	make all test
	###
	mkdir -p $(LUA_BIN)
	mv src/lua src/luac $(LUA_BIN)
	###
	rm -rf $(CURDIR)/lua-$(LUA_VERSION) $(CURDIR)/lua-$(LUA_VERSION).tar.gz
