#include "SimpleAudioEngineJni.h"

#include <android/log.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <map>
#include <unistd.h>
#include <sys/types.h>
#include <string>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include "platform/android/CCFileUtilsAndroid.h"
#include "platform/android/jni/JniHelper.h"
#include "CCDirector.h"
#include "CCScheduler.h"

#define DELAY_TIME_TO_REMOVE 0.5f

#define  LOG_TAG    "libSimpleAudioEngine"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  CLASS_NAME "org/cocos2dx/lib/Cocos2dxHelper"

#define  LOGEX(msg) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "fun:%s,line:%d,msg:%s",__func__,__LINE__,#msg)

// engine interfaces
static SLObjectItf s_engineObject = NULL;
static SLEngineItf s_engineEngine = NULL;
// output mix interfaces
static SLObjectItf s_outputMixObject = NULL;

static int s_currentAudioID = 1;
static float s_effectVolume = 1.0f;

static AAssetManager* s_assetManager = NULL;

typedef struct JniMethodInfo_
{
    JNIEnv *    env;
    jclass      classID;
    jmethodID   methodID;
} JniMethodInfo;

class AudioPlayer
{
public:
    AudioPlayer()
        : _fdPlayerObject(NULL)
        , _playOver(false)
        , _loop(false)
        , _assetFd(0)
		, _delayTimeToRemove(-1.f)
    {
    }

    ~AudioPlayer()
    {
		struct timeval tv;  
		gettimeofday(&tv,NULL);
		long long startTime = tv.tv_sec;
		startTime *= 1000;
		startTime += tv.tv_usec / 1000;
		//LOGEX("AudioPlayer::~AudioPlayer");
        if (_fdPlayerObject)
        {
			//LOGEX("AudioPlayer::~AudioPlayer Destroy");
            (*_fdPlayerObject)->Destroy(_fdPlayerObject);
            _fdPlayerObject = NULL;
            _fdPlayerPlay = NULL;
            _fdPlayerVolume = NULL;
            _fdPlayerSeek = NULL;
        }

        if (_assetFd > 0)
        {
            close(_assetFd);
            _assetFd = 0;
        }


		gettimeofday(&tv, NULL);
		long long endTime = tv.tv_sec;
		endTime *= 1000;
		endTime += tv.tv_usec / 1000;

		LOGD("~AudioPlayer _name = %s, costtime = %lld",_name.c_str(), endTime - startTime);
    }

    bool init(SLEngineItf engineEngine, SLObjectItf outputMixObject, const std::string& fileFullPath, float volume, bool loop)
    {
        bool ret = false;

        do
        {
            SLDataSource audioSrc;

            SLDataLocator_AndroidFD loc_fd;
            SLDataLocator_URI loc_uri;

            SLDataFormat_MIME format_mime = { SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED };
            audioSrc.pFormat = &format_mime;

            if (fileFullPath[0] != '/'){
                std::string relativePath = "";

                size_t position = fileFullPath.find("assets/");
                if (0 == position) {
                    // "assets/" is at the beginning of the path and we don't want it
                    relativePath += fileFullPath.substr(strlen("assets/"));
                }
                else {
                    relativePath += fileFullPath;
                }

                AAsset* asset = AAssetManager_open(s_assetManager, relativePath.c_str(), AASSET_MODE_UNKNOWN);

                // open asset as file descriptor
                off_t start, length;
                _assetFd = AAsset_openFileDescriptor(asset, &start, &length);
                if (_assetFd <= 0){
                    AAsset_close(asset);
                    break;
                }
                AAsset_close(asset);

                // configure audio source
                loc_fd.locatorType = SL_DATALOCATOR_ANDROIDFD;
                loc_fd.fd = _assetFd;
                loc_fd.offset = start;
                loc_fd.length = length;

                audioSrc.pLocator = &loc_fd;
            }
            else{
                loc_uri.locatorType = SL_DATALOCATOR_URI;
                loc_uri.URI = (SLchar*)fileFullPath.c_str();
                audioSrc.pLocator = &loc_uri;
            }

            // configure audio sink
            SLDataLocator_OutputMix loc_outmix = { SL_DATALOCATOR_OUTPUTMIX, outputMixObject };
            SLDataSink audioSnk = { &loc_outmix, NULL };

            // create audio player
            const SLInterfaceID ids[3] = { SL_IID_SEEK, SL_IID_PREFETCHSTATUS, SL_IID_VOLUME };
            const SLboolean req[3] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
            SLresult result = (*engineEngine)->CreateAudioPlayer(engineEngine, &_fdPlayerObject, &audioSrc, &audioSnk, 3, ids, req);
            if (SL_RESULT_SUCCESS != result){ LOGEX("create audio player fail"); break; }

            // realize the player
            result = (*_fdPlayerObject)->Realize(_fdPlayerObject, SL_BOOLEAN_FALSE);
            if (SL_RESULT_SUCCESS != result){ LOGEX("realize the player fail"); break; }

            // get the play interface
            result = (*_fdPlayerObject)->GetInterface(_fdPlayerObject, SL_IID_PLAY, &_fdPlayerPlay);
            if (SL_RESULT_SUCCESS != result){ LOGEX("get the play interface fail"); break; }

            // get the seek interface
            result = (*_fdPlayerObject)->GetInterface(_fdPlayerObject, SL_IID_SEEK, &_fdPlayerSeek);
            if (SL_RESULT_SUCCESS != result){ LOGEX("get the seek interface fail"); break; }

            // get the volume interface
            result = (*_fdPlayerObject)->GetInterface(_fdPlayerObject, SL_IID_VOLUME, &_fdPlayerVolume);
            if (SL_RESULT_SUCCESS != result){ LOGEX("get the volume interface fail"); break; }

            _loop = loop;
            if (loop){
                (*_fdPlayerSeek)->SetLoop(_fdPlayerSeek, SL_BOOLEAN_TRUE, 0, SL_TIME_UNKNOWN);
            }

            int dbVolume = 2000 * log10(volume);
            if (dbVolume < SL_MILLIBEL_MIN){
                dbVolume = SL_MILLIBEL_MIN;
            }
            (*_fdPlayerVolume)->SetVolumeLevel(_fdPlayerVolume, dbVolume);

            result = (*_fdPlayerPlay)->SetPlayState(_fdPlayerPlay, SL_PLAYSTATE_PLAYING);
            if (SL_RESULT_SUCCESS != result){ LOGEX("SetPlayState fail"); break; }

            ret = true;
        } while (0);

        return ret;
    }

    SLObjectItf _fdPlayerObject;
    SLSeekItf _fdPlayerSeek;
    SLVolumeItf _fdPlayerVolume;

    bool _playOver;
    bool _loop;
    SLPlayItf _fdPlayerPlay;

    int _audioID;
    int _assetFd;
	float _delayTimeToRemove;
	std::string _name;

    static void EffectPlayOverEvent(SLPlayItf caller, void* context, SLuint32 playEvent)
    {
        if (context && playEvent == SL_PLAYEVENT_HEADATEND)
        {
            AudioPlayer* player = (AudioPlayer*)context;
            //fix issue#8965:AudioEngine can't looping audio on Android 2.3.x
            if (player->_loop)
            {
                (*(player->_fdPlayerPlay))->SetPlayState(player->_fdPlayerPlay, SL_PLAYSTATE_PLAYING);
            }
            else
            {
                player->_playOver = true;
            }
        }
    }
};

//audioID,AudioInfo
static std::map<int, AudioPlayer>  s_audioPlayers;
typedef std::map<int, AudioPlayer>::iterator AudioPlayerIt;

class AudioPlayerController : public cocos2d::CCObject
{
public:
    void update(float dt)
    {
		//LOGEX("AudioPlayerController::update num %d", s_audioPlayers.size());
		int count = 0;
        AudioPlayerIt itend = s_audioPlayers.end();
        for (AudioPlayerIt iter = s_audioPlayers.begin(); iter != itend;)
        {
			if (iter->second._delayTimeToRemove > 0.f)
			{
				iter->second._delayTimeToRemove -= dt;
				if (iter->second._delayTimeToRemove < 0.f)
				{
					LOGD("AudioPlayer stop = %s", iter->second._name.c_str());
					s_audioPlayers.erase(iter++);
					++count;
					//break;
					continue;
				}
			}
			else if(iter->second._playOver)
			{
				LOGD("AudioPlayer playover = %s", iter->second._name.c_str());
				s_audioPlayers.erase(iter++);
				++count;
				//break;
				continue;
			}

			++iter;
        }

		LOGD("AudioPlayerController::update  total num %d, release num %d", s_audioPlayers.size(), count);
    }
};

void stopAllEffectsOpenSLJNI()
{
	struct timeval tv;  
	gettimeofday(&tv,NULL);
	long long startTime = tv.tv_sec;
	startTime *= 1000;
	startTime += tv.tv_usec / 1000;

	AudioPlayerIt itEnd = s_audioPlayers.end();
	for (AudioPlayerIt it = s_audioPlayers.begin(); it != itEnd; ++it)
	{
		SLresult result = (*it->second._fdPlayerPlay)->SetPlayState(it->second._fdPlayerPlay, SL_PLAYSTATE_STOPPED);
		if (SL_RESULT_SUCCESS != result){
			LOGD("%s error:%u", __func__, result);
		}

		if (it->second._delayTimeToRemove < 0.f)
		{
			//If destroy openSL object immediately,it may cause dead lock.
			it->second._delayTimeToRemove = DELAY_TIME_TO_REMOVE;
		}
	}
	//s_audioPlayers.clear();

	//s_currentAudioID = 0;

	gettimeofday(&tv, NULL);
	long long endTime = tv.tv_sec;
	endTime *= 1000;
	endTime += tv.tv_usec / 1000;

	LOGD("AudioPlayer stopAllEffectsOpenSLJNI  costtime = %lld", endTime - startTime);
}

static AudioPlayerController * s_playerController = NULL;

extern "C"
{
    // get env and cache it
    static JNIEnv* getJNIEnv(void)
    {
        
        JavaVM* jvm = cocos2d::JniHelper::getJavaVM();
        if (NULL == jvm) {
            LOGD("%s", "Failed to get JNIEnv. JniHelper::getJavaVM() is NULL");
            return NULL;
        }
        
        JNIEnv *env = NULL;
        // get jni environment
        jint ret = jvm->GetEnv((void**)&env, JNI_VERSION_1_4);
        
        switch (ret) {
            case JNI_OK :
                // Success!
                return env;
                
            case JNI_EDETACHED :
                // Thread not attached
                
                // TODO : If calling AttachCurrentThread() on a native thread
                // must call DetachCurrentThread() in future.
                // see: http://developer.android.com/guide/practices/design/jni.html
                
                if (jvm->AttachCurrentThread(&env, NULL) < 0)
                {
                    LOGD("%s", "Failed to get the environment using AttachCurrentThread()");
                    return NULL;
                } else {
                    // Success : Attached and obtained JNIEnv!
                    return env;
                }
                
            case JNI_EVERSION :
                // Cannot recover from this error
                LOGD("%s", "JNI interface version 1.4 not supported");
            default :
                LOGD("%s", "Failed to get the environment using GetEnv()");
                return NULL;
        }
    }
    
    // get class and make it a global reference, release it at endJni().
    static jclass getClassID(JNIEnv *pEnv)
    {
        jclass ret = pEnv->FindClass(CLASS_NAME);
        if (! ret)
        {
            LOGD("Failed to find class of %s", CLASS_NAME);
        }
        
        return ret;
    }
    
    static bool getStaticMethodInfo(JniMethodInfo &methodinfo, const char *methodName, const char *paramCode)
    {
        jmethodID methodID = 0;
        JNIEnv *pEnv = 0;
        bool bRet = false;
        
        do 
        {
            pEnv = getJNIEnv();
            if (! pEnv)
            {
                break;
            }
            
            jclass classID = getClassID(pEnv);
            
            methodID = pEnv->GetStaticMethodID(classID, methodName, paramCode);
            if (! methodID)
            {
                LOGD("Failed to find static method id of %s", methodName);
                break;
            }
            
            methodinfo.classID = classID;
            methodinfo.env = pEnv;
            methodinfo.methodID = methodID;
            
            bRet = true;
        } while (0);
        
        return bRet;
    }

	static void initOpenSL()
	{
		if (s_engineObject)
		{
			return;
		}

		do{
			JniMethodInfo methodInfo;
			if (! getStaticMethodInfo(methodInfo, "getAssetManager", "()Landroid/content/res/AssetManager;"))
			{
				methodInfo.env->DeleteLocalRef(methodInfo.classID);
				return;
			}
			jobject assetManagerObj = methodInfo.env->CallStaticObjectMethod(methodInfo.classID, methodInfo.methodID);
			methodInfo.env->DeleteLocalRef(methodInfo.classID);
			s_assetManager = AAssetManager_fromJava(methodInfo.env, assetManagerObj);


			// create engine
			SLresult result = slCreateEngine(&s_engineObject, 0, NULL, 0, NULL, NULL);
			if (SL_RESULT_SUCCESS != result){ LOGEX("create opensl engine fail"); break; }

			// realize the engine
			result = (*s_engineObject)->Realize(s_engineObject, SL_BOOLEAN_FALSE);
			if (SL_RESULT_SUCCESS != result){ LOGEX("realize the engine fail"); break; }

			// get the engine interface, which is needed in order to create other objects
			result = (*s_engineObject)->GetInterface(s_engineObject, SL_IID_ENGINE, &s_engineEngine);
			if (SL_RESULT_SUCCESS != result){ LOGEX("get the engine interface fail"); break; }

			// create output mix
			const SLInterfaceID outputMixIIDs[] = {};
			const SLboolean outputMixReqs[] = {};
			result = (*s_engineEngine)->CreateOutputMix(s_engineEngine, &s_outputMixObject, 0, outputMixIIDs, outputMixReqs);
			if (SL_RESULT_SUCCESS != result){ LOGEX("create output mix fail"); break; }

			// realize the output mix
			result = (*s_outputMixObject)->Realize(s_outputMixObject, SL_BOOLEAN_FALSE);
			if (SL_RESULT_SUCCESS != result){ LOGEX("realize the output mix fail"); break; }

			using namespace cocos2d;
			if (s_playerController == NULL)
			{
				s_playerController = new AudioPlayerController;
				cocos2d::CCScheduler* scheduler = cocos2d::CCDirector::sharedDirector()->getScheduler();
				//scheduler->scheduleSelector(schedule_selector(AudioPlayerController::update), s_playerController, 0.03f, false);
				scheduler->scheduleUpdateForTarget(s_playerController, kCCPrioritySystem, false);
			}

		} while (false);
	}
    
    void preloadBackgroundMusicOpenSLJNI(const char *path)
    {
        // void playBackgroundMusic(String,boolean)
        JniMethodInfo methodInfo;
        
        if (! getStaticMethodInfo(methodInfo, "preloadBackgroundMusic", "(Ljava/lang/String;)V"))
        {            
            return;
        }
        
        jstring stringArg = methodInfo.env->NewStringUTF(path);
        methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID, stringArg);
        methodInfo.env->DeleteLocalRef(stringArg);
        methodInfo.env->DeleteLocalRef(methodInfo.classID);
    }
    
    void playBackgroundMusicOpenSLJNI(const char *path, bool isLoop)
    {
        // void playBackgroundMusic(String,boolean)
        
        JniMethodInfo methodInfo;
        
        if (! getStaticMethodInfo(methodInfo, "playBackgroundMusic", "(Ljava/lang/String;Z)V"))
        {
            return;
        }
        
        jstring stringArg = methodInfo.env->NewStringUTF(path);
        methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID, stringArg, isLoop);
        methodInfo.env->DeleteLocalRef(stringArg);
        methodInfo.env->DeleteLocalRef(methodInfo.classID);
    }
    
    void stopBackgroundMusicOpenSLJNI()
    {
        // void stopBackgroundMusic()
        
        JniMethodInfo methodInfo;
        
        if (! getStaticMethodInfo(methodInfo, "stopBackgroundMusic", "()V"))
        {
            return;
        }
        
        methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID);
        methodInfo.env->DeleteLocalRef(methodInfo.classID);
    }
    
    void pauseBackgroundMusicOpenSLJNI()
    {
        // void pauseBackgroundMusic()
        
        JniMethodInfo methodInfo;
        
        if (! getStaticMethodInfo(methodInfo, "pauseBackgroundMusic", "()V"))
        {
            return;
        }
        
        methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID);
        methodInfo.env->DeleteLocalRef(methodInfo.classID);
    }
    
    void resumeBackgroundMusicOpenSLJNI()
    {
        // void resumeBackgroundMusic()
        
        JniMethodInfo methodInfo;
        
        if (! getStaticMethodInfo(methodInfo, "resumeBackgroundMusic", "()V"))
        {
            return;
        }
        
        methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID);
        methodInfo.env->DeleteLocalRef(methodInfo.classID);
    }
    
    void rewindBackgroundMusicOpenSLJNI()
    {
        // void rewindBackgroundMusic()
        
        JniMethodInfo methodInfo;
        
        if (! getStaticMethodInfo(methodInfo, "rewindBackgroundMusic", "()V"))
        {
            return;
        }
        
        methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID);
        methodInfo.env->DeleteLocalRef(methodInfo.classID);
    }
    
    bool isBackgroundMusicPlayingOpenSLJNI()
    {
        // boolean rewindBackgroundMusic()
        
        JniMethodInfo methodInfo;
        jboolean ret = false;
        
        if (! getStaticMethodInfo(methodInfo, "isBackgroundMusicPlaying", "()Z"))
        {
            return ret;
        }
        
        ret = methodInfo.env->CallStaticBooleanMethod(methodInfo.classID, methodInfo.methodID);
        methodInfo.env->DeleteLocalRef(methodInfo.classID);
        
        return ret;
    }
    
    float getBackgroundMusicVolumeOpenSLJNI()
    {
        // float getBackgroundMusicVolume()
        
        JniMethodInfo methodInfo;
        jfloat ret = -1.0;
        
        if (! getStaticMethodInfo(methodInfo, "getBackgroundMusicVolume", "()F"))
        {
            return ret;
        }
        
        ret = methodInfo.env->CallStaticFloatMethod(methodInfo.classID, methodInfo.methodID);
        methodInfo.env->DeleteLocalRef(methodInfo.classID);
        
        return ret;
    }
    
    void setBackgroundMusicVolumeOpenSLJNI(float volume)
    {
        // void setBackgroundMusicVolume()
        
        JniMethodInfo methodInfo;
        
        if (! getStaticMethodInfo(methodInfo, "setBackgroundMusicVolume", "(F)V"))
        {
            return ;
        }
        
        methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID, volume);
        methodInfo.env->DeleteLocalRef(methodInfo.classID);
    }
    
    void endOpenSLJNI()
    {
        // void end()
        
        JniMethodInfo methodInfo;
        
        if (! getStaticMethodInfo(methodInfo, "end", "()V"))
        {
            return ;
        }
        
        methodInfo.env->CallStaticVoidMethod(methodInfo.classID, methodInfo.methodID);
        methodInfo.env->DeleteLocalRef(methodInfo.classID);

        stopAllEffectsOpenSLJNI();

        if (s_outputMixObject)
        {
            (*s_outputMixObject)->Destroy(s_outputMixObject);
            s_outputMixObject = NULL;
        }

        if (s_engineObject)
        {
            (*s_engineObject)->Destroy(s_engineObject);
            s_engineObject = NULL;
            s_engineEngine = NULL;
        }
    }

	unsigned int playEffectOpenSLJNI(const char* filePath, bool loop)
	{
		struct timeval tv;  
		gettimeofday(&tv,NULL);
		long long startTime = tv.tv_sec;
		startTime *= 1000;
		startTime += tv.tv_usec / 1000;

		if (s_engineObject == NULL)
		{
			initOpenSL();
		}

		if (s_engineObject == NULL)
		{
			return 0;
		}

		unsigned int audioId = 0;

		do
		{
			if (s_engineEngine == NULL)
				break;

			AudioPlayer& player = s_audioPlayers[s_currentAudioID];
			std::string fullPath = cocos2d::CCFileUtils::sharedFileUtils()->fullPathForFilename(filePath);

			bool initPlayer = player.init(s_engineEngine, s_outputMixObject, fullPath, s_effectVolume, loop);
			if (!initPlayer)
			{
				s_audioPlayers.erase(s_currentAudioID);
				LOGD("%s,%d message:create player for %s fail", __func__, __LINE__, filePath);
				break;
			}
			audioId = s_currentAudioID++;
			player._audioID = audioId;
			player._name = fullPath;

			(*(player._fdPlayerPlay))->RegisterCallback(player._fdPlayerPlay, AudioPlayer::EffectPlayOverEvent, (void*)&player);
			(*(player._fdPlayerPlay))->SetCallbackEventsMask(player._fdPlayerPlay, SL_PLAYEVENT_HEADATEND);

			gettimeofday(&tv, NULL);
			long long endTime = tv.tv_sec;
			endTime *= 1000;
			endTime += tv.tv_usec / 1000;

			LOGD("AudioPlayer playEffectOpenSLJNI _name = %s, costtime = %lld", fullPath.c_str(), endTime - startTime);
		} while (0);

		
		return audioId;
	}
	float getEffectsVolumeOpenSLJNI()
	{
		return s_effectVolume;
	}

	void setEffectsVolumeOpenSLJNI(float volume)
	{
		if (volume > 1.0f)
		{
			volume = 1.0f;
		}
		else if (volume < 0.0f)
		{
			volume = 0.0f;
		}

		s_effectVolume = volume;

		int dbVolume = 2000 * log10(volume);
		if(dbVolume < SL_MILLIBEL_MIN){
			dbVolume = SL_MILLIBEL_MIN;
		}

		AudioPlayerIt itEnd = s_audioPlayers.end();
		for (AudioPlayerIt it = s_audioPlayers.begin(); it != itEnd; ++it)
		{
			SLresult result = (*it->second._fdPlayerVolume)->SetVolumeLevel(it->second._fdPlayerVolume, dbVolume);
			if (SL_RESULT_SUCCESS != result){
				LOGD("%s error:%u", __func__, result);
			}
		}
	}

	void preloadEffectOpenSLJNI(const char *path)
	{

	}

	void unloadEffectOpenSLJNI(const char* path)
	{

	}

	void pauseEffectOpenSLJNI(unsigned int soundId)
	{
		if (s_audioPlayers.find(soundId) != s_audioPlayers.end())
		{
			AudioPlayer& player = s_audioPlayers[soundId];
			SLresult result = (*player._fdPlayerPlay)->SetPlayState(player._fdPlayerPlay, SL_PLAYSTATE_PAUSED);
			if (SL_RESULT_SUCCESS != result){
				LOGD("%s error:%u", __func__, result);
			}
		}
	}

	void pauseAllEffectsOpenSLJNI()
	{
		AudioPlayerIt itEnd = s_audioPlayers.end();
		for (AudioPlayerIt it = s_audioPlayers.begin(); it != itEnd; ++it)
		{
			SLresult result = (*it->second._fdPlayerPlay)->SetPlayState(it->second._fdPlayerPlay, SL_PLAYSTATE_PAUSED);
			if (SL_RESULT_SUCCESS != result){
				LOGD("%s error:%u", __func__, result);
			}
		}
	}

	void resumeEffectOpenSLJNI(unsigned int soundId)
	{
		if (s_audioPlayers.find(soundId) != s_audioPlayers.end())
		{
			AudioPlayer& player = s_audioPlayers[soundId];
			SLresult result = (*player._fdPlayerPlay)->SetPlayState(player._fdPlayerPlay, SL_PLAYSTATE_PLAYING);
			if (SL_RESULT_SUCCESS != result){
				LOGD("%s error:%u", __func__, result);
			}
		}
	}

	void resumeAllEffectsOpenSLJNI()
	{
		AudioPlayerIt itEnd = s_audioPlayers.end();
		for (AudioPlayerIt it = s_audioPlayers.begin(); it != itEnd; ++it)
		{
			SLresult result = (*it->second._fdPlayerPlay)->SetPlayState(it->second._fdPlayerPlay, SL_PLAYSTATE_PLAYING);
			if (SL_RESULT_SUCCESS != result){
				LOGD("%s error:%u", __func__, result);
			}
		}
	}

	void stopEffectOpenSLJNI(unsigned int soundId)
	{
		struct timeval tv;  
		gettimeofday(&tv,NULL);
		long long startTime = tv.tv_sec;
		startTime *= 1000;
		startTime += tv.tv_usec / 1000;

		if (s_audioPlayers.find(soundId) != s_audioPlayers.end())
		{
			AudioPlayer& player = s_audioPlayers[soundId];
			SLresult result = (*player._fdPlayerPlay)->SetPlayState(player._fdPlayerPlay, SL_PLAYSTATE_STOPPED);
			if (SL_RESULT_SUCCESS != result){
				LOGD("%s error:%u", __func__, result);
			}

			/*If destroy openSL object immediately,it may cause dead lock.
			*It's a system issue.For more information:
			*    https://github.com/cocos2d/cocos2d-x/issues/11697
			*    https://groups.google.com/forum/#!msg/android-ndk/zANdS2n2cQI/AT6q1F3nNGIJ
			*/
			player._delayTimeToRemove = DELAY_TIME_TO_REMOVE;
			//s_audioPlayers.erase(soundId);

			gettimeofday(&tv, NULL);
			long long endTime = tv.tv_sec;
			endTime *= 1000;
			endTime += tv.tv_usec / 1000;

			LOGD("AudioPlayer stopEffectOpenSLJNI _name = %s, costtime = %lld", player._name.c_str(), endTime - startTime);
		}

	}
}





