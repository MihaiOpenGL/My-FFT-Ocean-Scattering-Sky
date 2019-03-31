/* Author: BAIRAC MIHAI */

#include "GlobalConfig.h"
#include "CommonHeaders.h"
// glm::vec2, glm::vec3 come from the header
#include "glm/trigonometric.hpp" //radians()
#include <sstream>
#include <assert.h>


GlobalConfig::GlobalConfig ( void )
	: m_pConfigParser(nullptr)
{
	LOG("GlobalConfig successfully created!");
}

GlobalConfig::GlobalConfig ( const std::string& i_FileName )
{
	Initialize(i_FileName);
}

GlobalConfig::~GlobalConfig ( void )
{
	Destroy();
}

void GlobalConfig::Destroy ( void )
{
	// free resources
	SAFE_DELETE(m_pConfigParser);

	LOG("GlobalConfig successfully destroyed!");
}

bool GlobalConfig::Initialize ( const std::string& i_FileName )
{
	// Parse the config file
	m_pConfigParser = new XMLParser(i_FileName);
	assert(m_pConfigParser != nullptr);
	LOG("ConfigParser is: %d bytes in size", sizeof(*m_pConfigParser));

	if (!Setup())
	{
		return false;
	}

	LOG("GlobalConfig successfully created!");
	return true;
}


bool GlobalConfig::Setup ( void )
{
	std::map<std::string, XMLGenericType> keyMap;
	m_pConfigParser->PopulateKeyMap(keyMap);

	// Setup Config variables
	Window.UseWindowHints = keyMap["GlobalConfig.Window.UseWindowHints"].ToBool();
	Window.IsWindowMode = keyMap["GlobalConfig.Window.IsWindowMode"].ToBool();
	// Available only in Window Mode TODO - this mode is not fully operable
	Window.IsWindowResizable = keyMap["GlobalConfig.Window.IsWindowResizable"].ToBool();

	OpenGLContext.OpenGLVersion.major = keyMap["GlobalConfig.OpenGLContext.OpenGLVersion.major"].ToInt();
	OpenGLContext.OpenGLVersion.minor = keyMap["GlobalConfig.OpenGLContext.OpenGLVersion.minor"].ToInt();
	OpenGLContext.IsCoreProfile = keyMap["GlobalConfig.OpenGLContext.IsCoreProfile"].ToBool();

	Input.KeySpeed = keyMap["GlobalConfig.Input.KeySpeed"].ToFloat();
	Input.MouseSpeed = keyMap["GlobalConfig.Input.MouseSpeed"].ToFloat();

	Camera.UseConstraints = keyMap["GlobalConfig.Camera.UseConstraints"].ToBool();
	Camera.InitialPosition = keyMap["GlobalConfig.Camera.InitialPosition"].ToVec3();
	Camera.InitialPitch = keyMap["GlobalConfig.Camera.InitialPitch"].ToFloat();
	Camera.InitialYaw = keyMap["GlobalConfig.Camera.InitialYaw"].ToFloat();
	Camera.InitialFieldOfView = keyMap["GlobalConfig.Camera.InitialFieldOfView"].ToFloat(); // 45 degrees FOV is recommended for usual outdoor scenes
	Camera.InitialZNear = keyMap["GlobalConfig.Camera.InitialZNear"].ToFloat();
	Camera.InitialZFar = keyMap["GlobalConfig.Camera.InitialZFar"].ToFloat();

	Simulation.TimeScale = keyMap["GlobalConfig.Simulation.TimeScale"].ToFloat();
	Simulation.ShowGUI = keyMap["GlobalConfig.Simulation.ShowGUI"].ToBool();

	Rendering.HDR.Enabled = keyMap["GlobalConfig.Rendering.HDR.Enabled"].ToBool();
	Rendering.HDR.Exposure = keyMap["GlobalConfig.Rendering.HDR.Exposure"].ToFloat();

	Shaders.UseStrictVerification = keyMap["GlobalConfig.Shaders.UseStrictVerification"].ToBool();

	// Texture Allocation
	TexUnit.Global.ReflectionMap = keyMap["GlobalConfig.TexUnit.Global.ReflectionMap"].ToInt();
	TexUnit.Global.RefractionMap = keyMap["GlobalConfig.TexUnit.Global.RefractionMap"].ToInt();
	TexUnit.Global.PostProcessingMap = keyMap["GlobalConfig.TexUnit.Global.PostProcessingMap"].ToInt();
	TexUnit.Sky.CubeMapSkyModel.CubeMap = keyMap["GlobalConfig.TexUnit.Sky.CubeMapSkyModel.CubeMap"].ToInt();
	TexUnit.Sky.PrecomputedScatteringSkyModel.IrradianceMap = keyMap["GlobalConfig.TexUnit.Sky.PrecomputedScatteringSkyModel.IrradianceMap"].ToInt();
	TexUnit.Sky.PrecomputedScatteringSkyModel.InscatterMap = keyMap["GlobalConfig.TexUnit.Sky.PrecomputedScatteringSkyModel.InscatterMap"].ToInt();
	TexUnit.Sky.PrecomputedScatteringSkyModel.TransmittanceMap = keyMap["GlobalConfig.TexUnit.Sky.PrecomputedScatteringSkyModel.TransmittanceMap"].ToInt();
	TexUnit.Sky.PrecomputedScatteringSkyModel.NoiseMap = keyMap["GlobalConfig.TexUnit.Sky.PrecomputedScatteringSkyModel.NoiseMap"].ToInt();
	TexUnit.Ocean.CPU2DIFFT.FFTMap = keyMap["GlobalConfig.TexUnit.Ocean.CPU2DIFFT.FFTMap"].ToInt();
	TexUnit.Ocean.GPU2DIFFT.ButterflyMap = keyMap["GlobalConfig.TexUnit.Ocean.GPU2DIFFT.ButterflyMap"].ToInt();
	TexUnit.Ocean.GPU2DIFFT.PingArrayMap = keyMap["GlobalConfig.TexUnit.Ocean.GPU2DIFFT.PingArrayMap"].ToInt(); //11, 12
	TexUnit.Ocean.GPU2DIFFTComp.IndicesMap = keyMap["GlobalConfig.TexUnit.Ocean.GPU2DIFFTComp.IndicesMap"].ToInt();
	TexUnit.Ocean.GPU2DIFFTComp.WeightsMap = keyMap["GlobalConfig.TexUnit.Ocean.GPU2DIFFTComp.WeightsMap"].ToInt();
	TexUnit.Ocean.GPU2DIFFTComp.PingArrayMap = keyMap["GlobalConfig.TexUnit.Ocean.GPU2DIFFTComp.PingArrayMap"].ToInt(); //12, 13
	TexUnit.Ocean.FFTNormalGradientFoldingBase.NormalGradientFoldingMap = keyMap["GlobalConfig.TexUnit.Ocean.FFTNormalGradientFoldingBase.NormalGradientFoldingMap"].ToInt();
	TexUnit.Ocean.FFTOceanPatchGPUFrag.FFTInitDataMap = keyMap["GlobalConfig.TexUnit.Ocean.FFTOceanPatchGPUFrag.FFTInitDataMap"].ToInt();
	TexUnit.Ocean.FFTOceanPatchGPUComp.FFTInitDataMap = keyMap["GlobalConfig.TexUnit.Ocean.FFTOceanPatchGPUComp.FFTInitDataMap"].ToInt();
	TexUnit.Ocean.Surface.PerlinDisplacementMap = keyMap["GlobalConfig.TexUnit.Ocean.Surface.PerlinDisplacementMap"].ToInt();
	TexUnit.Ocean.Surface.WavesFoamMap = keyMap["GlobalConfig.TexUnit.Ocean.Surface.WavesFoamMap"].ToInt();
	TexUnit.Ocean.Surface.BoatFoamMap = keyMap["GlobalConfig.TexUnit.Ocean.Surface.BoatFoamMap"].ToInt();
	TexUnit.Ocean.Surface.PropellerWashMap = keyMap["GlobalConfig.TexUnit.Ocean.Surface.PropellerWashMap"].ToInt();
	TexUnit.Ocean.Surface.KelvinWakeDispNormMap = keyMap["GlobalConfig.TexUnit.Ocean.Surface.KelvinWakeDispNormMap"].ToInt();
	TexUnit.Ocean.Surface.KelvinWakeFoamMap = keyMap["GlobalConfig.TexUnit.Ocean.Surface.KelvinWakeFoamMap"].ToInt();
	TexUnit.Ocean.UnderWater.GodRaysMap = keyMap["GlobalConfig.TexUnit.Ocean.UnderWater.GodRaysMap"].ToInt();
	TexUnit.Ocean.Bottom.SandDiffuseMap = keyMap["GlobalConfig.TexUnit.Ocean.Bottom.SandDiffuseMap"].ToInt();
	TexUnit.Ocean.Bottom.CausticsMap = keyMap["GlobalConfig.TexUnit.Ocean.Bottom.CausticsMap"].ToInt();
	TexUnit.MotorBoat.BoatDiffMap = keyMap["GlobalConfig.TexUnit.MotorBoat.BoatDiffMap"].ToInt();
	TexUnit.MotorBoat.BoatNormalMap = keyMap["GlobalConfig.TexUnit.MotorBoat.BoatNormalMap"].ToInt();

	VisualEffects.ShowReflections = keyMap["GlobalConfig.VisualEffects.ShowReflections"].ToBool();
	VisualEffects.ShowRefractions = keyMap["GlobalConfig.VisualEffects.ShowRefractions"].ToBool();
	VisualEffects.PostProcessing.Enabled = keyMap["GlobalConfig.VisualEffects.PostProcessing.Enabled"].ToBool();
	VisualEffects.PostProcessing.EffectType = keyMap["GlobalConfig.VisualEffects.PostProcessing.EffectType"].ToPostProcessingEffectType();

	Scene.Sky.Model.Type = keyMap["GlobalConfig.Scene.Sky.Model.Type"].ToSkyModelType();

	Scene.Sky.Model.Cubemap.Sun.AllowChangeDirWithMouse = keyMap["GlobalConfig.Scene.Sky.Model.Cubemap.Sun.AllowChangeDirWithMouse"].ToBool();
	Scene.Sky.Model.Cubemap.Sun.IsDynamic = keyMap["GlobalConfig.Scene.Sky.Model.Cubemap.Sun.IsDynamic"].ToBool();
	Scene.Sky.Model.Cubemap.Sun.MoveFactor = keyMap["GlobalConfig.Scene.Sky.Model.Cubemap.Sun.MoveFactor"].ToFloat();
	Scene.Sky.Model.Cubemap.Sun.InitialTheta = glm::radians(keyMap["GlobalConfig.Scene.Sky.Model.Cubemap.Sun.InitialTheta"].ToFloat()); // degrees to radians
	Scene.Sky.Model.Cubemap.Sun.InitialPhi = glm::radians(keyMap["GlobalConfig.Scene.Sky.Model.Cubemap.Sun.InitialPhi"].ToFloat()); // degrees to radians
	Scene.Sky.Model.Cubemap.Sun.Shininess = keyMap["GlobalConfig.Scene.Sky.Model.Cubemap.Sun.Shininess"].ToFloat();
	Scene.Sky.Model.Cubemap.Sun.Strength = keyMap["GlobalConfig.Scene.Sky.Model.Cubemap.Sun.Strength"].ToFloat();
	Scene.Sky.Model.Cubemap.Sun.SunFactor = keyMap["GlobalConfig.Scene.Sky.Model.Cubemap.Sun.SunFactor"].ToFloat();

	if (Scene.Sky.Model.Cubemap.Sun.IsDynamic) Scene.Sky.Model.Cubemap.Sun.AllowChangeDirWithMouse = false;

	Scene.Sky.Model.Scattering.Sun.AllowChangeDirWithMouse = keyMap["GlobalConfig.Scene.Sky.Model.Scattering.Sun.AllowChangeDirWithMouse"].ToBool();
	Scene.Sky.Model.Scattering.Sun.IsDynamic = keyMap["GlobalConfig.Scene.Sky.Model.Scattering.Sun.IsDynamic"].ToBool();
	Scene.Sky.Model.Scattering.Sun.MoveFactor = keyMap["GlobalConfig.Scene.Sky.Model.Scattering.Sun.MoveFactor"].ToFloat();
	Scene.Sky.Model.Scattering.Sun.InitialTheta = glm::radians(keyMap["GlobalConfig.Scene.Sky.Model.Scattering.Sun.InitialTheta"].ToFloat()); // degrees to radians
	Scene.Sky.Model.Scattering.Sun.InitialPhi = glm::radians(keyMap["GlobalConfig.Scene.Sky.Model.Scattering.Sun.InitialPhi"].ToFloat()); // degrees to radians
	Scene.Sky.Model.Scattering.Sun.Shininess = keyMap["GlobalConfig.Scene.Sky.Model.Scattering.Sun.Shininess"].ToFloat();
	Scene.Sky.Model.Scattering.Sun.Strength = keyMap["GlobalConfig.Scene.Sky.Model.Scattering.Sun.Strength"].ToFloat();
	Scene.Sky.Model.Scattering.Atmosphere.SampleCount = keyMap["GlobalConfig.Scene.Sky.Model.Scattering.Atmosphere.SampleCount"].ToInt();
	Scene.Sky.Model.Scattering.Atmosphere.RayleighScatteringConstant = keyMap["GlobalConfig.Scene.Sky.Model.Scattering.Atmosphere.RayleighScatteringConstant"].ToFloat();//0.0025f;
	Scene.Sky.Model.Scattering.Atmosphere.MieScatteringConstant = keyMap["GlobalConfig.Scene.Sky.Model.Scattering.Atmosphere.MieScatteringConstant"].ToFloat();
	Scene.Sky.Model.Scattering.Atmosphere.SunBrightnessConstant = keyMap["GlobalConfig.Scene.Sky.Model.Scattering.Atmosphere.SunBrightnessConstant"].ToFloat();
	Scene.Sky.Model.Scattering.Atmosphere.WaveLength = keyMap["GlobalConfig.Scene.Sky.Model.Scattering.Atmosphere.WaveLength"].ToVec3();
	Scene.Sky.Model.Scattering.Atmosphere.InnerRadius = keyMap["GlobalConfig.Scene.Sky.Model.Scattering.Atmosphere.InnerRadius"].ToFloat();
	Scene.Sky.Model.Scattering.Atmosphere.OuterRadius = keyMap["GlobalConfig.Scene.Sky.Model.Scattering.Atmosphere.OuterRadius"].ToFloat();
	Scene.Sky.Model.Scattering.Atmosphere.GeometrySliceCount = keyMap["GlobalConfig.Scene.Sky.Model.Scattering.Atmosphere.GeometrySliceCount"].ToInt();
	Scene.Sky.Model.Scattering.Atmosphere.AltitudeOffset = keyMap["GlobalConfig.Scene.Sky.Model.Scattering.Atmosphere.AltitudeOffset"].ToFloat();
	Scene.Sky.Model.Scattering.Clouds.Enabled = keyMap["GlobalConfig.Scene.Sky.Model.Scattering.Clouds.Enabled"].ToBool();
	Scene.Sky.Model.Scattering.Clouds.Octaves = keyMap["GlobalConfig.Scene.Sky.Model.Scattering.Clouds.Octaves"].ToInt();
	Scene.Sky.Model.Scattering.Clouds.Lacunarity = keyMap["GlobalConfig.Scene.Sky.Model.Scattering.Clouds.Lacunarity"].ToFloat();
	Scene.Sky.Model.Scattering.Clouds.Gain = keyMap["GlobalConfig.Scene.Sky.Model.Scattering.Clouds.Gain"].ToFloat();
	Scene.Sky.Model.Scattering.Clouds.ScaleFactor = keyMap["GlobalConfig.Scene.Sky.Model.Scattering.Clouds.ScaleFactor"].ToFloat();
	Scene.Sky.Model.Scattering.Clouds.Offset = keyMap["GlobalConfig.Scene.Sky.Model.Scattering.Clouds.Offset"].ToFloat();
	Scene.Sky.Model.Scattering.Clouds.Altitude = keyMap["GlobalConfig.Scene.Sky.Model.Scattering.Clouds.Altitude"].ToFloat();

	if (Scene.Sky.Model.Scattering.Sun.IsDynamic) Scene.Sky.Model.Scattering.Sun.AllowChangeDirWithMouse = false;

	Scene.Sky.Model.PrecomputedScattering.Sun.AllowChangeDirWithMouse = keyMap["GlobalConfig.Scene.Sky.Model.PrecomputedScattering.Sun.AllowChangeDirWithMouse"].ToBool();
	Scene.Sky.Model.PrecomputedScattering.Sun.IsDynamic = keyMap["GlobalConfig.Scene.Sky.Model.PrecomputedScattering.Sun.IsDynamic"].ToBool();
	Scene.Sky.Model.PrecomputedScattering.Sun.MoveFactor = keyMap["GlobalConfig.Scene.Sky.Model.PrecomputedScattering.Sun.MoveFactor"].ToFloat();
	Scene.Sky.Model.PrecomputedScattering.Sun.InitialTheta = glm::radians(keyMap["GlobalConfig.Scene.Sky.Model.PrecomputedScattering.Sun.InitialTheta"].ToFloat()); // degrees to radians
	Scene.Sky.Model.PrecomputedScattering.Sun.InitialPhi = glm::radians(keyMap["GlobalConfig.Scene.Sky.Model.PrecomputedScattering.Sun.InitialPhi"].ToFloat()); // degrees to radians
	Scene.Sky.Model.PrecomputedScattering.Atmosphere.SunIntensity = keyMap["GlobalConfig.Scene.Sky.Model.PrecomputedScattering.Atmosphere.SunIntensity"].ToFloat();
	Scene.Sky.Model.PrecomputedScattering.Atmosphere.MieScattering = keyMap["GlobalConfig.Scene.Sky.Model.PrecomputedScattering.Atmosphere.MieScattering"].ToFloat();
	Scene.Sky.Model.PrecomputedScattering.Atmosphere.Rgtl = keyMap["GlobalConfig.Scene.Sky.Model.PrecomputedScattering.Atmosphere.Rgtl"].ToVec3();
	Scene.Sky.Model.PrecomputedScattering.Atmosphere.BetaR = keyMap["GlobalConfig.Scene.Sky.Model.PrecomputedScattering.Atmosphere.BetaR"].ToVec3();
	Scene.Sky.Model.PrecomputedScattering.Clouds.Enabled = keyMap["GlobalConfig.Scene.Sky.Model.PrecomputedScattering.Clouds.Enabled"].ToBool();
	Scene.Sky.Model.PrecomputedScattering.Clouds.Octaves = keyMap["GlobalConfig.Scene.Sky.Model.PrecomputedScattering.Clouds.Octaves"].ToInt();
	Scene.Sky.Model.PrecomputedScattering.Clouds.Lacunarity = keyMap["GlobalConfig.Scene.Sky.Model.PrecomputedScattering.Clouds.Lacunarity"].ToFloat();
	Scene.Sky.Model.PrecomputedScattering.Clouds.Gain = keyMap["GlobalConfig.Scene.Sky.Model.PrecomputedScattering.Clouds.Gain"].ToFloat();
	Scene.Sky.Model.PrecomputedScattering.Clouds.Norm = keyMap["GlobalConfig.Scene.Sky.Model.PrecomputedScattering.Clouds.Norm"].ToFloat();
	Scene.Sky.Model.PrecomputedScattering.Clouds.Clamp1 = keyMap["GlobalConfig.Scene.Sky.Model.PrecomputedScattering.Clouds.Clamp1"].ToFloat();
	Scene.Sky.Model.PrecomputedScattering.Clouds.Clamp2 = keyMap["GlobalConfig.Scene.Sky.Model.PrecomputedScattering.Clouds.Clamp2"].ToFloat();
	Scene.Sky.Model.PrecomputedScattering.Clouds.Color = keyMap["GlobalConfig.Scene.Sky.Model.PrecomputedScattering.Clouds.Color"].ToVec3();
	Scene.Sky.Model.PrecomputedScattering.Clouds.Offset = keyMap["GlobalConfig.Scene.Sky.Model.PrecomputedScattering.Clouds.Offset"].ToFloat();
	Scene.Sky.Model.PrecomputedScattering.Clouds.Altitude = keyMap["GlobalConfig.Scene.Sky.Model.PrecomputedScattering.Clouds.Altitude"].ToFloat();
	Scene.Sky.Model.PrecomputedScattering.Clouds.AltitudeOffset = keyMap["GlobalConfig.Scene.Sky.Model.PrecomputedScattering.Clouds.AltitudeOffset"].ToFloat();

	if (Scene.Sky.Model.PrecomputedScattering.Sun.IsDynamic) Scene.Sky.Model.PrecomputedScattering.Sun.AllowChangeDirWithMouse = false;

	Scene.Sky.UnderWaterColor = keyMap["GlobalConfig.Scene.Sky.UnderWaterColor"].ToVec3();

	Scene.Ocean.Grid.Type = keyMap["GlobalConfig.Scene.Ocean.Grid.Type"].ToOceanGridType();
	Scene.Ocean.Grid.WorldSpace.Width = keyMap["GlobalConfig.Scene.Ocean.Grid.WorldSpace.Width"].ToInt();
	Scene.Ocean.Grid.WorldSpace.Height = keyMap["GlobalConfig.Scene.Ocean.Grid.WorldSpace.Height"].ToInt();
	Scene.Ocean.Grid.ScreenSpace.GridResolution = keyMap["GlobalConfig.Scene.Ocean.Grid.ScreenSpace.GridResolution"].ToFloat();

	Scene.Ocean.Surface.Projector.Position = keyMap["GlobalConfig.Scene.Ocean.Surface.Projector.Position"].ToVec3();
	Scene.Ocean.Surface.Projector.Normal = keyMap["GlobalConfig.Scene.Ocean.Surface.Projector.Normal"].ToVec3();
	Scene.Ocean.Surface.Projector.MaxWaveAmplitude = keyMap["GlobalConfig.Scene.Ocean.Surface.Projector.MaxWaveAmplitude"].ToFloat();
	Scene.Ocean.Surface.Projector.StrengthElevation = keyMap["GlobalConfig.Scene.Ocean.Surface.Projector.StrengthElevation"].ToFloat();
	Scene.Ocean.Surface.Projector.AimPointCorrection = keyMap["GlobalConfig.Scene.Ocean.Surface.Projector.AimPointCorrection"].ToFloat();
	Scene.Ocean.Surface.Projector.UseGridCorners = keyMap["GlobalConfig.Scene.Ocean.Surface.Projector.UseGridCorners"].ToBool();

	Scene.Ocean.Surface.WaterColor = keyMap["GlobalConfig.Scene.Ocean.Surface.WaterColor"].ToVec3();
	Scene.Ocean.Surface.WaterRefrColor = keyMap["GlobalConfig.Scene.Ocean.Surface.WaterRefrColor"].ToVec3();
	Scene.Ocean.Surface.ReflectionDistortFactor = keyMap["GlobalConfig.Scene.Ocean.Surface.ReflectionDistortFactor"].ToFloat();
	Scene.Ocean.Surface.RefractionDistortFactor = keyMap["GlobalConfig.Scene.Ocean.Surface.RefractionDistortFactor"].ToFloat();
	Scene.Ocean.Surface.MaxFadeAltitude = keyMap["GlobalConfig.Scene.Ocean.Surface.MaxFadeAltitude"].ToFloat();

	Scene.Ocean.Surface.Foam.Enabled = keyMap["GlobalConfig.Scene.Ocean.Surface.Foam.Enabled"].ToBool();
	Scene.Ocean.Surface.Foam.CoverageFactor = keyMap["GlobalConfig.Scene.Ocean.Surface.Foam.CoverageFactor"].ToFloat();
	Scene.Ocean.Surface.Foam.ScaleFactor = keyMap["GlobalConfig.Scene.Ocean.Surface.Foam.ScaleFactor"].ToFloat();

	Scene.Ocean.Surface.SubSurfaceScattering.Enabled = keyMap["GlobalConfig.Scene.Ocean.Surface.SubSurfaceScattering.Enabled"].ToBool();
	Scene.Ocean.Surface.SubSurfaceScattering.Color = keyMap["GlobalConfig.Scene.Ocean.Surface.SubSurfaceScattering.Color"].ToVec3();
	Scene.Ocean.Surface.SubSurfaceScattering.Scale = keyMap["GlobalConfig.Scene.Ocean.Surface.SubSurfaceScattering.Scale"].ToFloat();
	Scene.Ocean.Surface.SubSurfaceScattering.Power = keyMap["GlobalConfig.Scene.Ocean.Surface.SubSurfaceScattering.Power"].ToFloat();
	Scene.Ocean.Surface.SubSurfaceScattering.WaveHeightScale = keyMap["GlobalConfig.Scene.Ocean.Surface.SubSurfaceScattering.WaveHeightScale"].ToFloat();
	Scene.Ocean.Surface.SubSurfaceScattering.MaxAllowedValue = keyMap["GlobalConfig.Scene.Ocean.Surface.SubSurfaceScattering.MaxAllowedValue"].ToFloat();

	Scene.Ocean.Surface.UnderWater.Color = keyMap["GlobalConfig.Scene.Ocean.Surface.UnderWater.Color"].ToVec3();
	Scene.Ocean.Surface.UnderWater.Fog.Enabled = keyMap["GlobalConfig.Scene.Ocean.Surface.UnderWater.Fog.Enabled"].ToBool();
	Scene.Ocean.Surface.UnderWater.Fog.Color = keyMap["GlobalConfig.Scene.Ocean.Surface.UnderWater.Fog.Color"].ToVec3();
	Scene.Ocean.Surface.UnderWater.Fog.Density = keyMap["GlobalConfig.Scene.Ocean.Surface.UnderWater.Fog.Density"].ToFloat();

	Scene.Ocean.Surface.OceanPatch.FFTSize = keyMap["GlobalConfig.Scene.Ocean.Surface.OceanPatch.FFTSize"].ToInt();
	Scene.Ocean.Surface.OceanPatch.PatchSize = keyMap["GlobalConfig.Scene.Ocean.Surface.OceanPatch.PatchSize"].ToInt();
	Scene.Ocean.Surface.OceanPatch.WaveAmpltitude = keyMap["GlobalConfig.Scene.Ocean.Surface.OceanPatch.WaveAmpltitude"].ToFloat();
	Scene.Ocean.Surface.OceanPatch.WindSpeed = keyMap["GlobalConfig.Scene.Ocean.Surface.OceanPatch.WindSpeed"].ToFloat();
	Scene.Ocean.Surface.OceanPatch.WindSpeedMixLimit = keyMap["GlobalConfig.Scene.Ocean.Surface.OceanPatch.WindSpeedMixLimit"].ToFloat();
	Scene.Ocean.Surface.OceanPatch.WindDirection = keyMap["GlobalConfig.Scene.Ocean.Surface.OceanPatch.WindDirection"].ToVec2();
	Scene.Ocean.Surface.OceanPatch.DispersionFrequencyTimePeriod = keyMap["GlobalConfig.Scene.Ocean.Surface.OceanPatch.DispersionFrequencyTimePeriod"].ToFloat();
	Scene.Ocean.Surface.OceanPatch.ChoppyScale = keyMap["GlobalConfig.Scene.Ocean.Surface.OceanPatch.ChoppyScale"].ToFloat();
	Scene.Ocean.Surface.OceanPatch.TileScale = keyMap["GlobalConfig.Scene.Ocean.Surface.OceanPatch.TileScale"].ToFloat();

	Scene.Ocean.Surface.OceanPatch.Spectrum.Phillips.OpposingWavesFactor = keyMap["GlobalConfig.Scene.Ocean.Surface.OceanPatch.Spectrum.Phillips.OpposingWavesFactor"].ToFloat();
	Scene.Ocean.Surface.OceanPatch.Spectrum.Phillips.VerySmallWavesFactor = keyMap["GlobalConfig.Scene.Ocean.Surface.OceanPatch.Spectrum.Phillips.VerySmallWavesFactor"].ToFloat();

	Scene.Ocean.Surface.OceanPatch.Spectrum.Unified.SeaState = keyMap["GlobalConfig.Scene.Ocean.Surface.OceanPatch.Spectrum.Unified.SeaState"].ToFloat();
	Scene.Ocean.Surface.OceanPatch.Spectrum.Unified.MinimumPhaseSpeed = keyMap["GlobalConfig.Scene.Ocean.Surface.OceanPatch.Spectrum.Unified.MinimumPhaseSpeed"].ToFloat();
	Scene.Ocean.Surface.OceanPatch.Spectrum.Unified.SecondaryGravityCapillaryPeak = keyMap["GlobalConfig.Scene.Ocean.Surface.OceanPatch.Spectrum.Unified.SecondaryGravityCapillaryPeak"].ToFloat();

	Scene.Ocean.Surface.OceanPatch.ComputeFFT.UseFFTSlopes = keyMap["GlobalConfig.Scene.Ocean.Surface.OceanPatch.ComputeFFT.UseFFTSlopes"].ToBool();
	Scene.Ocean.Surface.OceanPatch.ComputeFFT.Use2FBOs = keyMap["GlobalConfig.Scene.Ocean.Surface.OceanPatch.ComputeFFT.Use2FBOs"].ToBool(); //Available only for CFT_GPU_FRAG type
	Scene.Ocean.Surface.OceanPatch.ComputeFFT.Type = keyMap["GlobalConfig.Scene.Ocean.Surface.OceanPatch.ComputeFFT.Type"].ToOceanComputeFFTType();

	Scene.Ocean.Surface.OceanPatch.Spectrum.Type = keyMap["GlobalConfig.Scene.Ocean.Surface.OceanPatch.Spectrum.Type"].ToOceanSpectrumType();

	Scene.Ocean.Surface.OceanPatch.NormalGradientFolding.Type = keyMap["GlobalConfig.Scene.Ocean.Surface.OceanPatch.NormalGradientFolding.Type"].ToOceanNormalGradientFoldingType();

	Scene.Ocean.Surface.PerlinNoise.Octaves = keyMap["GlobalConfig.Scene.Ocean.Surface.PerlinNoise.Octaves"].ToVec3();
	Scene.Ocean.Surface.PerlinNoise.Amplitudes = keyMap["GlobalConfig.Scene.Ocean.Surface.PerlinNoise.Amplitudes"].ToVec3();
	Scene.Ocean.Surface.PerlinNoise.Gradients = keyMap["GlobalConfig.Scene.Ocean.Surface.PerlinNoise.Gradients"].ToVec3();
	Scene.Ocean.Surface.PerlinNoise.Speed = keyMap["GlobalConfig.Scene.Ocean.Surface.PerlinNoise.Speed"].ToFloat();

	Scene.Ocean.Surface.WaveBlending.Begin = keyMap["GlobalConfig.Scene.Ocean.Surface.WaveBlending.Begin"].ToFloat();
	Scene.Ocean.Surface.WaveBlending.End = keyMap["GlobalConfig.Scene.Ocean.Surface.WaveBlending.End"].ToFloat();

	Scene.Ocean.Surface.BoatEffects.Foam.Enabled = keyMap["GlobalConfig.Scene.Ocean.Surface.BoatEffects.Foam.Enabled"].ToBool(); // boat foam depends on kelvin wake data!
	Scene.Ocean.Surface.BoatEffects.Foam.Scale = keyMap["GlobalConfig.Scene.Ocean.Surface.BoatEffects.Foam.Scale"].ToFloat();
	Scene.Ocean.Surface.BoatEffects.KelvinWake.Enabled = keyMap["GlobalConfig.Scene.Ocean.Surface.BoatEffects.KelvinWake.Enabled"].ToBool();
	Scene.Ocean.Surface.BoatEffects.KelvinWake.Scale = keyMap["GlobalConfig.Scene.Ocean.Surface.BoatEffects.KelvinWake.Scale"].ToFloat();
	Scene.Ocean.Surface.BoatEffects.PropellerWash.Enabled = keyMap["GlobalConfig.Scene.Ocean.Surface.BoatEffects.PropellerWash.Enabled"].ToBool();
	Scene.Ocean.Surface.BoatEffects.PropellerWash.DistortFactor = keyMap["GlobalConfig.Scene.Ocean.Surface.BoatEffects.PropellerWash.DistortFactor"].ToFloat();
	Scene.Ocean.Surface.BoatEffects.HideInsideWater = keyMap["GlobalConfig.Scene.Ocean.Surface.BoatEffects.HideInsideWater"].ToBool();
	Scene.Ocean.Surface.BoatEffects.Buoyancy.Enabled = keyMap["GlobalConfig.Scene.Ocean.Surface.BoatEffects.Buoyancy.Enabled"].ToBool();

	Scene.Ocean.UnderWater.Color = keyMap["GlobalConfig.Scene.Ocean.UnderWater.Color"].ToVec3();
	Scene.Ocean.UnderWater.Fog.Enabled = keyMap["GlobalConfig.Scene.Ocean.UnderWater.Fog.Enabled"].ToBool();
	Scene.Ocean.UnderWater.Fog.Color = keyMap["GlobalConfig.Scene.Ocean.UnderWater.Fog.Color"].ToVec3();
	Scene.Ocean.UnderWater.Fog.Density = keyMap["GlobalConfig.Scene.Ocean.UnderWater.Fog.Density"].ToFloat();
	Scene.Ocean.UnderWater.GodRays.Enabled = keyMap["GlobalConfig.Scene.Ocean.UnderWater.GodRays.Enabled"].ToBool();
	Scene.Ocean.UnderWater.GodRays.NumberOfSamples = keyMap["GlobalConfig.Scene.Ocean.UnderWater.GodRays.NumberOfSamples"].ToInt();
	Scene.Ocean.UnderWater.GodRays.Exposure = keyMap["GlobalConfig.Scene.Ocean.UnderWater.GodRays.Exposure"].ToFloat();
	Scene.Ocean.UnderWater.GodRays.Decay = keyMap["GlobalConfig.Scene.Ocean.UnderWater.GodRays.Decay"].ToFloat();
	Scene.Ocean.UnderWater.GodRays.Density = keyMap["GlobalConfig.Scene.Ocean.UnderWater.GodRays.Density"].ToFloat();
	Scene.Ocean.UnderWater.GodRays.Weight = keyMap["GlobalConfig.Scene.Ocean.UnderWater.GodRays.Weight"].ToFloat();

	Scene.Ocean.UnderWater.GodRays.Occluder.Size = keyMap["GlobalConfig.Scene.Ocean.UnderWater.GodRays.Occluder.Size"].ToInt();
	Scene.Ocean.UnderWater.GodRays.Occluder.Step = keyMap["GlobalConfig.Scene.Ocean.UnderWater.GodRays.Occluder.Step"].ToInt();

	Scene.Ocean.Bottom.PatchSize = keyMap["GlobalConfig.Scene.Ocean.Bottom.PatchSize"].ToFloat();

	Scene.Ocean.Bottom.Projector.Position = keyMap["GlobalConfig.Scene.Ocean.Bottom.Projector.Position"].ToVec3();
	Scene.Ocean.Bottom.Projector.Normal = keyMap["GlobalConfig.Scene.Ocean.Bottom.Projector.Normal"].ToVec3();
	Scene.Ocean.Bottom.Projector.MaxWaveAmplitude = keyMap["GlobalConfig.Scene.Ocean.Bottom.Projector.MaxWaveAmplitude"].ToFloat();
	Scene.Ocean.Bottom.Projector.StrengthElevation = keyMap["GlobalConfig.Scene.Ocean.Bottom.Projector.StrengthElevation"].ToFloat();
	Scene.Ocean.Bottom.Projector.AimPointCorrection = keyMap["GlobalConfig.Scene.Ocean.Bottom.Projector.AimPointCorrection"].ToFloat();
	// TODO - Fix Bottom.Projector.UseGridCorners
	Scene.Ocean.Bottom.Projector.UseGridCorners = keyMap["GlobalConfig.Scene.Ocean.Bottom.Projector.UseGridCorners"].ToBool(); //TODO - fix

	Scene.Ocean.Bottom.Fog.Enabled = keyMap["GlobalConfig.Scene.Ocean.Bottom.Fog.Enabled"].ToBool();
	Scene.Ocean.Bottom.Fog.Color = keyMap["GlobalConfig.Scene.Ocean.Bottom.Fog.Color"].ToVec3();
	Scene.Ocean.Bottom.Fog.Density = keyMap["GlobalConfig.Scene.Ocean.Bottom.Fog.Density"].ToFloat();

	Scene.Ocean.Bottom.Sand.Scale = keyMap["GlobalConfig.Scene.Ocean.Bottom.Sand.Scale"].ToFloat();

	Scene.Ocean.Bottom.PerlinNoise.Scale = keyMap["GlobalConfig.Scene.Ocean.Bottom.PerlinNoise.Scale"].ToFloat();
	Scene.Ocean.Bottom.PerlinNoise.Amplitude = keyMap["GlobalConfig.Scene.Ocean.Bottom.PerlinNoise.Amplitude"].ToFloat();

	Scene.Ocean.Bottom.Caustics.Enabled = keyMap["GlobalConfig.Scene.Ocean.Bottom.Caustics.Enabled"].ToBool();
	Scene.Ocean.Bottom.Caustics.MapSize = keyMap["GlobalConfig.Scene.Ocean.Bottom.Caustics.MapSize"].ToInt();
	Scene.Ocean.Bottom.Caustics.Color = keyMap["GlobalConfig.Scene.Ocean.Bottom.Caustics.Color"].ToVec3();
	Scene.Ocean.Bottom.Caustics.Intensity = keyMap["GlobalConfig.Scene.Ocean.Bottom.Caustics.Intensity"].ToFloat();
	Scene.Ocean.Bottom.Caustics.PlaneDistanceOffset = keyMap["GlobalConfig.Scene.Ocean.Bottom.Caustics.PlaneDistanceOffset"].ToFloat();
	Scene.Ocean.Bottom.Caustics.Scale = keyMap["GlobalConfig.Scene.Ocean.Bottom.Caustics.Scale"].ToFloat();

	Scene.Boat.Position = keyMap["GlobalConfig.Scene.Boat.Position"].ToVec3();
	Scene.Boat.KelvinWakeOffset = keyMap["GlobalConfig.Scene.Boat.KelvinWakeOffset"].ToFloat();
	Scene.Boat.PropellerWashOffset = keyMap["GlobalConfig.Scene.Boat.PropellerWashOffset"].ToFloat();
	Scene.Boat.PropellerWashWidth = keyMap["GlobalConfig.Scene.Boat.PropellerWashWidth"].ToFloat();
	Scene.Boat.AccelerationFactor = keyMap["GlobalConfig.Scene.Boat.AccelerationFactor"].ToFloat();
	Scene.Boat.TurnAngleFactor = keyMap["GlobalConfig.Scene.Boat.TurnAngleFactor"].ToFloat();
	Scene.Boat.KelvinWakeDisplacementFactor = keyMap["GlobalConfig.Scene.Boat.KelvinWakeDisplacementFactor"].ToFloat();
	Scene.Boat.FoamAmountFactor = keyMap["GlobalConfig.Scene.Boat.FoamAmountFactor"].ToFloat();
	Scene.Boat.UseFlattenedModel = keyMap["GlobalConfig.Scene.Boat.UseFlattenedModel"].ToBool();

	Scene.Boat.Density = keyMap["GlobalConfig.Scene.Boat.Density"].ToFloat();
	Scene.Boat.DragCoefficient = keyMap["GlobalConfig.Scene.Boat.DragCoefficient"].ToFloat();
	Scene.Boat.YAccelerationFactor = keyMap["GlobalConfig.Scene.Boat.YAccelerationFactor"].ToFloat();

	///////////////////////////////////
	// SHADER DEFINES REMAIN IN HERE !!!

	// OpenGL/GLSL mapping across the versions
	std::map<std::string, std::string> opengl2glslVerMap = 
	{{"20", "110"}, {"21", "120"}, {"30", "130"}, {"31", "140"}, {"32", "150"}, {"33", "330"},
	 {"40", "400"}, {"41", "410"}, {"42", "420"}, {"43", "430"}, {"44", "440"}, {"45", "450"},
	 {"46", "460"}};
	

	std::string openglVer = std::to_string(OpenGLContext.OpenGLVersion.major) + std::to_string(OpenGLContext.OpenGLVersion.minor);
	std::string glslVer = opengl2glslVerMap[openglVer];

	std::string openglProfile = OpenGLContext.IsCoreProfile ? " core" : "";
	ShaderDefines.Header = "#version " + glslVer + openglProfile + "\n\n\n";

	LOG("Selected OpenGL version: %s", openglVer.c_str());
	LOG("Selected OpenGL profile: %s", openglProfile.c_str());
	LOG("Selected GLSL shader version: %s", ShaderDefines.Header.c_str());

	// checks
	if (OpenGLContext.OpenGLVersion.major < 3)
	{
		ERR("This app shall not run under OpenGL 3.0 !!!");
		return false;
	}

	if (OpenGLContext.OpenGLVersion.minor < 2)
	{
		ERR("This app shall not run properly under OpenGL 3.2 !!!");
		return false;
	}

	/*if (Scene.Ocean.Grid.Type == CustomTypes::Ocean::GridType::GT_SCREEN_SPACE)
	{
		if (OpenGLContext.OpenGLVersion.major != 4 &&
			OpenGLContext.OpenGLVersion.minor < 1)
		{
			ERR("To support geometry shaders Opengl 4.1 minimum is needed !!!");
			return false;
		}
	}*/

	if (Scene.Ocean.Surface.OceanPatch.ComputeFFT.Type == CustomTypes::Ocean::ComputeFFTType::CFT_GPU_COMP ||
		Scene.Ocean.Surface.OceanPatch.NormalGradientFolding.Type == CustomTypes::Ocean::NormalGradientFoldingType::NGF_GPU_COMP)
	{
		if (OpenGLContext.OpenGLVersion.major != 4 &&
			OpenGLContext.OpenGLVersion.minor < 4 )
			{
				ERR("To support compute shaders Opengl 4.4 minimum is needed ! We need 4.3 for compute shader support, but we also use constant expressions in our compute shader so we can't go below 4.4 !!!");
				return false;
			}
	}

	std::stringstream ss;
	ss << "#define FFT_SIZE " << Scene.Ocean.Surface.OceanPatch.FFTSize << "\n";
	ShaderDefines.Ocean.Surface.FFTSize = ss.str();

	ShaderDefines.HDR = Rendering.HDR.Enabled ? "#define HDR\n" : "#define NO_HDR\n";
	ShaderDefines.Ocean.Surface.GridCorners = Scene.Ocean.Surface.Projector.UseGridCorners ? "#define USE_GRID_CORNERS_SURFACE\n" : "#define NO_USE_GRID_CORNERS_SURFACE\n";
	ShaderDefines.Ocean.Surface.Foam = Scene.Ocean.Surface.Foam.Enabled ? "#define WAVES_FOAM\n" : "#define NO_WAVES_FOAM\n";
	ShaderDefines.Ocean.Surface.SSS = Scene.Ocean.Surface.SubSurfaceScattering.Enabled ? "#define WAVES_SSS\n" : "#define NO_WAVES_SSS\n";
	ShaderDefines.Ocean.Surface.UnderWaterFog = Scene.Ocean.Surface.UnderWater.Fog.Enabled ? "#define UNDERWATER_FOG_SURFACE\n" : "#define NO_UNDERWATER_FOG_SURFACE\n";
	ShaderDefines.Ocean.Surface.BoatEffects.Foam = Scene.Ocean.Surface.BoatEffects.Foam.Enabled ? "#define BOAT_FOAM\n" : "#define NO_BOAT_FOAM\n";
	ShaderDefines.Ocean.Surface.BoatEffects.KelvinWake = Scene.Ocean.Surface.BoatEffects.KelvinWake.Enabled ? "#define BOAT_KELVIN_WAKE\n" : "#define NO_BOAT_KELVIN_WAKE\n";
	ShaderDefines.Ocean.Surface.BoatEffects.PropellerWash = Scene.Ocean.Surface.BoatEffects.PropellerWash.Enabled ? "#define BOAT_PROPELLER_WASH\n" : "#define NO_BOAT_PROPELLER_WASH\n";
	ShaderDefines.Ocean.UnderWater.Fog = Scene.Ocean.UnderWater.Fog.Enabled ? "#define UNDERWATER_FOG\n" : "#define NO_UNDERWATER_FOG\n";
	ShaderDefines.Ocean.UnderWater.GodRays = Scene.Ocean.UnderWater.GodRays.Enabled ? "#define UNDERWATER_GODRAYS\n" : "#define NO_UNDERWATER_GODRAYS\n";
	ShaderDefines.Ocean.Bottom.GridCorners = Scene.Ocean.Bottom.Projector.UseGridCorners ? "#define USE_GRID_CORNERS_BOTTOM\n" : "#define NO_USE_GRID_CORNERS_BOTTOM\n";
	ShaderDefines.Ocean.Bottom.UnderWaterFog = Scene.Ocean.Bottom.Fog.Enabled ? "#define UNDERWATER_FOG_BOTTOM\n" : "#define NO_UNDERWATER_FOG_BOTTOM\n";
	ShaderDefines.Ocean.Bottom.Caustics = Scene.Ocean.Bottom.Caustics.Enabled ? "#define UNDERWATER_CAUSTICS\n" : "#define NO_UNDERWATER_CAUSTICS\n";

	return true;
}
