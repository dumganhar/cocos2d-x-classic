/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2011      Zynga Inc.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

// standard includes
#include <string>

// cocos2d includes
#include "base_nodes/CCBatchNodeMgr.h"
#include "CCDirector.h"
#include "ccFPSImages.h"
#include "draw_nodes/CCDrawingPrimitives.h"
#include "CCConfiguration.h"
#include "cocoa/CCNS.h"
#include "layers_scenes_transitions_nodes/CCScene.h"
#include "cocoa/CCArray.h"
#include "CCScheduler.h"
#include "ccMacros.h"
#include "touch_dispatcher/CCTouchDispatcher.h"
#include "support/CCPointExtension.h"
#include "support/CCNotificationCenter.h"
#include "layers_scenes_transitions_nodes/CCTransition.h"
#include "textures/CCTextureCache.h"
#include "sprite_nodes/CCSpriteFrameCache.h"
#include "cocoa/CCAutoreleasePool.h"
#include "platform/platform.h"
#include "platform/CCFileUtils.h"
#include "CCApplication.h"
#include "label_nodes/CCLabelBMFont.h"
#include "label_nodes/CCLabelAtlas.h"
#include "actions/CCActionManager.h"
#include "CCConfiguration.h"
#include "keypad_dispatcher/CCKeypadDispatcher.h"
#include "CCAccelerometer.h"
#include "sprite_nodes/CCAnimationCache.h"
#include "touch_dispatcher/CCTouch.h"
#include "support/user_default/CCUserDefault.h"
#include "shaders/ccGLStateCache.h"
#include "shaders/CCShaderCache.h"
#include "kazmath/kazmath.h"
#include "kazmath/GL/matrix.h"
#include "support/CCProfiling.h"
#include "platform/CCImage.h"
#include "CCEGLView.h"
#include "CCConfiguration.h"
#include <pthread.h>

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include "platform/android/jni/Java_org_cocos2dx_lib_Cocos2dxEngineDataManager.h"
#endif

/**
 Position of the FPS
 
 Default: 0,0 (bottom-left corner)
 */
#ifndef CC_DIRECTOR_STATS_POSITION
#define CC_DIRECTOR_STATS_POSITION CCDirector::sharedDirector()->getVisibleOrigin()
#endif

using namespace std;

unsigned int g_uNumberOfDraws = 0;
unsigned int g_uNumberOfVertex = 0;
pthread_t g_mainThread;

NS_CC_BEGIN
// XXX it should be a Director ivar. Move it there once support for multiple directors is added

// singleton stuff
static CCDisplayLinkDirector *s_SharedDirector = NULL;

#define kDefaultFPS        60  // 60 frames per second
extern const char* cocos2dVersion(void);

CCDirector* CCDirector::sharedDirector(void)
{
    if (!s_SharedDirector)
    {
        s_SharedDirector = new CCDisplayLinkDirector();
        s_SharedDirector->init();
    }

    return s_SharedDirector;
}

CCDirector::CCDirector(void)
{
    m_pBrowserEventListener = 0;
	m_bEnableThreadMutual = false;

	m_bAutoBatch = false;

	m_bPrintCurFrameCostTime = false;
	m_bOpenTest = false;

	m_9SpriteNoBatchNode = false;
}

bool CCDirector::init(void)
{
    
	setDefaultValues();

    m_hookBeforeSetNextScene = NULL;
    m_hookAfterDraw = NULL;
    m_fDeltaTime = 0.0f;

    // scenes
    m_pRunningScene = NULL;
    m_pNextScene = NULL;

    m_pNotificationNode = NULL;

    m_pobScenesStack = new CCArray();
    m_pobScenesStack->init();

    // projection delegate if "Custom" projection is used
    m_pProjectionDelegate = NULL;

    // FPS
    m_fAccumDt = 0.0f;
    m_fFrameRate = 0.0f;
    m_pFPSLabel = NULL;
    m_pSPFLabel = NULL;
    m_pDrawsLabel = NULL;
	m_pDelayLabel = NULL;
	m_pUdpServerDelayLabel = NULL;
	m_pBulletNumLabel = NULL;
	m_pEnemyNumLabel = NULL;
	m_nNetTcpDelay = 0;
	m_nNetUdpDelay = 0;
	m_nUdpServerLostRate = 0;
	m_nUdpServerAvgDelay = 0;
	m_nUdpServerRealTimeDelay = 0;
	m_nMainBulletNum = 0;
	m_nEnemyBulletNum = 0;
	m_nEnemyNum = 0;

    m_uTotalFrames = m_uFrames = 0;
    m_pszFPS = new char[100];
    m_pLastUpdate = new struct cc_timeval();

    // paused ?
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	m_bPaused = false;
#else
	m_bPaused = true;
#endif
   
    // purge ?
    m_bPurgeDirecotorInNextLoop = false;

    m_obWinSizeInPoints = CCSizeZero;    

    m_pobOpenGLView = NULL;

    m_fContentScaleFactor = 1.0f;

    // scheduler
    m_pScheduler = new CCScheduler();
    // action manager
    m_pActionManager = new CCActionManager();
    m_pScheduler->scheduleUpdateForTarget(m_pActionManager, kCCPrioritySystem, false);

	// scheduler
	m_pSceneSlowScheduler = new CCScheduler();
	// action manager
	m_pSceneSlowActionManager = new CCActionManager();
	m_pSceneSlowScheduler->scheduleUpdateForTarget(m_pSceneSlowActionManager, kCCPrioritySystem, false);

    // touchDispatcher
    m_pTouchDispatcher = new CCTouchDispatcher();
    m_pTouchDispatcher->init();

    // KeypadDispatcher
    m_pKeypadDispatcher = new CCKeypadDispatcher();

    // Accelerometer
    m_pAccelerometer = new CCAccelerometer();
	m_fSecondsPerFrame = 0.0f;
    // create autorelease pool
    CCPoolManager::sharedPoolManager()->push();

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    EngineDataManager::init();
#endif

    return true;
}

#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
void CCDirector::addDrawTextureIDToVec(GLuint textureID)
{
	if(m_bTextureDrawInfo)
	{
		m_listDrawTexture.push_back(textureID);

		std::map<GLuint, std::string>::iterator iter = m_mapTextureFile.find(textureID);
		if(iter != m_mapTextureFile.end())
		{
			CCLog("xxxxxxxxxxx TextureInfo addDrawTextureIDToVec = %s", iter->second.c_str());
		}
	}
}

void CCDirector::putTextureIDFileMap(std::string &fileName, GLuint textureID)
{ 
	m_mapTextureFile[textureID] = fileName; 
}

void CCDirector::removeTextureIDFromMap(GLuint textureID)
{
	std::map<GLuint, std::string>::iterator iter = m_mapTextureFile.find(textureID);
	if(iter != m_mapTextureFile.end())
	{
		m_mapTextureFile.erase(iter);
	}
}
#endif
    
CCDirector::~CCDirector(void)
{
    CCLog("cocos2d: deallocing CCDirector %p", this);

    CC_SAFE_RELEASE(m_pFPSLabel);
    CC_SAFE_RELEASE(m_pSPFLabel);
    CC_SAFE_RELEASE(m_pDrawsLabel);
	CC_SAFE_RELEASE(m_pDelayLabel);
	CC_SAFE_RELEASE(m_pUdpServerDelayLabel);
	CC_SAFE_RELEASE(m_pBulletNumLabel);
	CC_SAFE_RELEASE(m_pEnemyNumLabel);
    
    CC_SAFE_RELEASE(m_pRunningScene);
    CC_SAFE_RELEASE(m_pNotificationNode);
    CC_SAFE_RELEASE(m_pobScenesStack);
    CC_SAFE_RELEASE(m_pScheduler);
	CC_SAFE_RELEASE(m_pActionManager);
	CC_SAFE_RELEASE(m_pSceneSlowScheduler);
    CC_SAFE_RELEASE(m_pSceneSlowActionManager);
    CC_SAFE_RELEASE(m_pTouchDispatcher);
    CC_SAFE_RELEASE(m_pKeypadDispatcher);
    CC_SAFE_DELETE(m_pAccelerometer);

    // pop the autorelease pool
    CCPoolManager::sharedPoolManager()->pop();
    CCPoolManager::purgePoolManager();

    // delete m_pLastUpdate
    CC_SAFE_DELETE(m_pLastUpdate);
    // delete fps string
    CC_SAFE_DELETE_ARRAY(m_pszFPS);

    s_SharedDirector = NULL;
}

void CCDirector::setDefaultValues(void)
{
	CCConfiguration *conf = CCConfiguration::sharedConfiguration();

	// default FPS
	double fps = conf->getNumber("cocos2d.x.fps", kDefaultFPS);
	m_dOldAnimationInterval = m_dAnimationInterval = 1.0 / fps;

	// Display FPS
	m_bDisplayStats = conf->getBool("cocos2d.x.display_fps", false);

	// GL projection
	const char *projection = conf->getCString("cocos2d.x.gl.projection", "2d");
	if( strcmp(projection, "3d") == 0 )
		m_eProjection = kCCDirectorProjection3D;
	else if (strcmp(projection, "2d") == 0)
		m_eProjection = kCCDirectorProjection2D;
	else if (strcmp(projection, "custom") == 0)
		m_eProjection = kCCDirectorProjectionCustom;
	else
		CCAssert(false, "Invalid projection value");

	// Default pixel format for PNG images with alpha
	const char *pixel_format = conf->getCString("cocos2d.x.texture.pixel_format_for_png", "rgba8888");
	if( strcmp(pixel_format, "rgba8888") == 0 )
		CCTexture2D::setDefaultAlphaPixelFormat(kCCTexture2DPixelFormat_RGBA8888);
	else if( strcmp(pixel_format, "rgba4444") == 0 )
		CCTexture2D::setDefaultAlphaPixelFormat(kCCTexture2DPixelFormat_RGBA4444);
	else if( strcmp(pixel_format, "rgba5551") == 0 )
		CCTexture2D::setDefaultAlphaPixelFormat(kCCTexture2DPixelFormat_RGB5A1);

	// PVR v2 has alpha premultiplied ?
	bool pvr_alpha_premultipled = conf->getBool("cocos2d.x.texture.pvrv2_has_alpha_premultiplied", false);
	CCTexture2D::PVRImagesHavePremultipliedAlpha(pvr_alpha_premultipled);

	// Always use premultiplied for PVR v2
	CCTexture2D::PVRImagesHavePremultipliedAlpha(true);
}

void CCDirector::setGLDefaultValues(void)
{
    // This method SHOULD be called only after openGLView_ was initialized
    CCAssert(m_pobOpenGLView, "opengl view should not be null");

    setAlphaBlending(true);
    // XXX: Fix me, should enable/disable depth test according the depth format as cocos2d-iphone did
    // [self setDepthTest: view_.depthFormat];
    setDepthTest(false);
    setProjection(m_eProjection);

    // set other opengl default values
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

// Draw the Scene
void CCDirector::drawScene(void)
{	
	CC_PROFILER_HELPER;
	// calculate "global" dt
	calculateDeltaTime();
	drawSceneImpl();	
}

void CCDirector::drawSceneForAddFrame( float delta )
{
    // calculate "global" dt
    //calculateDeltaTime();
	m_fDeltaTime = delta;

	drawSceneImpl();
}

void CCDirector::drawSceneImpl(void)
{
	CC_PROFILER_HELPER;
	if (m_bEnableThreadMutual)
	{
		if (!isInMainThread())
		{
			return;
		}
	}	

	////////////////////////////////////////////////////////////////
	// add by camel
	//CCProfiler::sharedProfiler()->beginTimingBlock("CCDirector::drawSceneImpl beforeDrawScene");
	for(unsigned int i = 0; i < m_vecDrawSceneListener.size(); i++)
	{
		CCDrawSceneListener* pListener = m_vecDrawSceneListener[i];
		if(pListener)
		{
			pListener->beforeDrawScene(m_fDeltaTime);
		}
	}
	//CCProfiler::sharedProfiler()->endTimingBlock("CCDirector::drawSceneImpl beforeDrawScene");
	////////////////////////////////////////////////////////////////

	CCProfiler::sharedProfiler()->beginTimingBlock("SchedulerUpdate");
    //tick before glClear: issue #533
    if (! m_bPaused)
    {
        m_pScheduler->update(m_fDeltaTime);
		m_pSceneSlowScheduler->update(m_fDeltaTime);
    }

	CCProfiler::sharedProfiler()->endTimingBlock("SchedulerUpdate");

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* to avoid flickr, nextScene MUST be here: after tick and before draw.
     XXX: Which bug is this one. It seems that it can't be reproduced with v0.9 */
    if (m_pNextScene)
    {
        setNextScene();
    }

    kmGLPushMatrix();

	CCProfiler::sharedProfiler()->beginTimingBlock("SceneVisit");
    // draw the scene
    if (m_pRunningScene)
    {
        //clear draw stats
        g_uNumberOfDraws = g_uNumberOfVertex = 0;
        m_pRunningScene->visit();
    }
	CCProfiler::sharedProfiler()->endTimingBlock("SceneVisit");

    // draw the notifications node
    if (m_pNotificationNode)
    {
        m_pNotificationNode->visit();
    }

	flushDraw();
    
    updateFrameRate();
//    if (m_bDisplayStats)
//    {
        showStats();
//    }

    kmGLPopMatrix();

    m_uTotalFrames++;

    if (m_hookAfterDraw != NULL)
    {
        m_hookAfterDraw();
    }
    
	CCProfiler::sharedProfiler()->beginTimingBlock("SwapBuffers");
    // swap buffers
    if (m_pobOpenGLView)
    {
        m_pobOpenGLView->swapBuffers();
    }
    CCProfiler::sharedProfiler()->endTimingBlock("SwapBuffers");
//    if (m_bDisplayStats)
//    {
        calculateMPF();
//    }

	////////////////////////////////////////////////////////////////
	// add by camel
	//CCProfiler::sharedProfiler()->beginTimingBlock("CCDirector::drawSceneImpl afterDrawScene");
	for(unsigned int i = 0; i < m_vecDrawSceneListener.size(); i++)
	{
		CCDrawSceneListener* pListener = m_vecDrawSceneListener[i];
		if(pListener)
		{
			pListener->afterDrawScene(m_fDeltaTime);
		}
	}
	//CCProfiler::sharedProfiler()->endTimingBlock("CCDirector::drawSceneImpl afterDrawScene");
	////////////////////////////////////////////////////////////////
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	if(m_bTextureDrawInfo)
	{
		CCLog("xxxxxxxxxxx start xxxxxxxxxxx");
		CCLog("xxxxxxxxxxxTextureInfo size = %d", m_listDrawTexture.size());

		list<GLuint>::iterator iter = m_listDrawTexture.begin();
		for (; iter != m_listDrawTexture.end(); iter++)
		{
			GLuint textureID = *iter;
			std::map<GLuint, std::string>::iterator iter = m_mapTextureFile.find(textureID);
			if(iter != m_mapTextureFile.end())
			{
				CCLog("xxxxxxxxxxx TextureInfo seq = %s", iter->second.c_str());
			}
		}
		CCLog("xxxxxxxxxxx end xxxxxxxxxxx");
		m_listDrawTexture.clear();
	}
	
#endif
}

void CCDirector::calculateDeltaTime(void)
{
    struct cc_timeval now;

    if (CCTime::gettimeofdayCocos2d(&now, NULL) != 0)
    {
        CCLOG("error in gettimeofday");
        m_fDeltaTime = 0;
        return;
    }

    // new delta time. Re-fixed issue #1277
    if (m_bNextDeltaTimeZero)
    {
        m_fDeltaTime = 0;
        m_bNextDeltaTimeZero = false;
    }
    else
    {
        m_fDeltaTime = (now.tv_sec - m_pLastUpdate->tv_sec) + (now.tv_usec - m_pLastUpdate->tv_usec) / 1000000.0f;
        m_fDeltaTime = MAX(0, m_fDeltaTime);
    }

#ifdef DEBUG
    // If we are debugging our code, prevent big delta time
    if(m_fDeltaTime > 0.2f)
    {
        m_fDeltaTime = 1 / 60.0f;
    }
#endif

    *m_pLastUpdate = now;
}
float CCDirector::getDeltaTime()
{
	return m_fDeltaTime;
}
void CCDirector::setOpenGLView(CCEGLView *pobOpenGLView)
{
    CCAssert(pobOpenGLView, "opengl view should not be null");
	CCLog("CCDirector::setOpenGLView");
    if (m_pobOpenGLView != pobOpenGLView)
    {
		// Configuration. Gather GPU info
		CCConfiguration *conf = CCConfiguration::sharedConfiguration();
		conf->gatherGPUInfo();
		conf->dumpInfo();

        // EAGLView is not a CCObject
        delete m_pobOpenGLView; // [openGLView_ release]
        m_pobOpenGLView = pobOpenGLView;

        // set size
        m_obWinSizeInPoints = m_pobOpenGLView->getDesignResolutionSize();
        
        createStatsLabel();
        
        if (m_pobOpenGLView)
        {
            setGLDefaultValues();
        }  
        
        CHECK_GL_ERROR_DEBUG();

        m_pobOpenGLView->setTouchDelegate(m_pTouchDispatcher);
        m_pTouchDispatcher->setDispatchEvents(true);
    }
}

void CCDirector::setViewport()
{
	CCAssert(metis::CCBatchNodeMgr::GetInstance()->Empty(), "AutoBatch must be empty beforn change viewport!");

    if (m_pobOpenGLView)
    {
        m_pobOpenGLView->setViewPortInPoints(0, 0, m_obWinSizeInPoints.width, m_obWinSizeInPoints.height);
    }
}

void CCDirector::setNextDeltaTimeZero(bool bNextDeltaTimeZero)
{
    m_bNextDeltaTimeZero = bNextDeltaTimeZero;
}

void CCDirector::setProjection(ccDirectorProjection kProjection)
{
	CCAssert(metis::CCBatchNodeMgr::GetInstance()->Empty(), "AutoBatch must be empty beforn change Projection!");

    CCSize size = m_obWinSizeInPoints;

    setViewport();

    switch (kProjection)
    {
    case kCCDirectorProjection2D:
        {
            kmGLMatrixMode(KM_GL_PROJECTION);
            kmGLLoadIdentity();
            kmMat4 orthoMatrix;
            kmMat4OrthographicProjection(&orthoMatrix, 0, size.width, 0, size.height, -1024, 1024 );
            kmGLMultMatrix(&orthoMatrix);
            kmGLMatrixMode(KM_GL_MODELVIEW);
            kmGLLoadIdentity();
        }
        break;

    case kCCDirectorProjection3D:
        {
            float zeye = this->getZEye();

            kmMat4 matrixPerspective, matrixLookup;

            kmGLMatrixMode(KM_GL_PROJECTION);
            kmGLLoadIdentity();

            // issue #1334
            kmMat4PerspectiveProjection( &matrixPerspective, 60, (GLfloat)size.width/size.height, 0.1f, zeye*2);
            // kmMat4PerspectiveProjection( &matrixPerspective, 60, (GLfloat)size.width/size.height, 0.1f, 1500);

            kmGLMultMatrix(&matrixPerspective);

            kmGLMatrixMode(KM_GL_MODELVIEW);
            kmGLLoadIdentity();
            kmVec3 eye, center, up;
            kmVec3Fill( &eye, size.width/2, size.height/2, zeye );
            kmVec3Fill( &center, size.width/2, size.height/2, 0.0f );
            kmVec3Fill( &up, 0.0f, 1.0f, 0.0f);
            kmMat4LookAt(&matrixLookup, &eye, &center, &up);
            kmGLMultMatrix(&matrixLookup);
        }
        break;
            
    case kCCDirectorProjectionCustom:
        if (m_pProjectionDelegate)
        {
            m_pProjectionDelegate->updateProjection();
        }
        break;
            
    default:
        CCLOG("cocos2d: Director: unrecognized projection");
        break;
    }

    m_eProjection = kProjection;
    ccSetProjectionMatrixDirty();
}

void CCDirector::purgeCachedData(void)
{
    CCLabelBMFont::purgeCachedData();
    if (s_SharedDirector->getOpenGLView())
    {
        CCTextureCache::sharedTextureCache()->removeUnusedTextures();
    }
    CCFileUtils::sharedFileUtils()->purgeCachedEntries();
}

float CCDirector::getZEye(void)
{
    return (m_obWinSizeInPoints.height / 1.1566f);
}

void CCDirector::setAlphaBlending(bool bOn)
{
    if (bOn)
    {
        ccGLBlendFunc(CC_BLEND_SRC, CC_BLEND_DST);
    }
    else
    {
        ccGLBlendFunc(GL_ONE, GL_ZERO);
    }

    CHECK_GL_ERROR_DEBUG();
}

void CCDirector::reshapeProjection(const CCSize& newWindowSize)
{
	CC_UNUSED_PARAM(newWindowSize);
	if (m_pobOpenGLView)
	{
		m_obWinSizeInPoints = CCSizeMake(newWindowSize.width * m_fContentScaleFactor,
			newWindowSize.height * m_fContentScaleFactor);
		setProjection(m_eProjection);       
	}

}

void CCDirector::setDepthTest(bool bOn)
{
    if (bOn)
    {
        glClearDepth(1.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
//        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }
    CHECK_GL_ERROR_DEBUG();
}

static void
GLToClipTransform(kmMat4 *transformOut)
{
	kmMat4 projection;
	kmGLGetMatrix(KM_GL_PROJECTION, &projection);
	
	kmMat4 modelview;
	kmGLGetMatrix(KM_GL_MODELVIEW, &modelview);
	
	kmMat4Multiply(transformOut, &projection, &modelview);
}

CCPoint CCDirector::convertToGL(const CCPoint& uiPoint)
{
    kmMat4 transform;
	GLToClipTransform(&transform);
	
	kmMat4 transformInv;
	kmMat4Inverse(&transformInv, &transform);
	
	// Calculate z=0 using -> transform*[0, 0, 0, 1]/w
	kmScalar zClip = transform.mat[14]/transform.mat[15];
	
    CCSize glSize = m_pobOpenGLView->getDesignResolutionSize();
	kmVec3 clipCoord = {2.0f*uiPoint.x/glSize.width - 1.0f, 1.0f - 2.0f*uiPoint.y/glSize.height, zClip};
	
	kmVec3 glCoord;
	kmVec3TransformCoord(&glCoord, &clipCoord, &transformInv);
	
	return ccp(glCoord.x, glCoord.y);
}

CCPoint CCDirector::convertToUI(const CCPoint& glPoint)
{
    kmMat4 transform;
	GLToClipTransform(&transform);
    
	kmVec3 clipCoord;
	// Need to calculate the zero depth from the transform.
	kmVec3 glCoord = {glPoint.x, glPoint.y, 0.0};
	kmVec3TransformCoord(&clipCoord, &glCoord, &transform);
	
	CCSize glSize = m_pobOpenGLView->getDesignResolutionSize();
	return ccp(glSize.width*(clipCoord.x*0.5 + 0.5), glSize.height*(-clipCoord.y*0.5 + 0.5));
}

CCSize CCDirector::getWinSize(void)
{
    return m_obWinSizeInPoints;
}

CCSize CCDirector::getWinSizeInPixels()
{
    return CCSizeMake(m_obWinSizeInPoints.width * m_fContentScaleFactor, m_obWinSizeInPoints.height * m_fContentScaleFactor);
}

CCSize CCDirector::getVisibleSize()
{
    if (m_pobOpenGLView)
    {
        return m_pobOpenGLView->getVisibleSize();
    }
    else 
    {
        return CCSizeZero;
    }
}

CCPoint CCDirector::getVisibleOrigin()
{
    if (m_pobOpenGLView)
    {
        return m_pobOpenGLView->getVisibleOrigin();
    }
    else 
    {
        return CCPointZero;
    }
}

// scene management

void CCDirector::runWithScene(CCScene *pScene)
{
    CCAssert(pScene != NULL, "This command can only be used to start the CCDirector. There is already a scene present.");
    CCAssert(m_pRunningScene == NULL, "m_pRunningScene should be null");

    pushScene(pScene);
    startAnimation();
}

void CCDirector::replaceScene(CCScene *pScene)
{
    CCAssert(m_pRunningScene, "Use runWithScene: instead to start the director");
    CCAssert(pScene != NULL, "the scene should not be null");

    unsigned int index = m_pobScenesStack->count();

    m_bSendCleanupToScene = true;
    m_pobScenesStack->replaceObjectAtIndex(index - 1, pScene);

    m_pNextScene = pScene;
}

void CCDirector::pushScene(CCScene *pScene)
{
    CCAssert(pScene, "the scene should not null");

    m_bSendCleanupToScene = false;

    m_pobScenesStack->addObject(pScene);
    m_pNextScene = pScene;
}

void CCDirector::popScene(void)
{
    CCAssert(m_pRunningScene != NULL, "running scene should not null");

    m_pobScenesStack->removeLastObject();
    unsigned int c = m_pobScenesStack->count();

    if (c == 0)
    {
        end();
    }
    else
    {
        m_bSendCleanupToScene = true;
        m_pNextScene = (CCScene*)m_pobScenesStack->objectAtIndex(c - 1);
    }
}

void CCDirector::popToRootScene(void)
{
    popToSceneStackLevel(1);
}

void CCDirector::popToSceneStackLevel(int level)
{
    CCAssert(m_pRunningScene != NULL, "A running Scene is needed");
    int c = (int)m_pobScenesStack->count();

    // level 0? -> end
    if (level == 0)
    {
        end();
        return;
    }

    // current level or lower -> nothing
    if (level >= c)
        return;

	// pop stack until reaching desired level
	while (c > level)
    {
		CCScene *current = (CCScene*)m_pobScenesStack->lastObject();

		if (current->isRunning())
        {
            current->onExitTransitionDidStart();
            current->onExit();
		}

        current->cleanup();
        m_pobScenesStack->removeLastObject();
		c--;
	}

	m_pNextScene = (CCScene*)m_pobScenesStack->lastObject();
	m_bSendCleanupToScene = false;
}

void CCDirector::end()
{
    m_bPurgeDirecotorInNextLoop = true;
}

void CCDirector::purgeDirector()
{
	CCLog("CCDirector::purgeDirector");
    // cleanup scheduler
    getScheduler()->unscheduleAll();
    
    // don't release the event handlers
    // They are needed in case the director is run again
    m_pTouchDispatcher->removeAllDelegates();

    if (m_pRunningScene)
    {
        m_pRunningScene->onExitTransitionDidStart();
        m_pRunningScene->onExit();
        m_pRunningScene->cleanup();
        m_pRunningScene->release();
    }
    
    m_pRunningScene = NULL;
    m_pNextScene = NULL;

    // remove all objects, but don't release it.
    // runWithScene might be executed after 'end'.
    m_pobScenesStack->removeAllObjects();

	stopRender();
    stopAnimation();

    CC_SAFE_RELEASE_NULL(m_pFPSLabel);
    CC_SAFE_RELEASE_NULL(m_pSPFLabel);
    CC_SAFE_RELEASE_NULL(m_pDrawsLabel);
	CC_SAFE_RELEASE_NULL(m_pDelayLabel);
	CC_SAFE_RELEASE_NULL(m_pUdpServerDelayLabel);

    // purge bitmap cache
    CCLabelBMFont::purgeCachedData();

    // purge all managed caches
    ccDrawFree();
    CCAnimationCache::purgeSharedAnimationCache();
    CCSpriteFrameCache::purgeSharedSpriteFrameCache();
    CCTextureCache::purgeSharedTextureCache();
    CCShaderCache::purgeSharedShaderCache();
    CCFileUtils::purgeFileUtils();
    CCConfiguration::purgeConfiguration();

    // cocos2d-x specific data structures
    CCUserDefault::purgeSharedUserDefault();
    CCNotificationCenter::purgeNotificationCenter();

    ccGLInvalidateStateCache();
    
    CHECK_GL_ERROR_DEBUG();
    
    // OpenGL view
    m_pobOpenGLView->end();
    m_pobOpenGLView = NULL;

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    EngineDataManager::destroy();
#endif

    // delete CCDirector
    release();
}

void CCDirector::setNextScene(void)
{
    if (m_hookBeforeSetNextScene != NULL)
    {
        m_hookBeforeSetNextScene();
    }

    bool runningIsTransition = dynamic_cast<CCTransitionScene*>(m_pRunningScene) != NULL;
    bool newIsTransition = dynamic_cast<CCTransitionScene*>(m_pNextScene) != NULL;

    // If it is not a transition, call onExit/cleanup
     if (! newIsTransition)
     {
         if (m_pRunningScene)
         {
             m_pRunningScene->onExitTransitionDidStart();
             m_pRunningScene->onExit();
         }
 
         // issue #709. the root node (scene) should receive the cleanup message too
         // otherwise it might be leaked.
         if (m_bSendCleanupToScene && m_pRunningScene)
         {
             m_pRunningScene->cleanup();
         }
     }

    if (m_pRunningScene)
    {
        m_pRunningScene->release();
    }
    m_pRunningScene = m_pNextScene;
    m_pNextScene->retain();
    m_pNextScene = NULL;

    if ((! runningIsTransition) && m_pRunningScene)
    {
        m_pRunningScene->onEnter();
        m_pRunningScene->onEnterTransitionDidFinish();
    }
}

void CCDirector::pause(void)
{
    if (m_bPaused)
    {
        return;
    }

    m_dOldAnimationInterval = m_dAnimationInterval;

    // when paused, don't consume CPU
    setAnimationInterval(1 / 4.0);
    m_bPaused = true;
}

void CCDirector::resume(void)
{
    if (! m_bPaused)
    {
        return;
    }

    setAnimationInterval(m_dOldAnimationInterval);

    if (CCTime::gettimeofdayCocos2d(m_pLastUpdate, NULL) != 0)
    {
        CCLOG("cocos2d: Director: Error in gettimeofday");
    }

    m_bPaused = false;
    m_fDeltaTime = 0;
}

void CCDirector::updateFrameRate()
{
    static const float FPS_FILTER = 0.1f;
    static float prevDeltaTime = 0.016f;

    float dt = m_fDeltaTime * FPS_FILTER + (1.0f-FPS_FILTER) * prevDeltaTime;
    prevDeltaTime = dt;
    m_fFrameRate = 1.0f/dt;
}

// display the FPS using a LabelAtlas
// updates the FPS every frame
void CCDirector::showStats(void)
{
    m_uFrames++;
    m_fAccumDt += m_fDeltaTime;
    
    if (m_bDisplayStats)
    {
        if (m_pFPSLabel && m_pSPFLabel && m_pDrawsLabel)
        {
            if (m_fAccumDt > CC_DIRECTOR_STATS_INTERVAL)
            {
                sprintf(m_pszFPS, "%.3f", m_fSecondsPerFrame);
                m_pSPFLabel->setString(m_pszFPS);
                
                m_uFrames = 0;
                m_fAccumDt = 0;
                
                sprintf(m_pszFPS, "%.1f", m_fFrameRate);
                m_pFPSLabel->setString(m_pszFPS);
                
                sprintf(m_pszFPS, "%4lu", (unsigned long)g_uNumberOfDraws);
                m_pDrawsLabel->setString(m_pszFPS);
            }
            
            m_pDrawsLabel->visit();
            m_pFPSLabel->visit();
            m_pSPFLabel->visit();
        }

        if (m_pDelayLabel && m_pUdpServerDelayLabel && m_pBulletNumLabel && m_pEnemyNumLabel)
        {
			int nNetTcpDelay = m_nNetTcpDelay;
			if (nNetTcpDelay > 100000)
			{
				nNetTcpDelay = 99999;
			}
			
			int nNetUdpDelay = m_nNetUdpDelay;
			if (nNetUdpDelay > 100000)
			{
				nNetUdpDelay = 99999;
			}

			sprintf(m_pszFPS, "%d %d", nNetTcpDelay, nNetUdpDelay);
			m_pDelayLabel->setString(m_pszFPS);

			sprintf(m_pszFPS, "%d %d %d", m_nUdpServerLostRate, m_nUdpServerAvgDelay, m_nUdpServerRealTimeDelay);
			m_pUdpServerDelayLabel->setString(m_pszFPS);

			sprintf(m_pszFPS, "%d %d", m_nMainBulletNum, m_nEnemyBulletNum);
			m_pBulletNumLabel->setString(m_pszFPS);

			sprintf(m_pszFPS, "%d", m_nEnemyNum);
			m_pEnemyNumLabel->setString(m_pszFPS);
			
			m_pBulletNumLabel->visit();
			m_pEnemyNumLabel->visit();
            m_pUdpServerDelayLabel->visit();
			m_pDelayLabel->visit();
        }
    }    
}

float CCDirector::getFPS()
{
    return m_fFrameRate;
}

void CCDirector::calculateMPF()
{
    struct cc_timeval now;
    CCTime::gettimeofdayCocos2d(&now, NULL);
    
    m_fSecondsPerFrame = (now.tv_sec - m_pLastUpdate->tv_sec) + (now.tv_usec - m_pLastUpdate->tv_usec) / 1000000.0f;
}

// returns the FPS image data pointer and len
void CCDirector::getFPSImageData(unsigned char** datapointer, unsigned int* length)
{
    // XXX fixed me if it should be used 
    *datapointer = cc_fps_images_png;
	*length = cc_fps_images_len();
}

void CCDirector::createStatsLabel()
{
    CCTexture2D *texture = NULL;
    CCTextureCache *textureCache = CCTextureCache::sharedTextureCache();

    if( m_pFPSLabel && m_pSPFLabel )
    {
        CC_SAFE_RELEASE_NULL(m_pFPSLabel);
        CC_SAFE_RELEASE_NULL(m_pSPFLabel);
        CC_SAFE_RELEASE_NULL(m_pDrawsLabel);
		CC_SAFE_RELEASE_NULL(m_pDelayLabel);
		CC_SAFE_RELEASE_NULL(m_pUdpServerDelayLabel);
		CC_SAFE_RELEASE_NULL(m_pBulletNumLabel);
		CC_SAFE_RELEASE_NULL(m_pEnemyNumLabel);
        textureCache->removeTextureForKey("cc_fps_images");
        CCFileUtils::sharedFileUtils()->purgeCachedEntries();
    }

    CCTexture2DPixelFormat currentFormat = CCTexture2D::defaultAlphaPixelFormat();
    CCTexture2D::setDefaultAlphaPixelFormat(kCCTexture2DPixelFormat_RGBA4444);
    unsigned char *data = NULL;
    unsigned int data_len = 0;
    getFPSImageData(&data, &data_len);

    CCImage* image = new CCImage();
    bool isOK = image->initWithImageData(data, data_len);
    if (!isOK) {
        CCLOGERROR("%s", "Fails: init fps_images");
        return;
    }

    texture = textureCache->addUIImage(image, "cc_fps_images");
    CC_SAFE_RELEASE(image);

    /*
     We want to use an image which is stored in the file named ccFPSImage.c 
     for any design resolutions and all resource resolutions. 
     
     To achieve this,
     
     Firstly, we need to ignore 'contentScaleFactor' in 'CCAtlasNode' and 'CCLabelAtlas'.
     So I added a new method called 'setIgnoreContentScaleFactor' for 'CCAtlasNode',
     this is not exposed to game developers, it's only used for displaying FPS now.
     
     Secondly, the size of this image is 480*320, to display the FPS label with correct size, 
     a factor of design resolution ratio of 480x320 is also needed.
     */
    float factor = CCEGLView::sharedOpenGLView()->getDesignResolutionSize().height / 320.0f;

    m_pFPSLabel = new CCLabelAtlas();
    m_pFPSLabel->setIgnoreContentScaleFactor(true);
    m_pFPSLabel->initWithString("00.0", texture, 12, 32 , '.');
    m_pFPSLabel->setScale(factor);

    m_pSPFLabel = new CCLabelAtlas();
    m_pSPFLabel->setIgnoreContentScaleFactor(true);
    m_pSPFLabel->initWithString("0.000", texture, 12, 32, '.');
    m_pSPFLabel->setScale(factor);

    m_pDrawsLabel = new CCLabelAtlas();
    m_pDrawsLabel->setIgnoreContentScaleFactor(true);
    m_pDrawsLabel->initWithString("000", texture, 12, 32, '.');
    m_pDrawsLabel->setScale(factor);

	m_pDelayLabel = new CCLabelAtlas();
	m_pDelayLabel->setIgnoreContentScaleFactor(true);
	m_pDelayLabel->initWithString("000", texture, 12, 32, '.');
	m_pDelayLabel->setScale(factor);

	m_pUdpServerDelayLabel = new CCLabelAtlas();
	m_pUdpServerDelayLabel->setIgnoreContentScaleFactor(true);
	m_pUdpServerDelayLabel->initWithString("000", texture, 12, 32, '.');
	m_pUdpServerDelayLabel->setScale(factor);

	m_pBulletNumLabel = new CCLabelAtlas();
	m_pBulletNumLabel->setIgnoreContentScaleFactor(true);
	m_pBulletNumLabel->initWithString("000", texture, 12, 32, '.');
	m_pBulletNumLabel->setScale(factor);

	m_pEnemyNumLabel = new CCLabelAtlas();
	m_pEnemyNumLabel->setIgnoreContentScaleFactor(true);
	m_pEnemyNumLabel->initWithString("000", texture, 12, 32, '.');
	m_pEnemyNumLabel->setScale(factor);

    CCTexture2D::setDefaultAlphaPixelFormat(currentFormat);

	m_pEnemyNumLabel->setPosition(ccpAdd(ccp(0, 101*factor), CC_DIRECTOR_STATS_POSITION));
	m_pBulletNumLabel->setPosition(ccpAdd(ccp(0, 85*factor), CC_DIRECTOR_STATS_POSITION));
	m_pUdpServerDelayLabel->setPosition(ccpAdd(ccp(0, 68*factor), CC_DIRECTOR_STATS_POSITION));
	m_pDelayLabel->setPosition(ccpAdd(ccp(0, 51*factor), CC_DIRECTOR_STATS_POSITION));
    m_pDrawsLabel->setPosition(ccpAdd(ccp(0, 34*factor), CC_DIRECTOR_STATS_POSITION));
    m_pSPFLabel->setPosition(ccpAdd(ccp(0, 17*factor), CC_DIRECTOR_STATS_POSITION));
    m_pFPSLabel->setPosition(CC_DIRECTOR_STATS_POSITION);
}

float CCDirector::getContentScaleFactor(void)
{
    return m_fContentScaleFactor;
}

void CCDirector::setContentScaleFactor(float scaleFactor)
{
    if (scaleFactor != m_fContentScaleFactor)
    {
        m_fContentScaleFactor = scaleFactor;
        createStatsLabel();
    }
}

CCNode* CCDirector::getNotificationNode() 
{ 
    return m_pNotificationNode; 
}

void CCDirector::setNotificationNode(CCNode *node)
{
    CC_SAFE_RELEASE(m_pNotificationNode);
    m_pNotificationNode = node;
    CC_SAFE_RETAIN(m_pNotificationNode);
}

CCDirectorDelegate* CCDirector::getDelegate() const
{
    return m_pProjectionDelegate;
}

void CCDirector::setDelegate(CCDirectorDelegate* pDelegate)
{
    m_pProjectionDelegate = pDelegate;
}

void CCDirector::setScheduler(CCScheduler* pScheduler)
{
    if (m_pScheduler != pScheduler)
    {
        CC_SAFE_RETAIN(pScheduler);
        CC_SAFE_RELEASE(m_pScheduler);
        m_pScheduler = pScheduler;
    }
}

CCScheduler* CCDirector::getScheduler()
{
    return m_pScheduler;
}


void CCDirector::setSlowScheduler(CCScheduler* pScheduler)
{
	if (m_pSceneSlowScheduler != pScheduler)
	{
		CC_SAFE_RETAIN(pScheduler);
		CC_SAFE_RELEASE(m_pSceneSlowScheduler);
		m_pSceneSlowScheduler = pScheduler;
	}
}

CCScheduler* CCDirector::getSlowScheduler()
{
	return m_pSceneSlowScheduler;
}

void CCDirector::setActionManager(CCActionManager* pActionManager)
{
    if (m_pActionManager != pActionManager)
    {
        CC_SAFE_RETAIN(pActionManager);
        CC_SAFE_RELEASE(m_pActionManager);
        m_pActionManager = pActionManager;
    }    
}

CCActionManager* CCDirector::getActionManager()
{
    return m_pActionManager;
}


void CCDirector::setSlowActionManager(CCActionManager* pActionManager)
{
	if (m_pSceneSlowActionManager != pActionManager)
	{
		CC_SAFE_RETAIN(pActionManager);
		CC_SAFE_RELEASE(m_pSceneSlowActionManager);
		m_pSceneSlowActionManager = pActionManager;
	}    
}

CCActionManager* CCDirector::getSlowActionManager()
{
	return m_pSceneSlowActionManager;
}

void CCDirector::setTouchDispatcher(CCTouchDispatcher* pTouchDispatcher)
{
    if (m_pTouchDispatcher != pTouchDispatcher)
    {
        CC_SAFE_RETAIN(pTouchDispatcher);
        CC_SAFE_RELEASE(m_pTouchDispatcher);
        m_pTouchDispatcher = pTouchDispatcher;
    }    
}

CCTouchDispatcher* CCDirector::getTouchDispatcher()
{
    return m_pTouchDispatcher;
}

void CCDirector::setKeypadDispatcher(CCKeypadDispatcher* pKeypadDispatcher)
{
    CC_SAFE_RETAIN(pKeypadDispatcher);
    CC_SAFE_RELEASE(m_pKeypadDispatcher);
    m_pKeypadDispatcher = pKeypadDispatcher;
}

CCKeypadDispatcher* CCDirector::getKeypadDispatcher()
{
    return m_pKeypadDispatcher;
}

void CCDirector::setAccelerometer(CCAccelerometer* pAccelerometer)
{
    if (m_pAccelerometer != pAccelerometer)
    {
        CC_SAFE_DELETE(m_pAccelerometer);
        m_pAccelerometer = pAccelerometer;
    }
}

CCAccelerometer* CCDirector::getAccelerometer()
{
    return m_pAccelerometer;
}

//////////////////////////////////////////////////////////////////////
// add by camel

void CCDirector::registerDrawSceneListener(CCDrawSceneListener* pListener)
{
	VECCCDRAWSCENELISTENER::iterator it = m_vecDrawSceneListener.begin();
	for( ; it != m_vecDrawSceneListener.end(); it++)
	{
		if(*it == pListener)
		{
			return;
		}
	}

	m_vecDrawSceneListener.push_back(pListener);
}

void CCDirector::unregisterDrawSceneListener(CCDrawSceneListener* pListener)
{
	VECCCDRAWSCENELISTENER::iterator it = m_vecDrawSceneListener.begin();
	for( ; it != m_vecDrawSceneListener.end(); it++)
	{
		if(*it == pListener)
		{
			m_vecDrawSceneListener.erase(it);
			return;
		}
	}
}
//////////////////////////////////////////////////////////////////////

int CCDirector::fireeEvent_BrowserWillOpen(const char * url)
{
    if(m_pBrowserEventListener)
    {
        return m_pBrowserEventListener->onEvent_willOpen(url);
    }
    
    return 0;
}

int CCDirector::fireBrowserEventMessage(int nMsgID, const char * pMessage)
{
	if(m_pBrowserEventListener)
	{
		return m_pBrowserEventListener->onEvent_BrowserMessage(nMsgID, pMessage);
	}
	return 0;
}

void CCDirector::registerBrowserEventListener(CCBrowserEventListener * listener)
{
    m_pBrowserEventListener = listener;
}

void CCDirector::enableThreadMutual(bool bEnable)
{
	m_bEnableThreadMutual = bEnable;
	g_mainThread = pthread_self();
}

bool CCDirector::isInMainThread()
{
	if (m_bEnableThreadMutual)
	{
		pthread_t thread = pthread_self();
		if (pthread_equal(thread, g_mainThread) == 0)
		{
			return false;
		}		
		else
		{
			return true;
		}
	}
	return true;
}

/***************************************************
* implementation of DisplayLinkDirector
**************************************************/

CCDisplayLinkDirector::CCDisplayLinkDirector(void)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	startRender();
	m_bIsAnimationPlaying = true;
#else
	stopRender();
	m_bIsAnimationPlaying = false;
#endif
}

void CCDisplayLinkDirector::startRender(void)
{
	m_bIsRendering = true;
}

void CCDisplayLinkDirector::stopRender(void)
{
	m_bIsRendering = false;
}

bool CCDisplayLinkDirector::isRendering(void) const
{
	return m_bIsRendering;
}

// should we implement 4 types of director ??
// I think DisplayLinkDirector is enough
// so we now only support DisplayLinkDirector
void CCDisplayLinkDirector::startAnimation(void)
{
    if (CCTime::gettimeofdayCocos2d(m_pLastUpdate, NULL) != 0)
    {
        CCLOG("cocos2d: DisplayLinkDirector: Error on gettimeofday");
    }

	m_bIsAnimationPlaying = true;
#ifndef EMSCRIPTEN
	if (CCApplication::sharedApplication())
	{
		CCApplication::sharedApplication()->setAnimationInterval(m_dAnimationInterval);
	}    
#endif // EMSCRIPTEN
}

void CCDisplayLinkDirector::mainLoop(void)
{
	if (CCProfiler::sharedProfiler()->isEnablePerFrameLog())
	{
		CCProfiler::sharedProfiler()->releaseAllTimers();
	}	
	
	m_bPrintCurFrameCostTime = false;
	long long startTime = 0;

	if(m_bOpenTest)
	{
		struct timeval tv;  
		gettimeofday(&tv,NULL);
		startTime = tv.tv_sec;
		startTime *= 1000;
		startTime += tv.tv_usec / 1000;
	}


	{
		//root callnode Ãû×ÖÖ¸¶¨
		CCProfilerHelper profilerHelper("CCDirector", "mainLoop");

		if (m_bPurgeDirecotorInNextLoop)
		{
			m_bPurgeDirecotorInNextLoop = false;
			purgeDirector();
			return;
		}
		else if (isRendering())
		{
			drawScene();

			// release the objects
			CCProfilerHelper profilerHelper("CCPoolManager", "pop");
			CCPoolManager::sharedPoolManager()->pop();        
		}

		for(unsigned int i = 0; i < m_vecDrawSceneListener.size(); i++)
		{
			CCDrawSceneListener* pListener = m_vecDrawSceneListener[i];
			if(pListener)
			{
				pListener->onMainLoopEnd();
			}
		}
	}

	CCProfiler::sharedProfiler()->mergeCallTree();
	if (CCProfiler::sharedProfiler()->isEnablePerFrameLog())
	{
		/*gettimeofday(&tv, NULL);
		long long endTime = tv.tv_sec;
		endTime *= 1000;
		endTime += tv.tv_usec / 1000;

		if(endTime - startTime < 50)
		{
			return;
		}*/
		CCProfiler::sharedProfiler()->displayTimers();	
	}

	if(m_bOpenTest && m_bPrintCurFrameCostTime)
	{
		struct timeval tv;  
		gettimeofday(&tv,NULL);
		long long endTime = tv.tv_sec;
		endTime *= 1000;
		endTime += tv.tv_usec / 1000;
		CCLog("printAuidoTestData***current frame costtime = %lld, frame = %f", endTime - startTime, getFPS());
	}
}

void CCDirector::mainLoopForAddFrame( float delta )
{
	if (m_bPaused)
	{
		return;
	}
	
	drawSceneForAddFrame(delta);

	// release the objects
	CCPoolManager::sharedPoolManager()->pop();        
}

void CCDirector::setNetTcpDelay( int nDelay )
{
	m_nNetTcpDelay = nDelay;
}

void CCDirector::setNetUdpDelay( int nDelay )
{
	m_nNetUdpDelay = nDelay;
}

bool CCDirector::isEnableAutoBatch()
{
	return m_bAutoBatch;
}

void CCDirector::flushDraw()
{
	metis::CCBatchNodeMgr::GetInstance()->FlushDraw();
}

void CCDisplayLinkDirector::stopAnimation(void)
{
	m_bIsAnimationPlaying = false;
}

void CCDisplayLinkDirector::setAnimationInterval(double dValue)
{
    m_dAnimationInterval = dValue;
	if (m_bIsAnimationPlaying)
    {
        stopAnimation();
        startAnimation();
    }    
}

NS_CC_END

