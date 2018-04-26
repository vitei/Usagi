usagi
=====

Usagi is a Hierarchical Entity-Component-System based game engine created by [Vitei Inc.](http://www.vitei.com/) designed to priortise performance.  
The Component-Entity-System design has taken the games industry by a storm over the last ten years or so, but we have applied some of our own innovations which come out of arranging our entities in a hierarchy to help control the flow of execution, and allows us to make a number of thread-safe optimisations.  

Usagi was developed to allow us to create demanding action titles on limited hardware, however the unique design of its component entity system should be suitable for any genre and should be even better suited to more modern hardware. It is actually a relatively new engine, work only began on it in 2013, and as such it was designed to be performant on multi-core systems. As Usagi was only ever used in one commerical title which for all practical purposes only had access to a single core, many of benefits remain theoretical. Even on a single core the cache friendly nature of the design did give superb performance however.  

 Internally demos were created for other platforms and as such support for slightly more advanced lighting and effects is in place, but certainly nothing bleeding edge.  


 Certain code has to be removed for release:   
 Platform specific code other than windows  
 2D UI and menu code (due to their dependence on propriatary formats)   
 Some of the networking code (as it did not follow the standards for platform independent coding)  
 
 More detailed information is available in the [wiki](https://github.com/vitei/Usagi/wiki).  

Getting up and running
----------------------

The build process has not been improved or significantly modified and currently has a number of dependencies which can not be directly included in an open source project.  Do *not* clone this project first as it should be checked out to a sub folder of a project which uses the engine.

1. Install **Visual Studio 2017**.  
   https://www.visualstudio.com/downloads/

1. Install **Ruby 2.3.3**.  
   Make sure that you select the option to add a path variable  
   Note that versions newer than 2.4 will not function with nokogiri  
   https://rubyinstaller.org/

1. Install **Python 2.7.14**.  
   Make sure that you select the option to add a path variable  
   https://www.python.org/downloads/

1. Clone the **PhysX repository 3.4**.  
   Requires accepting a EULA  
   https://developer.nvidia.com/physx-source-github  

1. Build the PhysX binaries  
   [**PhysX Checkout Dir**]\PhysX_3.4\Source\compiler\vc15win64\PhysX.sln and compile for both debug and release  
   Use the project located in Source\compiler\vc15win64  
   Note that you must currently change the default options to build with Multi-threaded Debug DLL and Multi-threaded DLL

1. **Optional** Install the **FBX SDK 2018.1.1 VS2015**.  
   Not necessary to run, but required to build the model converter (Ayataka)
   https://www.autodesk.com/products/fbx/overview

1. **Optional** Install and build the **Oculus SDK for Windows**.  
   Not necessary to run, but required if you want to make a VR app
   Again Note that you must currently change the default options to build with Multi-threaded Debug DLL and Multi-threaded DLL
   https://developer.oculus.com/downloads/package/oculus-sdk-for-windows/

1. Clone a **Usagi Project**.  
   For example [UsagiTest](https://github.com/vitei/UsagiTest)  
   Always clone Usagi repos recursively as we make heavy use of submodules

1. From a command window run **Setup.bat**  
   If gems fail to install confirm your ruby version  
   If python packages fail to install confirm your python version  
   Pay attention to which environemt variables EnvironmentSetup.exe failed to set  
   Note you must run environemnt setup whenever you switch to running a different Usagi project in order to set the correct USAGI_DIR env variable  

1. Add the following environment variables either manually or by running EnvironmentSetup.exe  
   MSBUILD_DIR -> [**Visual Studio Install Dir**]\MSBuild\15.0\Bin  
   FBXSDK_DIR -> [**FBX SDK Install Dir**]\2018.1.1  
   USAGI_DIR -> [**This checkout**]  

1. Manually add the following environment variables  
   EnvironmentSetup.exe may have been able to automatically find and add them if they were in ../../PhysX-3.4 and ../../OculusSDK  
   PHYSX_DIR -> [**PhysX Checkout Dir**]\PhysX_3.4  
   OCULUS_SDK_DIR -> [**Oculus SDK directory**]  

1. Follow the instructions in that projects README.md  

Issues
----------------------

**The current is a list of known issues with Usagi which need to be addressed.**

There is no longer a system for linking projects and engine versions

A replacement model editor would be required to take advantage of the existing custom shader support

Resource loading is slow and single threaded

Resources don't have an internal list of dependenices

The Vulkan implementation is only half complete

The running of systems is not yet multi-threaded

Spot and projection lights are not properlly culled

If the window loses focus at the wrong time creating the render targets will fail

The AI can not currently handle a 3D space


License
----------------------

Usagi its self is available under the MIT licence (see [LICENSE](LICENSE) in this repository). We would appreciate you letting us know if you make use of any part of the code or the design; but there is no requirement to do so.  
Third party software falls under the MIT, Modified BSD and zLib licenses, the specifics of which are detailed in [Documents/License.md](Documents/License.md)
