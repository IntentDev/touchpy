@echo off

@REM build_release.bat package upload 		@REM will build the package and upload it to PyPi
@REM build_release.bat package test_upload 	@REM will build the package and upload it to TestPyPi
@REM build_release.bat 						@REM will compile the files and copy them only to install/modules
@REM build_release.bat path\to\destination 	@REM will compile the files and copy them to the specified directory


setlocal

cmake -S . -B out/install_build/py39 -G "Ninja" --preset x64-release-py39
ninja -C out/install_build/py39

cmake -S . -B out/install_build/py310 -G "Ninja" --preset x64-release-py310
ninja -C out/install_build/py310

cmake -S . -B out/install_build/py311 -G "Ninja" --preset x64-release-py311
ninja -C out/install_build/py311

cmake -S . -B out/install_build/py312 -G "Ninja" --preset x64-release-py312
ninja -C out/install_build/py312


@REM pushd %~dp0

@REM REM Command file for Sphinx documentation

@REM if "%SPHINXBUILD%" == "" (
@REM 	set SPHINXBUILD=sphinx-build
@REM )
@REM set SOURCEDIR=docs/source
@REM set BUILDDIR=install/docs

@REM %SPHINXBUILD% >NUL 2>NUL
@REM if errorlevel 9009 (
@REM 	echo.
@REM 	echo.The 'sphinx-build' command was not found. Make sure you have Sphinx
@REM 	echo.installed, then set the SPHINXBUILD environment variable to point
@REM 	echo.to the full path of the 'sphinx-build' executable. Alternatively you
@REM 	echo.may add the Sphinx directory to PATH.
@REM 	echo.
@REM 	echo.If you don't have Sphinx installed, grab it from
@REM 	echo.https://www.sphinx-doc.org/
@REM 	exit /b 1
@REM )

@REM %SPHINXBUILD% -M html %SOURCEDIR% %BUILDDIR% %SPHINXOPTS% %O%

@REM :end
@REM popd

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

@REM if the destination directory is the subdirectory package run the build_package.bat script
if "%DEST_DIR%" == "package" (

	@REM set the current directory to the package directory
	pushd package

	@REM if there is a second arg pass it to the build_package.bat script
	if not "%~2" == "" (
		call build_package.bat %~2
	) else (
		call build_package.bat
	)

	@REM return to the original directory
	popd
)


endlocal