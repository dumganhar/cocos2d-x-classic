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

#ifndef __CCDIRECTOR_H__
#define __CCDIRECTOR_H__

//////////////////////////
// add by camel
#include <vector>
//////////////////////////

#include "platform/CCPlatformMacros.h"
#include "cocoa/CCObject.h"
#include "ccTypes.h"
#include "cocoa/CCGeometry.h"
#include "cocoa/CCArray.h"
#include "CCGL.h"
#include "kazmath/mat4.h"
#include "label_nodes/CCLabelAtlas.h"
#include "ccTypeInfo.h"

NS_CC_BEGIN

/**
 * @addtogroup base_nodes
 * @{
 */

/** @typedef ccDirectorProjection
 Possible OpenGL projections used by director
 */
typedef enum {
    /// sets a 2D projection (orthogonal projection)
    kCCDirectorProjection2D,
    
    /// sets a 3D projection with a fovy=60, znear=0.5f and zfar=1500.
    kCCDirectorProjection3D,
    
    /// it calls "updateProjection" on the projection delegate.
    kCCDirectorProjectionCustom,
    
    /// Default projection is 3D projection
    kCCDirectorProjectionDefault = kCCDirectorProjection3D,
} ccDirectorProjection;

/* Forward declarations. */
class CCLabelAtlas;
class CCScene;
class CCEGLView;
class CCDirectorDelegate;
class CCNode;
class CCScheduler;
class CCActionManager;
class CCTouchDispatcher;
class CCKeypadDispatcher;
class CCAccelerometer;

///////////////////////////////////////////////////////////////////
// add by camel
class CCDrawSceneListener
{
public:
	virtual void beforeDrawScene(float fDelta) = 0;
	virtual void afterDrawScene(float fDelta) = 0;


	// ������ѭ����֡�ص�����ͨҵ���߼����ô˻ص� [10/17/2014 gusterzhai]
	virtual void onMainLoopEnd() = 0;
};

typedef std::vector<CCDrawSceneListener*> VECCCDRAWSCENELISTENER;
///////////////////////////////////////////////////////////////////

// add by Jamesgu
class CCBrowserEventListener
{
public:
	enum
	{
		CCE_MSG_WEBKITERROR					= 0,				// ��������
		CCE_MSG_STARTOPENURL,									// ׼����URL
		CCE_MSG_RECEIVETITLE,										// �յ�title����
		CCE_MSG_LOADINGURL,										// ��ʼ����URL
		CCE_MSG_OPENURLCOMPLETED,							// ��URL���
		
	};
public:
    virtual ~ CCBrowserEventListener(){}
    
public:
    virtual int onEvent_willOpen(const char * szURL) = 0;
	virtual int onEvent_BrowserMessage(int nMsgID, const char  * pMessage) = 0;
};

/**
@brief Class that creates and handle the main Window and manages how
and when to execute the Scenes.
 
 The CCDirector is also responsible for:
  - initializing the OpenGL context
  - setting the OpenGL pixel format (default on is RGB565)
  - setting the OpenGL buffer depth (default one is 0-bit)
  - setting the projection (default one is 3D)
  - setting the orientation (default one is Portrait)
 
 Since the CCDirector is a singleton, the standard way to use it is by calling:
  _ CCDirector::sharedDirector()->methodName();
 
 The CCDirector also sets the default OpenGL context:
  - GL_TEXTURE_2D is enabled
  - GL_VERTEX_ARRAY is enabled
  - GL_COLOR_ARRAY is enabled
  - GL_TEXTURE_COORD_ARRAY is enabled
*/
class CC_DLL CCDirector : public CCObject, public TypeInfo
{
public:
    CCDirector(void);
    virtual ~CCDirector(void);
    virtual bool init(void);
    virtual long getClassTypeInfo() {
		static const long id = cocos2d::getHashCodeByString(typeid(cocos2d::CCDirector).name());
		return id;
    }

    // attribute

    /** Get current running Scene. Director can only run one Scene at the time */
    inline CCScene* getRunningScene(void) { return m_pRunningScene; }

    /** Get the FPS value */
    inline double getAnimationInterval(void) { return m_dAnimationInterval; }
    /** Set the FPS value. */
    virtual void setAnimationInterval(double dValue) = 0;

    /** Whether or not to display the FPS on the bottom-left corner */
    inline bool isDisplayStats(void) { return m_bDisplayStats; }
    /** Display the FPS on the bottom-left corner */
    inline void setDisplayStats(bool bDisplayStats) { m_bDisplayStats = bDisplayStats; }
    
    /** seconds per frame */
    inline float getSecondsPerFrame() { return m_fSecondsPerFrame; }

    /** Get the CCEGLView, where everything is rendered */
    inline CCEGLView* getOpenGLView(void) { return m_pobOpenGLView; }
    void setOpenGLView(CCEGLView *pobOpenGLView);

    inline bool isNextDeltaTimeZero(void) { return m_bNextDeltaTimeZero; }
    void setNextDeltaTimeZero(bool bNextDeltaTimeZero);

    /** Whether or not the Director is paused */
    inline bool isPaused(void) { return m_bPaused; }

    /** How many frames were called since the director started */
    inline unsigned int getTotalFrames(void) { return m_uTotalFrames; }
    
    /** Sets an OpenGL projection
     @since v0.8.2
     */
    inline ccDirectorProjection getProjection(void) { return m_eProjection; }
    void setProjection(ccDirectorProjection kProjection);

     /** reshape projection matrix when canvas has been change"*/
    void reshapeProjection(const CCSize& newWindowSize);
    
    /** Sets the glViewport*/
    void setViewport();

    /** How many frames were called since the director started */
    
    
    /** Whether or not the replaced scene will receive the cleanup message.
     If the new scene is pushed, then the old scene won't receive the "cleanup" message.
     If the new scene replaces the old one, the it will receive the "cleanup" message.
     @since v0.99.0
     */
    inline bool isSendCleanupToScene(void) { return m_bSendCleanupToScene; }

    /** This object will be visited after the main scene is visited.
     This object MUST implement the "visit" selector.
     Useful to hook a notification object, like CCNotifications (http://github.com/manucorporat/CCNotifications)
     @since v0.99.5
     */
    CCNode* getNotificationNode();
    void setNotificationNode(CCNode *node);
    
    /** CCDirector delegate. It shall implemente the CCDirectorDelegate protocol
     @since v0.99.5
     */
    CCDirectorDelegate* getDelegate() const;
    void setDelegate(CCDirectorDelegate* pDelegate);

    // window size

    /** returns the size of the OpenGL view in points.
    */
    CCSize getWinSize(void);

    /** returns the size of the OpenGL view in pixels.
    */
    CCSize getWinSizeInPixels(void);
    
    /** returns visible size of the OpenGL view in points.
     *  the value is equal to getWinSize if don't invoke
     *  CCEGLView::setDesignResolutionSize()
     */
    CCSize getVisibleSize();
    
    /** returns visible origin of the OpenGL view in points.
     */
    CCPoint getVisibleOrigin();

    /** converts a UIKit coordinate to an OpenGL coordinate
     Useful to convert (multi) touch coordinates to the current layout (portrait or landscape)
     */
    CCPoint convertToGL(const CCPoint& obPoint);

    /** converts an OpenGL coordinate to a UIKit coordinate
     Useful to convert node points to window points for calls such as glScissor
     */
    CCPoint convertToUI(const CCPoint& obPoint);

    /// XXX: missing description 
    float getZEye(void);

    // Scene Management

    /** Enters the Director's main loop with the given Scene.
     * Call it to run only your FIRST scene.
     * Don't call it if there is already a running scene.
     *
     * It will call pushScene: and then it will call startAnimation
     */
    void runWithScene(CCScene *pScene);

    /** Suspends the execution of the running scene, pushing it on the stack of suspended scenes.
     * The new scene will be executed.
     * Try to avoid big stacks of pushed scenes to reduce memory allocation. 
     * ONLY call it if there is a running scene.
     */
    void pushScene(CCScene *pScene);

    /** Pops out a scene from the queue.
     * This scene will replace the running one.
     * The running scene will be deleted. If there are no more scenes in the stack the execution is terminated.
     * ONLY call it if there is a running scene.
     */
    void popScene(void);

    /** Pops out all scenes from the queue until the root scene in the queue.
     * This scene will replace the running one.
     * Internally it will call `popToSceneStackLevel(1)`
     */
    void popToRootScene(void);

    /** Pops out all scenes from the queue until it reaches `level`.
     If level is 0, it will end the director.
     If level is 1, it will pop all scenes until it reaches to root scene.
     If level is <= than the current stack level, it won't do anything.
     */
 	void popToSceneStackLevel(int level);

    /** Replaces the running scene with a new one. The running scene is terminated.
     * ONLY call it if there is a running scene.
     */
    void replaceScene(CCScene *pScene);

    /** Ends the execution, releases the running scene.
     It doesn't remove the OpenGL view from its parent. You have to do it manually.
     */
    void end(void);

    /** Pauses the running scene.
     The running scene will be _drawed_ but all scheduled timers will be paused
     While paused, the draw rate will be 4 FPS to reduce CPU consumption
     */
    void pause(void);

    /** Resumes the paused scene
     The scheduled timers will be activated again.
     The "delta time" will be 0 (as if the game wasn't paused)
     */
    void resume(void);

	virtual void startRender(void) = 0;
	virtual void stopRender(void) = 0;
	virtual bool isRendering(void) const = 0;

    /** Stops the animation. Nothing will be drawn. The main loop won't be triggered anymore.
     If you don't want to pause your animation call [pause] instead.
     */
    virtual void stopAnimation(void) = 0;

    /** The main loop is triggered again.
     Call this function only if [stopAnimation] was called earlier
     @warning Don't call this function to start the main loop. To run the main loop call runWithScene
     */
    virtual void startAnimation(void) = 0;

    /** Draw the scene.
    This method is called every frame. Don't call it manually.
    */
    void drawScene(void);

	// �����ڲ�֡ [10/17/2014 gusterzhai]
	void drawSceneForAddFrame(float delta);

	void drawSceneImpl();

    // Memory Helper

    /** Removes cached all cocos2d cached data.
     It will purge the CCTextureCache, CCSpriteFrameCache, CCLabelBMFont cache
     @since v0.99.3
     */
    void purgeCachedData(void);

	/** sets the default values based on the CCConfiguration info */
    void setDefaultValues(void);

    // OpenGL Helper

    /** sets the OpenGL default values */
    void setGLDefaultValues(void);

    /** enables/disables OpenGL alpha blending */
    void setAlphaBlending(bool bOn);

    /** enables/disables OpenGL depth test */
    void setDepthTest(bool bOn);

    virtual void mainLoop(void) = 0;

	// �����ڲ�֡ [10/17/2014 gusterzhai]
	virtual void mainLoopForAddFrame(float delta);

    /** The size in pixels of the surface. It could be different than the screen size.
    High-res devices might have a higher surface size than the screen size.
    Only available when compiled using SDK >= 4.0.
    @since v0.99.4
    */
    void setContentScaleFactor(float scaleFactor);
    float getContentScaleFactor(void);

	void enableAutoBatch(bool bEnable){m_bAutoBatch = bEnable;}
	bool isEnableAutoBatch();

	void flushDraw();
    
       /**
     *  Gets Frame Rate.
     * @js NA
     */
    float getFrameRate() const { return m_fFrameRate; }

    typedef void (*HookFunc)();
    void setBeforeSetNextSceneHook(HookFunc func) { m_hookBeforeSetNextScene = func; }
    void setAfterDrawHook(HookFunc func) { m_hookAfterDraw = func; }

public:
    /** CCScheduler associated with this director
     @since v2.0
     */
    CC_PROPERTY(CCScheduler*, m_pScheduler, Scheduler);

    /** CCActionManager associated with this director
     @since v2.0
     */
    CC_PROPERTY(CCActionManager*, m_pActionManager, ActionManager);

    /** CCTouchDispatcher associated with this director
     @since v2.0
     */
    CC_PROPERTY(CCTouchDispatcher*, m_pTouchDispatcher, TouchDispatcher);

    /** CCKeypadDispatcher associated with this director
     @since v2.0
     */
    CC_PROPERTY(CCKeypadDispatcher*, m_pKeypadDispatcher, KeypadDispatcher);

    /** CCAccelerometer associated with this director
     @since v2.0
     */
    CC_PROPERTY(CCAccelerometer*, m_pAccelerometer, Accelerometer);

    /* delta time since last tick to main loop */
	CC_PROPERTY_READONLY(float, m_fDeltaTime, DeltaTime);


	/** CCScheduler associated with this director
     @since v2.0
     */
    CC_PROPERTY(CCScheduler*, m_pSceneSlowScheduler, SlowScheduler);

    /** CCActionManager associated with this director
     @since v2.0
     */
    CC_PROPERTY(CCActionManager*, m_pSceneSlowActionManager, SlowActionManager);
//////////////////////////////////////////////////////////////////////
	bool m_bPrintCurFrameCostTime;
	bool m_bOpenTest;
// add by camel
public:
	void registerDrawSceneListener(CCDrawSceneListener* pListener);
	void unregisterDrawSceneListener(CCDrawSceneListener* pListener);
//////////////////////////////////////////////////////////////////////
    
    int  fireeEvent_BrowserWillOpen(const char * url);
	int fireBrowserEventMessage(int nMsgID, const char * pMessage);
    void registerBrowserEventListener(CCBrowserEventListener * listener);

//////////////////////////////////////////////////////////////////////////
	// �˽ӿڱ�����cocos2dx��ѭ�����ڵ��̵߳��� [8/20/2014 gusterzhai]
	void enableThreadMutual(bool bEnable);
	bool isInMainThread();
public:
    /** returns a shared instance of the director */
    static CCDirector* sharedDirector(void);
    
    float getFPS();

	void setNetTcpDelay(int nDelay);
	void setNetUdpDelay(int nDelay);

	void setUdpServerLostRate(int value){m_nUdpServerLostRate = value;}
	void setUdpServerAvgDelay(int value){m_nUdpServerAvgDelay = value;}
	void setUdpServerRealTimeDelay(int value){m_nUdpServerRealTimeDelay = value;}
	void setEnemyBulletNum(int value){m_nEnemyBulletNum = value;}
	void setMainBulletNum(int value){m_nMainBulletNum = value;}
	void setEnemyNum(int value){m_nEnemyNum = value;}
	void set9SpriteNoBatchNode(bool isNoBatchNode) {m_9SpriteNoBatchNode = isNoBatchNode;}

	bool get9SpriteNoBatchNode() {return m_9SpriteNoBatchNode;}
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	void addDrawTextureIDToVec(GLuint textureID);
	void setTextureInfoState(bool state) {m_bTextureDrawInfo = state;}
	bool getTextureInfoState() {return m_bTextureDrawInfo;}
	void putTextureIDFileMap(std::string &fileName, GLuint textureID);
	void removeTextureIDFromMap(GLuint textureID);
#endif

protected:

    void purgeDirector();
    bool m_bPurgeDirecotorInNextLoop; // this flag will be set to true in end()
    
    void setNextScene(void);
    
    void updateFrameRate();
    void showStats();
    void createStatsLabel();
    void calculateMPF();
    void getFPSImageData(unsigned char** datapointer, unsigned int* length);
    
    /** calculates delta time since last time it was called */    
    void calculateDeltaTime();
protected:
    /* The CCEGLView, where everything is rendered */
    CCEGLView    *m_pobOpenGLView;

    double m_dAnimationInterval;
    double m_dOldAnimationInterval;

    /* landscape mode ? */
    bool m_bLandscape;
    
    bool m_bDisplayStats;
    float m_fAccumDt;
    float m_fFrameRate;
    
    CCLabelAtlas *m_pFPSLabel;
    CCLabelAtlas *m_pSPFLabel;
    CCLabelAtlas *m_pDrawsLabel;
	CCLabelAtlas *m_pDelayLabel;
	CCLabelAtlas *m_pUdpServerDelayLabel;
	CCLabelAtlas *m_pBulletNumLabel;
    CCLabelAtlas *m_pEnemyNumLabel;

    /** Whether or not the Director is paused */
    bool m_bPaused;

    /* How many frames were called since the director started */
    unsigned int m_uTotalFrames;
    unsigned int m_uFrames;
    float m_fSecondsPerFrame;
     
    /* The running scene */
    CCScene *m_pRunningScene;
    
    /* will be the next 'runningScene' in the next frame
     nextScene is a weak reference. */
    CCScene *m_pNextScene;
    
    /* If YES, then "old" scene will receive the cleanup message */
    bool    m_bSendCleanupToScene;

    /* scheduled scenes */
    CCArray* m_pobScenesStack;
    
    /* last time the main loop was updated */
    struct cc_timeval *m_pLastUpdate;

    /* whether or not the next delta time will be zero */
    bool m_bNextDeltaTimeZero;
    
    /* projection used */
    ccDirectorProjection m_eProjection;

    /* window size in points */
    CCSize    m_obWinSizeInPoints;
    
    /* content scale factor */
    float    m_fContentScaleFactor;

    /* store the fps string */
    char *m_pszFPS;

    /* This object will be visited after the scene. Useful to hook a notification node */
    CCNode *m_pNotificationNode;

    /* Projection protocol delegate */
    CCDirectorDelegate *m_pProjectionDelegate;

    HookFunc m_hookBeforeSetNextScene;
    HookFunc m_hookAfterDraw;
    
    // CCEGLViewProtocol will recreate stats labels to fit visible rect
    friend class CCEGLViewProtocol;

	////////////////////////////////////////////////
	// add by camel
	VECCCDRAWSCENELISTENER	m_vecDrawSceneListener;
	////////////////////////////////////////////////
    
    
    CCBrowserEventListener *    m_pBrowserEventListener;

	bool m_bEnableThreadMutual;

	int m_nNetTcpDelay;
	int m_nNetUdpDelay;

	int m_nUdpServerLostRate;
	int m_nUdpServerAvgDelay;
	int m_nUdpServerRealTimeDelay;

	int m_nEnemyBulletNum;
	int m_nMainBulletNum;
	int m_nEnemyNum; 

	bool m_bAutoBatch;
	bool m_9SpriteNoBatchNode; //9����ͼƬ ��ʹ��batchnode

#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	bool m_bTextureDrawInfo;
	std::list<GLuint> m_listDrawTexture;
	std::map<GLuint, std::string> m_mapTextureFile;
#endif
};

/** 
 @brief DisplayLinkDirector is a Director that synchronizes timers with the refresh rate of the display.
 
 Features and Limitations:
  - Scheduled timers & drawing are synchronizes with the refresh rate of the display
  - Only supports animation intervals of 1/60 1/30 & 1/15
 
 @since v0.8.2
 */
class CCDisplayLinkDirector : public CCDirector
{
public:
	CCDisplayLinkDirector(void);

    virtual void mainLoop(void);
	virtual void startRender(void);
	virtual void stopRender(void);
	virtual bool isRendering(void) const;
    virtual void setAnimationInterval(double dValue);
    virtual void startAnimation(void);
    virtual void stopAnimation();

protected:
	bool m_bIsRendering;
	bool m_bIsAnimationPlaying;
};

// end of base_node group
/// @}

NS_CC_END

#endif // __CCDIRECTOR_H__
