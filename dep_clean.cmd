@echo off
setlocal

set keepfile=.gitignore

set cleanDirectory=%~dp0lib\build\glTF-IBL-Sampler
call :cleanDir

set cleanDirectory=%~dp0lib\build\basis_universal
call :cleanDir

set cleanDirectory=%~dp0lib\build\glm
call :cleanDir

set cleanDirectory=%~dp0include\glTF-IBL-Sampler
call :cleanDir

set cleanDirectory=%~dp0include\ktx
call :cleanDir

set cleanDirectory=%~dp0include\stb
call :cleanDir

set cleanDirectory=%~dp0include\basis_universal\encoder
call :cleanDir

set cleanDirectory=%~dp0include\basis_universal\OpenCL
call :cleanDir

set cleanDirectory=%~dp0include\glm
call :cleanDir

:cleanDir
for /d %%a in (%cleanDirectory%\*) do rmdir /s /q %%a
for %%a in (%cleanDirectory%\*) do if /i not %%~nxa==%keepfile% del %%a
goto :eof
