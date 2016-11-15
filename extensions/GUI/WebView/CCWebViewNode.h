
#ifndef __CCWebViewNode__
#define __CCWebViewNode__

#include <iostream>

#include "cocos2d.h"


//#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
//#include "EXWebViewNode.h"
//#endif

USING_NS_CC;

class CCWebViewNode : public CCNode
{
public:
    CCWebViewNode();
    ~CCWebViewNode();

    virtual void open(bool isLocalPage,std::string url, bool bFullScreen = false);
    virtual std::string getUrl();
    
	virtual void goTop();

    CREATE_FUNC(CCWebViewNode);
private:
    std::string m_sUrl;
	bool m_bIsLocalPage;
    CCNode* m_exwWeb;
    void load(bool bFullScreen);

};



#endif /* defined(__CCWebViewNode__) */