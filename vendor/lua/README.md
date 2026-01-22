# Lua 5.4 Setup

Download Lua 5.4 source from: https://www.lua.org/ftp/lua-5.4.6.tar.gz

Extract and copy:
- All `.h` files from `src/` to `include/`
- All `.c` files from `src/` to `src/` (except lua.c and luac.c)

Or use vcpkg:
```
vcpkg install lua:x64-windows
```

The engine expects:
- Headers in: `vendor/lua/include/`
- Source in: `vendor/lua/src/` (for direct compilation)
