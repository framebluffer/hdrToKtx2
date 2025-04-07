@echo off

set conf=%1

echo cleaning lib build directories
call dep_clean.cmd

pushd %~dp0lib\build\glTF-IBL-Sampler
cmake ..\..\glTF-IBL-Sampler\CMakeLists.txt
msbuild glTFIBLSampler.sln /p:configuration=%conf%
popd

pushd %~dp0lib\build\basis_universal
cmake ..\..\basis_universal\CMakeLists.txt -DSSE=TRUE -DOPENCL=TRUE
msbuild basisu.sln /p:configuration=%conf%
popd

pushd %~dp0lib\build\glm
cmake -DGLM_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=OFF ..\..\glm\CMakeLists.txt
msbuild glm.sln /p:configuration=%conf%
popd

echo Copying lib header files
copy %~dp0lib\glTF-IBL-Sampler\lib\include\*.h %~dp0include\glTF-IBL-Sampler /Y
copy %~dp0lib\glTF-IBL-Sampler\lib\source\FileHelper.h %~dp0include\glTF-IBL-Sampler /Y
copy %~dp0lib\glTF-IBL-Sampler\thirdparty\KTX-Software\include\*.h %~dp0include\ktx /Y
copy %~dp0lib\glTF-IBL-Sampler\thirdparty\KTX-Software\lib\vkformat_enum.h %~dp0include\ktx /Y
copy %~dp0lib\glTF-IBL-Sampler\thirdparty\stb\*.h %~dp0include\stb /Y
copy %~dp0lib\basis_universal\transcoder\*.h %~dp0include\basis_universal\transcoder /Y
copy %~dp0lib\basis_universal\encoder\*.h %~dp0include\basis_universal\encoder /Y
copy %~dp0lib\basis_universal\OpenCL\CL\*.h %~dp0include\basis_universal\OpenCL /Y
robocopy %~dp0lib\glm\glm %~dp0include\glm *.h *.hpp *.inl /S

echo Copying lib dll's
copy /b/v/y %~dp0lib\build\glTF-IBL-Sampler\%conf%\*.dll %~dp0x64\%conf% /Y
