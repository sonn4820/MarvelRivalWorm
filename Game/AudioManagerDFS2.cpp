#include "AudioManagerDFS2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "App.hpp"
#include "GameCommon.hpp"

std::vector<AudioDefinitions*> AudioDefinitions::s_audioDefList;


Sound::Sound(std::string name, SoundID id)
	:m_name(name), m_ID(id)
{

}

LoadSoundJob::LoadSoundJob(AudioDefinitions* def, AudioSystem* system)
	:m_audioDef(*def), m_system(system)
{
	m_Bitflags = LOADING_ASSET_JOB_BITFLAG;
}

void LoadSoundJob::Execute()
{
	m_returnSoundID = m_system->CreateOrGetSound(m_audioDef.m_path, false);
}


AudioDefinitions::AudioDefinitions(XmlElement& element)
	:m_name(ParseXmlAttribute(element, "name", "")),
	m_path(ParseXmlAttribute(element, "path", ""))
{

}

void AudioDefinitions::InitializeDefs(char const* filePath)
{
	XmlDocument file;
	XmlError result = file.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "FILE IS NOT LOADED");

	XmlElement* rootElement = file.RootElement();
	GUARANTEE_OR_DIE(rootElement, "Root Element is null");

	XmlElement* audioDefElement = rootElement->FirstChildElement();

	while (audioDefElement)
	{
		std::string name = audioDefElement->Name();
		GUARANTEE_OR_DIE(name == "AudioDefinition", "Root child element is in the wrong format");

		AudioDefinitions* newAudioDef = new AudioDefinitions(*audioDefElement);

		s_audioDefList.push_back(newAudioDef);
		audioDefElement = audioDefElement->NextSiblingElement();
	}
}

void AudioDefinitions::ClearDefinition()
{
	for (auto& i : s_audioDefList)
	{
		if (i != nullptr)
		{
			delete i;
			i = nullptr;
		}
	}
}

AudioDefinitions* AudioDefinitions::GetByName(std::string name)
{
	for (auto& i : s_audioDefList)
	{
		if (i->m_name == name)
		{
			return i;
		}
	}
	return nullptr;
}


AudioManagerDFS2::AudioManagerDFS2(AudioSystem* system)
	:m_system(system)
{
	for (auto & i : AudioDefinitions::s_audioDefList)
	{
		LoadSoundJob* audioJob = new LoadSoundJob(i, m_system);
		g_theJobSystem->QueueJob(audioJob);
		m_loadingSoundJobs.push_back(audioJob);
	}

	m_threadWorker = g_theJobSystem->GetWorker(LOADING_THREAD_ID);
}

AudioManagerDFS2::~AudioManagerDFS2()
{
	m_isShuttingDown = true;
	//m_threadWorker->Join();
}

void AudioManagerDFS2::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	for (size_t i = 0; i < m_loadingSoundJobs.size(); i++)
	{
		if (m_isShuttingDown) return;

		auto job = g_theJobSystem->RetrieveJob(m_loadingSoundJobs[i]);
		if (job)
		{
			LoadSoundJob* audioJob = dynamic_cast<LoadSoundJob*>(job);
			Sound* newSound = new Sound(audioJob->m_audioDef.m_name, audioJob->m_returnSoundID);
			m_loadingSoundJobs.erase(m_loadingSoundJobs.begin() + i);
			m_sounds.push_back(newSound);
			i--;
		}
	}
}

bool AudioManagerDFS2::AreAllSoundsLoaded() const
{
	return m_loadingSoundJobs.empty();
}

SoundID AudioManagerDFS2::GetSoundByName(std::string name) const
{
	for (auto sound : m_sounds)
	{
		if (sound->m_name == name)
		{
			return sound->m_ID;
		}
	}

	return MISSING_SOUND_ID;
}

void AudioManagerDFS2::PlaySFXSound(std::string name, float volume, float speed, float balance, bool loop)
{
	m_latestSound = m_system->StartSound(GetSoundByName(name), loop, volume * m_sfxVolume, balance, speed);
}

SoundPlaybackID AudioManagerDFS2::PlayAndGetSFXSound(std::string name, float volume /*= 1.f*/, float speed /*= 1.f*/, float balance /*= 0.f*/, bool loop)
{
	m_latestSound = m_system->StartSound(GetSoundByName(name), loop, volume * m_sfxVolume, balance, speed);
	return m_latestSound;
}
void AudioManagerDFS2::PlayAndGetSFXSound_2(std::string name, SoundPlaybackID& playbackID, float volume /*= 1.f*/, float speed /*= 1.f*/, float balance /*= 0.f*/, bool loop)
{
	playbackID = m_system->StartSound(GetSoundByName(name), loop, volume * m_sfxVolume, balance, speed);
	m_latestSound = playbackID;
}

void AudioManagerDFS2::PlayMusic(std::string name, SoundPlaybackID& playbackID, float volume /*= 1.f*/, float speed /*= 1.f*/, float balance /*= 0.f*/, bool loop /*= true*/)
{
	playbackID = m_system->StartSound(GetSoundByName(name), loop, volume * m_musicVolume, balance, speed);
}

