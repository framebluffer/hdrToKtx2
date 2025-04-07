Build instructions:

```
> git clone https://github.com/framebluffer/hdrToKtx2

> git submodule update --init --recursive

> dep_build.cmd <Debug | Release>

> msbuild hdrToKtx2.sln
```

Produces executable hdrToKtx2.exe in x64/<Debug | Release>/hdrToKtx2.exe  

Usage:  
Takes hdr file as input and produces .ktx2 environment map textures set, for use in image based lighting. Including irradiance map, radiance map, brdf lookup table, and skybox.  
can optionally basis compress textures.  
can optionally tonemap skybox texture.
Running hdrToKtx2.exe will print usage instuctions and options.  
