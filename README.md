Dependencies:  
Visual Studio 2022, for now.  
  
You need to have installed the Vulkan SDK on your system.  
Visit https://vulkan.lunarg.com/ to download and install it.  
  
You also need Cmake  
Visit https://cmake.org/download/ to download and install it.  
  
Other dependencies are added as git submodules in the lib directory.  
  
Build instructions:

```
> git clone https://github.com/framebluffer/hdrToKtx2

> git submodule update --init --recursive

> dep_build.cmd <Debug | Release>   // To install the submodule dependencies

> msbuild hdrToKtx2.sln /p:configuration=<Debug | Release>  // Depending on if you installed dependencies with Debug or Release
```

Produces executable hdrToKtx2.exe in x64/<Debug | Release>/hdrToKtx2.exe  

Usage:  
Takes hdr file as input and produces .ktx2 environment map textures set, for use in image based lighting. Including irradiance map, radiance map, brdf lookup table, and skybox.  
can optionally basis compress textures.  
can optionally tonemap skybox texture.  
Running hdrToKtx2.exe will print usage instuctions and options.  
