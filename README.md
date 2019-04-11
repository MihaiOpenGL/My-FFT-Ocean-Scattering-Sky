This is a demo project which basically presents an outdoor scene created in OpenGL 3.2 for Windows, Linux & MacOSX.
For the used GL extensions check: GLUsedExtensionList.txt.

The scene consists of:
* Cloudy Sky
* Ocean
* Motor Boat

Check the images in screenshots/ to see how the end result looks!

### Controls

The main controls are: keyboard and mouse

Once the demo has started the main controllable object is the camera (by default).
It is a free flying camera, so one can go anywhere and take a look.

The keyboard controls are:
W - forward
S - backward
A - strafe left
D - strafe right
U - go up
B - go down

` - lock/unlock the mouse pointer (useful when sun moing with mouse is alowed, see below)
1 - wireframe mdeo on/off
2 - point mode on/off
3 - boat mode on/off
4 - show/hide GUI

When in boat mode the camera freezes and the W, S, A, D are used to move and turn the boat.

////////////////////

The project was concieved with flexibility in mind, so its parameters are cached in a XML config file.
Check the GlobalConfig.xml file. It has many parameters, but the main points of interest are:

### Camera - position, FOV

### VisualEffects - mainly the Post Processing Effects
Possible values are: 
EffectInvert, EffectGrey, EffectBlackWhite, EffectSepia, EffectWavy, EffectBlur, EffectEdgeDetection, NoEffect

Also one can disable/enabled the reflections and refractions.
NOTE! Refractions are only available underwater (only the boat).

### Scene:

a) ##Sky

Sky is of course what you see when you look up. It may contain clouds or not, it depends on the model.

Sky has 3 model types.

a.1) Possible type are:
* SkyCubemap - just a simple cubmap (always has clouds)
* SkyScattering - a more complex implementation which simulation light scattering in atmosphere, accounts for sky color, sun color
* SkyPrecomputedScattering - an even more complex implementation based on prebaked data, accounts for sky, clouds and sun colors

For the last 2 models the clouds can be enabled or disabled.

a.2) #Moving sun

Sun can be moved using the mouse, see the AllowChangeDirWithMouse option or move itself see the option IsDynamic.
InitialTheta and InitialPhi options represent the initial position of the sun. The values should be provided in degrees (internally are converted to radians).

b) ##Ocean

Ocean is rendered as infinite using a special technique called projected grid. More info in the source code.

There is only one ocean type, but it grid may vary.

b.1) #Grid types:
Possible types are: 
* GridWorldSpace - grid is projected from post-perspective space to world space.
* GridScreenSpace - grid is projected from screen space to world space.

NOTE! FOr the screen space grid cehckout the GridResolution options (it determines how spare is the grid).

Check these out to see the differences.

b.2) #Surface

The ocean surface provides some nice visual effects like: foam and sub surface scattering.

Check the Foam and SubSurfaceScattering options respectively.

b.2.1) #FFT Patch

The ocean was consist of 2 things: FFT waves and perlin noise waves.

FFT waves are near the camera and are the most interesting ones, being maniplulated by many parameters including wind.

Check the options: FFTSize, PatchSize, WaveAmpltitude, WindSpeed and WindDirection under OceanPatch main option.

* FFTSize can vary from 128 to 1024. Recommended values are: 256 and 512.
* PatchSize also can vary, it usually is equal to FFTSize is slightly bigger.
* WaveAmplitude is in between [0.0, 1.0]
* WindS]peed is in between [0.0, 200.0]
* WindDirection has 2 directions: OX and OZ, both can vary in [-1.0, +1.0]

b.2.1.1) #FFT Waves computation

Now there 3 possible ways to ompute the FFT Patch:
* GPU using fragment shader 
* GPU using compute shaders (your video card must support OpenGL 4.4)
* CPU using FFTW lib

In each case the config options is: FFTGpuFrag, FFTGpuComp and FFTCpuFFTW under ComputeFFT -> Type.

b.2.1.2) #FFT Normals computation

The waves normals also can be computed in various ways, mainly in 2:
* GPU - fragment shader
* GPU - compute shaders

For each case the config options are: NormalGpuFrag and NormalGpuComp

b.2.2) #Wave spectrum

Waves form mainly depends on the spectrum used to simulate them.

The project offers 2 spectra: Phillips and Unified. More info about these in the source code.

The config options are: SpectrumPhillips and SpectrumUnified and can be set under Spectrum -> Type option.

b.2.3) #Perlin Noise

As mentioned above perin noise is used to simulate moving far away waves.

In this case no perlin noise computation is done, instead a prebaked texture is used with displacement and normals.

Still, there are some options for flexibility. Check out: Octaves, Amplitudes, Gradients and Speed under PerlinNoise main option.

b.2.4) #Boat Effects

As we have an ocean and a motor boat on it hence the presence of some effects.

If the boat is still, then only the stationary foam can be seen around the boat.

If the boat is moving (how to trigger this will be show later on) then we have a couple of waves generated by the boat
that can be obserbed clearly:

* Bow Wakes - also know as Kelin wakes - the waves produces by the front of the boat on water. Usually under boat speed they appear in a V shape.
* Stern Wakes - also know as propperl wash - the waves generated by the rear or propeller of the boat as it moves along. Higher speed of the boat
longer wake as a trail.

So, the options are: Foam, KelvinWake and PropellerWash repectively.

b.3) #Underwater

Yes, we have some underwater efects as well!

Mainly can be observed: Fog and underwater god rays. Of course the other sie of the waves is also present (underwater) :)

The fog is implemented as exponential. God rays are implemented in screen space which is a limitation unfortunatelly (you will see why).

The good news is that god rays are really nicely done and also hoe options of their own.

The options are: Fog and GodRays under the UnderWater option.

b.4) #Bottom

Yep, we also have a bottom with a couple of effects.

We have fog which is just a simple exponential fog as before.

Caustics are the main attraction here - basically the refraction of light form the waves on the ocean bottom.

Check the config options: Fog and Caustics under the Bottom option.

Also, the bottom is implemented as displaced having a perlin noise applied to it to account for the udnerwater small hills :)

c) ##Boat

We have a simple motor boat. 
What really matters is the position of the boat. For this checkout Boat -> Position option.

Under BoatEffects option we also have these options:

* Buoyancy -> Enabled - enabled buoyancy on boat (only vertical force is implemented).
* HideInsideWater - shows or hides the water in the inside the boat

/////////////////////////////

### HOW TO BUILD

a) # Windows

UPDATE! Now its proj files have been updated to VS 2017!

This project was built using Visual Studio 2015 Community Edition. Its project files reflect this.
In case you use VS 2017 you can retarget the project. In case you use an older version of VS you can change the toolset yourself.
Both Debug and Release modes work.

NOTE! The dependecy libs - SDL2, fftw and AntweakBar which needed to be built depend on the os!
NOTE! For Windows I used the pre-built libs offered by the developers of those libs.
NOTE! I dropped GLEW and switched to GLAD as it can generate the source files according to your exact needs. Check the source code for more details.

TOOLS: Visual Studio 2017
Other requirements: N/A

b) # Linux

Check the Makefile.
Run: make CFG=release for release build, release is also the default rule.
For debug run: make CFG=debug.
To cleanup run: make clean CFG=release or just make clean for release.
To cleanup for debug run: make clean CFG=debug.

NOTE! The dependecy libs - SDL2, fftw and AntweakBar which needed to be built depend on the os!
NOTE! For Linux(Ubuntu) I compiled them myself.
NOTE! I dropped GLEW and switched to GLAD as it can generate the source files according to your exact needs. Check the source code for more details.

TOOLS: g++, make
Other requirements: install the package libglu10mesa-dev to solve the glu.h header issues while compiling

c) # Mac OS X

Check the Makefile_osx.
Run: make -f Makefile_osx CFG=release for release build, release is also the default rule.
For debug run: make -f Makefile_osx CFG=debug.
To cleanup run: make -f Makefile_osx clean CFG=release or just make -f Makefile_osx clean for release.
To cleanup for debug run: make -f Makefile_osx clean CFG=debug.

NOTE! The dependecy libs - SDL2, fftw and AntweakBar which needed to be built depend on the os!
NOTE! For MacOSX(HighSierra, Mojave) I compiled them myself.
NOTE! I dropped GLEW and switched to GLAD as it can generate the source files according to your exact needs. Check the source code for more details.

NOTE! If the built libs are used on an older version of Mac OS X there may be linking warnings!

TOOLS: XCode (it includes g++, make)
Other requirements: N/A


### TESTS
I have no automatic tests. A couple of manual tests were performed on each platform.

** On Windows 10 I used this setup:

- nvidia drivers directly.
- supported opengl version = 4.60
- tests went smooth, no issues.
- config - mostly ok
if using AMD card the app works with window hints
if using nVidia card the app doesn't work wih the window hints - driver issue I presume

** On Ubuntu I used this setup:

a) native hardware using installed Ubuntu or Ubuntu live stick
- nvidia drivers directly
- supported opengl version = 4.60
- tests went smooth, no issues.
- config - some issues if using the core profile.
It may be better if window hints are not used

NOTE! It is better to install Ubuntu than use the live stick as with the live stick there may be issues regarding the drivers installation.

b) vmware machine with Ubuntu as guest
- Mesa 18.3.3 version of the driver for SVGA vmware configuration
- supported opengl version = 3.30 (max at this point)
- vmware offers hardware acceleration
- encountered rendering issues like no correct waves displacement when using FFTGpuFrag type,
because of the driver no supporting fragment shader multiple output streas properly.
The sahders compiled and linked correctly, but the issue was there at runtime.
Another issue was the controls: keybord all good, but the mouse dx and dy were totally off,
because of another driver bug.
- config - some issues if using the core profile.
It may be better if window hints are not used

** On Mac OS X I used this setup:
a) native hardware using a separate hard drive
It wasn't easy, but in the end I managed to create a hackintosh an a separate hard driver and use it to test the application.
- nvidia web drivers (special drivers for mac)
- supported opengl version = 4.10
- tests went almost smooth. Issue - I had mouse movement, but couldn't access the GUI, keyboard ok.
- config - mac os x is limited to these parameters:
* windows hints - on
* opengl version >= 3.2
* core profile - on

* Without proper drivers & clover setup for the nvidia card, the system falls back to the apple software renderer, which is no good.
Using the software renderer in this case, I managed to start the app and see some garphics, but it had many artifacts and was not 
feasable for testing - extremely low fps.

*I had to use Mac os X High Sierra, as it is the only last mac os version which supports the last nVidia 10x family (Polaris) cards.
Mojave has no proper support in this case.

Useful links:
To create a usb stick with mac os high sierra + clover setup to be able to boot it on non-mac hardware
https://hackintosher.com/guides/high-sierra-install-full-guide/#bios
https://hackintosher.com/guides/macos-high-sierra-hackintosh-install-clover-walkthrough/

Using clover + nvidia web drivers to have access to a dedicate video card + hardware acceleration
https://github.com/Benjamin-Dobell/nvidia-update/blob/master/nvidia-update.sh

b) vmware machine with Mac OS X (Mojave) as guest
- first vmware doesn't support mac os systems yet, so I unlocked this feature using 
the Unlocker tool: https://github.com/DrDonk/unlocker/releases
this I've done prior to creating a machine for mac os x.
- no hardware acceleration for Mac os
- mac os x i vmware offers only an opengl software renderer
- supported opengl version = 4.10
- I managed to compile & link, but no actual tests were feasable,
because of extremely low framerate

* Useful links:
Mac os x vmware disk:
https://www.geekrar.com/download-macos-mojave-vmware-virtualbox/

I used these steps in the mac os x virtual machine in my attempt to create a live or extarnal disck with mac os x on it
http://osxdaily.com/2018/09/29/download-full-macos-mojave-installer/

https://www.macworld.co.uk/how-to/mac-software/macos-external-hard-drive-3659666/

https://lifehacker.com/how-to-create-a-portable-hackintosh-on-a-usb-thumb-driv-5739259



### License

This code and its resources are licensed under MIT License

Copyright (c) 2018 Bairac Mihai

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
