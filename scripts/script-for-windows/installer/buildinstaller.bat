candle HyokaiInstaller.wxs
if not "%ERRORLEVEL%"  == "0" GOTO ERR

light -ext WixUIExtension HyokaiInstaller.wixobj
if not "%ERRORLEVEL%"  == "0" GOTO ERR

EXIT 0

:ERR
@echo "Failed to build"
EXIT 1
