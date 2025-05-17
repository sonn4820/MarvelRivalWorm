#pragma once
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/JobSystem.hpp"
#include "Engine/Core/XmlUtils.hpp"

enum class GroundSound
{
	AIR = -1,
	GRASS,
	SNOW,
	SAND,
	ICE,
	STONE,
	NUM
};
struct Sound
{
	Sound() = default;
	~Sound() = default;
	Sound(std::string name, SoundID id);
	std::string m_name = "";
	SoundID m_ID;
	SoundPlaybackID m_playbackID;
};

struct AudioDefinitions
{
	std::string m_name = "";
	std::string m_path = "";

	AudioDefinitions(XmlElement& element);
	static void InitializeDefs(char const* filePath);
	static void ClearDefinition();
	static AudioDefinitions* GetByName(std::string name);
	static std::vector<AudioDefinitions*> s_audioDefList;
};

class LoadSoundJob : public Job
{
public:
	LoadSoundJob(AudioDefinitions* def, AudioSystem* system);
	void Execute() override;

	AudioDefinitions m_audioDef;
	AudioSystem* m_system = nullptr;
	SoundID m_returnSoundID;
};

class AudioManagerDFS2
{
public:
	AudioManagerDFS2(AudioSystem* system);
	~AudioManagerDFS2();
	void Update(float deltaSeconds);

	bool AreAllSoundsLoaded() const;

	SoundID GetSoundByName(std::string name) const;
	void PlaySFXSound(std::string name, float volume = 1.f, float speed = 1.f, float balance = 0.f, bool loop = false);
	SoundPlaybackID PlayAndGetSFXSound(std::string name, float volume = 1.f, float speed = 1.f, float balance = 0.f, bool loop = false);
	void PlayAndGetSFXSound_2(std::string name, SoundPlaybackID& playbackID, float volume = 1.f, float speed = 1.f, float balance = 0.f, bool loop = false);
	void PlayMusic(std::string name, SoundPlaybackID& playbackID, float volume = 1.f, float speed = 1.f, float balance = 0.f, bool loop = true);

	AudioSystem* m_system = nullptr;
	std::vector<Sound*> m_sounds;
	std::vector<LoadSoundJob*> m_loadingSoundJobs;
	std::atomic<bool> m_isShuttingDown;

	SoundPlaybackID m_latestSound;

	JobWorker* m_threadWorker = nullptr;

	float m_sfxVolume = 1.f;
	float m_musicVolume = 1.f;
};
