
@REM remove build, dist, touchpy.egg-info
if exist build rmdir /s /q build
if exist dist rmdir /s /q dist
if exist touchpy.egg-info rmdir /s /q touchpy.egg-info

py build_wheels.py

@REM if arg is uploadtest, upload to testpypi
@REM if arg is upload, upload to pypi

if "%1" == "test_upload" (
	py -m twine upload --repository testpypi dist/*
) else if "%1" == "upload" (
	py -m twine upload dist/*
)

@REM py -m twine upload --repository testpypi dist/*
@REM py -m twine upload dist/*