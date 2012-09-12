cl /Gd extension-functions.c /D WIN32 /DDLL /LD /link /export:sqlite3_extension_init /out:extensions.sqlext
