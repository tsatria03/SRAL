#define SRAL_EXPORT
#include "../Include/SRAL.h"
#include "Engine.h"
#ifdef _WIN32
#include "NVDA.h"
#include "SAPI.h"
#include "Jaws.h"
#include "UIA.h"
#else
#include "SpeechDispatcher.h"
#endif
#include <vector>
#include <string>
Engine* g_currentEngine = nullptr;
std::vector<Engine*> g_engines;
int g_excludes = 0;
extern "C" SRAL_API bool SRAL_Initialize(const char* library_path, int engines_exclude) {
#ifdef _WIN32
	g_engines.push_back(new NVDA);
	g_engines.push_back(new SAPI);
	g_engines.push_back(new Jaws);
	g_engines.push_back(new UIA);
#else
	g_engines.push_back(new SpeechDispatcher);
#endif
	bool found = false;
	for (uint64_t i = 0; i < g_engines.size(); ++i) {
		g_engines[i]->Initialize();
		if (g_engines[i]->GetActive() && !found && !(engines_exclude & g_engines[i]->GetNumber())) {
			g_currentEngine = g_engines[i];
			found = true;
		}
	}
	if (g_currentEngine == nullptr)return false;
	g_excludes = engines_exclude;
	return found;
}
extern "C" SRAL_API void SRAL_Uninitialize(void) {
	g_currentEngine->Uninitialize();
	delete g_currentEngine;
	g_currentEngine = nullptr;
	g_engines.clear();
	g_excludes = 0;
}
static void speech_engine_update() {
	if (!g_currentEngine->GetActive() || g_currentEngine->GetNumber() == ENGINE_SAPI) {
		for (uint64_t i = 0; i < g_engines.size(); ++i) {
			if (g_engines[i]->GetActive() && !(g_excludes & g_engines[i]->GetNumber())) {
				g_currentEngine = g_engines[i];
				break;
			}
		}
	}
}
extern "C" SRAL_API bool SRAL_Speak(const char* text, bool interrupt) {
	if (g_currentEngine == nullptr)		return false;
	speech_engine_update();
	return g_currentEngine->Speak(text, interrupt);
}
extern "C" SRAL_API bool SRAL_Braille(const char* text) {
	if (g_currentEngine == nullptr)return false;
	speech_engine_update();
	return g_currentEngine->Braille(text);
}
extern "C" SRAL_API bool SRAL_Output(const char* text, bool interrupt) {
	if (g_currentEngine == nullptr)return false;
	speech_engine_update();
	const bool speech = SRAL_Speak(text, interrupt);
	const bool braille = SRAL_Braille(text);
	return speech || braille;
}
extern "C" SRAL_API bool SRAL_StopSpeech(void) {
	if (g_currentEngine == nullptr)return false;
	speech_engine_update();
	return g_currentEngine->StopSpeech();
}
extern "C" SRAL_API int SRAL_GetCurrentScreenReader(void) {
	if (g_currentEngine == nullptr)return ENGINE_NONE;
	return g_currentEngine->GetNumber();
}
extern "C" SRAL_API int SRAL_GetEngineFeatures(void) {
	if (g_currentEngine == nullptr)return 0;
	return g_currentEngine->GetFeatures();
}
extern "C" SRAL_API bool SRAL_SetVolume(uint64_t value) {
	if (g_currentEngine == nullptr)return false;
	g_currentEngine->SetVolume(value);
	return true;
}
extern "C" SRAL_API uint64_t SRAL_GetVolume(void) {
	if (g_currentEngine == nullptr)return false;
	return g_currentEngine->GetVolume();
}
extern "C" SRAL_API bool SRAL_SetRate(uint64_t value) {
	if (g_currentEngine == nullptr)return false;
	g_currentEngine->SetRate(value);
	return true;
}
extern "C" SRAL_API uint64_t SRAL_GetRate(void) {
	if (g_currentEngine == nullptr)return false;
	return g_currentEngine->GetRate();
}
