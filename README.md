This is a demo project which basically presents an outdoor scene created in OpenGL 3.3 for Windows OS.
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
