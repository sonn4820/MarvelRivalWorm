#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/JobSystem.hpp"
#include "Game/App.hpp"
#include "Game/ParticleSystemDFS2.hpp"
#include "Game/Emitter.hpp"
#include "Game/SquareParticle.hpp"
#include "Game/AudioManagerDFS2.hpp"
#include "Engine/UI/UISystem.hpp"
#include "Engine/UI/Canvas.hpp"
#include "Engine/UI/Button.hpp"
#include "Engine/UI/Text.hpp"
#include "Engine/UI/Panel.hpp"
#include "Engine/UI/Sprite.hpp"
#include "Engine/UI/Slider.hpp"
#include "Engine/UI/Checkbox.hpp"

#include <math.h>

extern App* g_theApp;
extern Renderer* g_theRenderer;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern RandomNumberGenerator* g_theRNG;
extern BitmapFont* g_font;
extern Window* g_theWindow;
extern JobSystem* g_theJobSystem;
extern UISystem* g_theUI;
extern bool g_debugDraw;
extern bool g_debugDestructible;
extern bool g_debugCharacterMoveFast;

constexpr float WORLD_SIZE_X = 200.f;
constexpr float WORLD_SIZE_Y = 100.f;
constexpr float MAX_SHAKE = 10.f;
constexpr float SHAKE_REDUCTION_PER_SEC = 7.f;

constexpr int LOADING_THREAD_ID = 1;
constexpr int LOADING_ASSET_JOB_BITFLAG = 2;

