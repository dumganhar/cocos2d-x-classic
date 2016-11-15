#ifndef __SIMPLE_AUDIO_ENGINE_OPENSL_JNI__
#define __SIMPLE_AUDIO_ENGINE_OPENSL_JNI__

#include <jni.h>

void stopAllEffectsOpenSLJNI();

extern "C"
{
    extern void preloadBackgroundMusicOpenSLJNI(const char *path);
    extern void playBackgroundMusicOpenSLJNI(const char *path, bool isLoop);
    extern void stopBackgroundMusicOpenSLJNI();
    extern void pauseBackgroundMusicOpenSLJNI();
    extern void resumeBackgroundMusicOpenSLJNI();
    extern void rewindBackgroundMusicOpenSLJNI();
    extern bool isBackgroundMusicPlayingOpenSLJNI();
    extern float getBackgroundMusicVolumeOpenSLJNI();
    extern void setBackgroundMusicVolumeOpenSLJNI(float volume);
    extern void endOpenSLJNI();

	extern unsigned int playEffectOpenSLJNI(const char* path, bool bLoop);
	extern void stopEffectOpenSLJNI(unsigned int nSoundId);
	extern float getEffectsVolumeOpenSLJNI();
	extern void setEffectsVolumeOpenSLJNI(float volume);
	extern void preloadEffectOpenSLJNI(const char *path);
	extern void unloadEffectOpenSLJNI(const char* path);
	extern void pauseEffectOpenSLJNI(unsigned int nSoundId);
	extern void pauseAllEffectsOpenSLJNI();
	extern void resumeEffectOpenSLJNI(unsigned int nSoundId);
	extern void resumeAllEffectsOpenSLJNI();
}


#endif // __SIMPLE_AUDIO_ENGINE_OPENSL_OpenSLJNI__
