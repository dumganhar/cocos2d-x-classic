
#include "CCWebViewNode.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#include "EXWebViewNode.h"
#endif


#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "platform/android/jni/JniHelper.h"
#endif

#include <string>

CCWebViewNode::CCWebViewNode()
{
    m_exwWeb = NULL;
}

void callJniStaticFunction(const std::string &packageName , const std::string & functionName,const std::string & args)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

	JniMethodInfo t;
    if (JniHelper::getStaticMethodInfo(t, packageName.c_str(), functionName.c_str(), args.c_str()))
    {
        t.env->CallStaticVoidMethod(t.classID, t.methodID);
		//CCLog(packageName.c_str());
		//CCLog(functionName.c_str());
	}
#endif
}

int callJniStaticFunctionReturnInt(const std::string & packageName , const std::string & functionName,const std::string & args)
{
	int jIntValue = 0;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

	JniMethodInfo t;
    if (JniHelper::getStaticMethodInfo(t, packageName.c_str(), functionName.c_str(), args.c_str()))
    {
        jIntValue = t.env->CallStaticIntMethod(t.classID, t.methodID);
		//CCLog(packageName.c_str());
		//CCLog(functionName.c_str());
	}
#endif
	return jIntValue; 
}

CCWebViewNode::~CCWebViewNode()
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

	std::string packageName("com/tencent/feiji/MainActivity");
	std::string functionName("closeWebView");
	std::string args("()V");
	callJniStaticFunction(packageName, functionName, args);

#endif

}



void CCWebViewNode::open(bool isLocalPage , std::string url, bool bFullScreen)
{
	m_bIsLocalPage = isLocalPage;
    m_sUrl = url;
    load(bFullScreen);
}

std::string CCWebViewNode::getUrl()
{
    return m_sUrl;
}


void CCWebViewNode::goTop()
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	dynamic_cast<EXWebViewNode *>(m_exwWeb)->goTop();
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	std::string packageName("com/tencent/west/MainActivity");
	std::string functionName("goTop");
	std::string args("()V");
	callJniStaticFunction(packageName, functionName, args);
#endif
}

void CCWebViewNode::load(bool bFullScreen)
{
    CCNode* parent = this->getParent();
    if(parent == NULL)
    {
        CCLog("setUrl must call after addChild.");
        return;
    }


    //像素坐标世界坐标
    CCPoint localPos = getPosition();
    
    //cocos2d game世界坐标
    CCPoint worldPoint = parent->convertToWorldSpace(localPos);
    
    //宽高
    CCSize contentSize = getContentSize();
    
    CCEGLView* view = CCEGLView::sharedOpenGLView();
    
    // screenSize
    CCSize screenSize = view->getFrameSize();
    
    //design size
    CCSize designSize = view->getDesignResolutionSize();
    
    
    //屏幕宽高
    CCSize webSize;
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    m_exwWeb = EXWebViewNode::create();
    m_exwWeb->setPosition(CCPointZero);
    addChild(m_exwWeb);
    
    //屏幕宽高
    webSize =((EXWebViewNode*)m_exwWeb)->getScreenSize();
#endif
    
#if  (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	
	std::string packageName("com/tencent/feiji/MainActivity");
	std::string widthFunctionName("getWebWidth");
	std::string widthargs("()I");
    //width
    int androidWinWidth = callJniStaticFunctionReturnInt(packageName, widthFunctionName, widthargs);
	CCLog("androidWinWidth = %d", androidWinWidth);
    
	std::string heightFunctionName("getWebHeight");
	std::string heightArgs("()I");
    //height
    int androidWinHeight = callJniStaticFunctionReturnInt(packageName, heightFunctionName, heightArgs);
	CCLog("androidWinHeight = %d", androidWinHeight);
    
    //屏幕宽高（androidWinWidth、androidWinHeight jni返回值有问题？）
	webSize.width = androidWinWidth;
    webSize.height = androidWinHeight;
#endif
    
    
#if (CC_TARGET_PLATFORM==CC_PLATFORM_IOS)
    CCRect rect = view->getGLViewPort();
    
    float fDesignScaleX = webSize.width / designSize.width;
    float fDesignScaleY = webSize.height / designSize.height;
    float fDesignScale = (fDesignScaleX<fDesignScaleY)?(fDesignScaleX):(fDesignScaleY);
    
    //计算web在视图中的实际大小
    float fWebWidth = contentSize.width * fDesignScale;
    float fWebHeight = contentSize.height * fDesignScale;

    float fHwScaleX = webSize.width / screenSize.width;
    float fHwScaleY = webSize.height / screenSize.height;
    
    float fIOSWebPosX = (worldPoint.x+rect.origin.x) * fHwScaleX;
    float fIOSWebPosY = webSize.height - (worldPoint.y+rect.origin.y)*fHwScaleY - fWebHeight;
    
    if(2048 == screenSize.height)
    {
        fIOSWebPosX = (worldPoint.x) + rect.origin.x*fHwScaleX;
        fIOSWebPosY = webSize.height - (worldPoint.y+rect.origin.y) - fWebHeight;
    }
    
    if(screenSize.height<960)
    {
        fIOSWebPosX = (worldPoint.x+rect.origin.x) * 0.5;
        fIOSWebPosY = webSize.height - (worldPoint.y+rect.origin.y)*0.5 - fWebHeight;
    }

	if(bFullScreen)
	{
		fWebWidth = webSize.width;
		fWebHeight = webSize.height;

		fIOSWebPosX = 0;
		fIOSWebPosY = 0;
	}

#endif //
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    CCRect rect = view->getGLViewPort();
    
    float fDesignScaleX = contentSize.width / designSize.width;
    float fDesignScaleY = contentSize.height / designSize.height;
    
    float fDesignScale = (fDesignScaleX<fDesignScaleY)?(fDesignScaleX):(fDesignScaleY);
    
    
    //计算web在视图中的实际大小
    float fWebWidth = webSize.width * fDesignScaleX;
    float fWebHeight = webSize.height * fDesignScaleY;
    
    
    fWebWidth = fWebWidth - rect.origin.x;
    fWebHeight = fWebHeight - rect.origin.y;

    
    float fIOSWebPosX = webSize.width * (worldPoint.x)/(designSize.width) + webSize.width * (rect.origin.x/screenSize.width);
    
    float fIOSWebPosY = webSize.height * (worldPoint.y)/(designSize.height) + webSize.height * (rect.origin.y/screenSize.height);
    
    fIOSWebPosY = webSize.height - fIOSWebPosY - fWebHeight;
    
    fIOSWebPosX = fIOSWebPosX + rect.origin.x * 0.5;
    fIOSWebPosY = fIOSWebPosY + rect.origin.y * 0.5;

	if(bFullScreen)
	{
		fWebWidth = webSize.width;
		fWebHeight = webSize.height;

		fIOSWebPosX = 0;
		fIOSWebPosY = 0;
	}

#endif //
    
    
    // CCLog("=====>WebViewNode pos=%f,%f, size=%f,%f, url=%s", fIOSWebPosX, fIOSWebPosY, fWebWidth, fWebHeight, m_sUrl.c_str());
    
#if (CC_TARGET_PLATFORM==CC_PLATFORM_IOS)
    CCPoint iosPos;
    iosPos.x = fIOSWebPosX;
    iosPos.y = fIOSWebPosY;
    
    CCSize iosSize;
    iosSize.width=fWebWidth;
    iosSize.height=fWebHeight;
    
    ((EXWebViewNode*)m_exwWeb)->loadUrl(m_sUrl,iosPos,iosSize,m_bIsLocalPage);
#endif
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	JniMethodInfo t;
    if (JniHelper::getStaticMethodInfo(t, "com/tencent/feiji/MainActivity", "OpenUrlEx", "(Ljava/lang/String;IIIIZ)V"))
    {
		jstring jniStringUrl = t.env->NewStringUTF(m_sUrl.c_str());
        t.env->CallStaticVoidMethod(t.classID, t.methodID,
                                    jniStringUrl,
                                    (int)fIOSWebPosX,
                                    (int)fIOSWebPosY,
                                    (int)fWebWidth,
                                    (int)fWebHeight,
                                    m_bIsLocalPage
                                    );
	}
    
#endif
    
#if(CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	CCLog("CCWebViewNode does not support win32.");
#endif
}
