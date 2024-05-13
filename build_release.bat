@echo off

setlocal

cmake -S . -B out/install_build/py39 -G "Ninja" --preset x64-release-py39
ninja -C out/install_build/py39

cmake -S . -B out/install_build/py310 -G "Ninja" --preset x64-release-py310
ninja -C out/install_build/py310

cmake -S . -B out/install_build/py311 -G "Ninja" --preset x64-release-py311
ninja -C out/install_build/py311

cmake -S . -B out/install_build/py312 -G "Ninja" --preset x64-release-py312
ninja -C out/install_build/py312


pushd %~dp0

REM Command file for Sphinx documentation

if "%SPHINXBUILD%" == "" (
	set SPHINXBUILD=sphinx-build
)
set SOURCEDIR=docs/source
set BUILDDIR=install/docs

%SPHINXBUILD% >NUL 2>NUL
if errorlevel 9009 (
	echo.
	echo.The 'sphinx-build' command was not found. Make sure you have Sphinx
	echo.installed, then set the SPHINXBUILD environment variable to point
	echo.to the full path of the 'sphinx-build' executable. Alternatively you
	echo.may add the Sphinx directory to PATH.
	echo.
	echo.If you don't have Sphinx installed, grab it from
	echo.https://www.sphinx-doc.org/
	exit /b 1
)

%SPHINXBUILD% -M html %SOURCEDIR% %BUILDDIR% %SPHINXOPTS% %O%

:end
popd

@REM if first argument exists use it as a path to copy modules, docs, and examples to the specified directory

if "%~1" == "" (
    echo Usage: %0 destination_directory
    exit /b 1
)

set "DEST_DIR=%~1"

@REM Remove trailing backslashes
if "%DEST_DIR:~-1%" == "\" (
	set "DEST_DIR=%DEST_DIR:~0,-1%"
)

@REM Replace forward slashes with backslashes in the destination path
set "DEST_DIR=%DEST_DIR:/=\%"

@REM Ensure destination directory exists or attempt to create it
if not exist "%DEST_DIR%\" (
    mkdir "%DEST_DIR%"
    if errorlevel 1 (
        echo Failed to create destination directory.
        exit /b 1
    )
)

@REM copy all files other than .pyd files
echo Copying install\modules to: %DEST_DIR%\touchpy\
xcopy /E /Y /Q install\modules\ %DEST_DIR%\touchpy\ /exclude:install\modules\*.pyd

@REM copy all .pyd files
echo Copying install\modules\*.pyd to: %DEST_DIR%\pyd_files\
xcopy /Y /Q install\modules\*.pyd %DEST_DIR%\pyd_files\

echo Copying install\docs to: %DEST_DIR%\docs\
xcopy /E /Y /Q install\docs\ %DEST_DIR%\docs\

echo Copying install\examples to: %DEST_DIR%\examples\
xcopy /E /Y /Q install\examples\ %DEST_DIR%\examples\



endlocal