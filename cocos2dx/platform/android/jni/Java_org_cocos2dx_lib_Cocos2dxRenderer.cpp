#include "text_input_node/CCIMEDispatcher.h"
#include "CCDirector.h"
#include "../CCApplication.h"
#include "../CCEGLView.h"
#include "platform/CCFileUtils.h"
#include "CCEventType.h"
#include "support/CCNotificationCenter.h"
#include "JniHelper.h"
#include "Java_org_cocos2dx_lib_Cocos2dxEngineDataManager.h"
#include <jni.h>

using namespace cocos2d;

extern "C" {
    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeRender(JNIEnv* env) {
        //CCLOG("[Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeRender]");
        if (cocos2d::CCDirector::sharedDirector() && CCDirector::sharedDirector()->getOpenGLView())
        {
            cocos2d::CCDirector::sharedDirector()->mainLoop();
        }
    }

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeOnPause() {
        //CCLOG("[Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeOnPause]");
        if (!CCDirector::sharedDirector() || !CCDirector::sharedDirector()->getOpenGLView())
        {
            return;
        }

        EngineDataManager::onEnterBackground();
        if (CCApplication::sharedApplication())
        {
            CCApplication::sharedApplication()->applicationDidEnterBackground();
        }

        if (CCNotificationCenter::sharedNotificationCenter())
        {
            CCNotificationCenter::sharedNotificationCenter()->postNotification(EVENT_COME_TO_BACKGROUND, NULL);
        }
    }

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeOnResume() {
        if (CCDirector::sharedDirector()->getOpenGLView()) {
            EngineDataManager::onEnterForeground();
            CCApplication::sharedApplication()->applicationWillEnterForeground();
        }
    }

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeInsertText(JNIEnv* env, jobject thiz, jstring text) {
        const char* pszText = env->GetStringUTFChars(text, NULL);
        if (pszText)
        {
            cocos2d::CCIMEDispatcher::sharedDispatcher()->dispatchInsertText(pszText, strlen(pszText));
            env->ReleaseStringUTFChars(text, pszText);
        }
    }

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeDeleteBackward(JNIEnv* env, jobject thiz) {
        cocos2d::CCIMEDispatcher::sharedDispatcher()->dispatchDeleteBackward();
    }

    JNIEXPORT jstring JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeGetContentText() {
        JNIEnv * env = 0;

        if (JniHelper::getJavaVM()->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK || ! env) {
            return 0;
        }
        const char * pszText = cocos2d::CCIMEDispatcher::sharedDispatcher()->getContentText();
        return env->NewStringUTF(pszText);
    }

    //canny
    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeOnScreenSizeChange(JNIEnv * env, jobject thiz, jint width, jint height) {
        if (cocos2d::CCDirector::sharedDirector() && CCDirector::sharedDirector()->getOpenGLView())
        {
            CCLOG("Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeOnScreenSizeChange w,h = (%d,%d)",width, width);
            cocos2d::CCDirector::sharedDirector()->getOpenGLView()->updateFrameSize(width, height);
        }
    }
}
