@echo off

for /f "tokens=*" %%i in ('where git') do set git=%%i
set path=%git%\..\..\mingw64\bin\;%git%\..\..\usr\bin\;%VS140COMNTOOLS%..\..\vc\;%path%
set git_curl="%git%\..\..\usr\mingw64\curl.exe"
set git_unzip="%git%\..\..\usr\bin\unzip.exe"
set git_date="%git%\..\..\usr\bin\date.exe"

set root=%cd%
set php_src=%root%\php-sdk-5.6\vc11\php-5.6.33-src
set openrasp_src=%php_src%\ext\openrasp
for /f "tokens=*" %%i in ('%git_date% +%%Y-%%m-%%d') do set output_base=%root%\rasp-php-%%i

for %%i in (conf,assets,logs,locale,plugins) do if not exist %output_base%\%%i mkdir %output_base%\%%i

echo Download PHP SDK
curl -L -O https://packages.baidu.com/app/openrasp/php-sdk-5.6.zip
echo Extract PHP SDK
unzip -q -o -d php-sdk-5.6 php-sdk-5.6.zip
mklink /J %openrasp_src% agent\php
call %root%\php-sdk-5.6\bin\phpsdk_setvars.bat

:build_dll

pushd %php_src%

call buildconf.bat

echo Build x86
call vcvarsall.bat x86
call configure.bat --disable-all --enable-json --enable-pdo --enable-cli --enable-openrasp
nmake php_openrasp.dll
set output_ext=%output_base%\php\win32-php5.6-x86
md %output_ext%
copy Release_TS\php_openrasp.dll %output_ext%\

if not "%PROCESSOR_ARCHITECTURE%" == "AMD64" goto build_mo
echo Build x64
call vcvarsall.bat x64
call configure.bat --disable-all --enable-json --enable-pdo --enable-cli --enable-openrasp
nmake php_openrasp.dll
set output_ext=%output_base%\php\win32-php5.6-x64
md %output_ext%
copy x64\Release_TS\php_openrasp.dll %output_ext%\

popd

:build_mo
echo Build MO
pushd %openrasp_src%\po
for /d %%i in ("*") do (
	if not exist %output_base%\locale\%%i\LC_MESSAGES mkdir %output_base%\locale\%%i\LC_MESSAGES 
	msgfmt --output-file=%output_base%\locale\%%i\LC_MESSAGES\openrasp.mo %%i\openrasp.po
)
popd

copy %root%\plugins\official\plugin.js %output_base%\plugins\official.js
copy %root%\rasp-install\php\*.php %output_base%\

echo Zip
del rasp-php.zip
pushd %output_base%
zip -r ..\rasp-php.zip .\*
popd

rmdir /s /q php-sdk-5.6
del php-sdk-5.6.zip

echo.
echo Build Success
echo.

pause