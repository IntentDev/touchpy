@echo off

setlocal enabledelayedexpansion

set "args=%*"

:parse_args
if "%args%" == "" goto end_parse_args
for /f "tokens=1,2,* delims= " %%a in ("%args%") do (
    set "arg=%%a"
    set "args=%%c"  ; Update args to skip the current value

    if "!arg:~0,1!" == "-" (
        set "key=!arg:~1!"
        set "value_!key!=%%b"
    )
)
goto parse_args

:end_parse_args

if defined value_n (
    echo -n: !value_n!
)

if defined value_m (
    echo -m: !value_m!
)

endlocal
