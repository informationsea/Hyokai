candle HyokaiInstaller.wxs
@if not "%ERRORLEVEL%"  == "0" GOTO ERR

light -ext WixUIExtension -ext WixUtilExtension HyokaiInstaller.wixobj
@if not "%ERRORLEVEL%"  == "0" GOTO ERR

@GOTO FIN

:ERR
@echo Failed to build
:FIN
