//
//  EXWebViewLayer.h
//  COG
//
//  Created by Liu Yanghui on 12-12-28.
//  Copyright (c) 2012年 BoyoJoy. All rights reserved.
//

#ifndef COG_EXWebViewLayer_h
#define COG_EXWebViewLayer_h

// #include "CCCommon.h"
#include "cocos2d.h"

USING_NS_CC;


class EXWebViewNode : public CCNode{
public:
    EXWebViewNode();
    ~EXWebViewNode();
    
    bool loadUrl(std::string url,CCPoint pPosition ,CCSize size,bool isLocalPage);
    
    // static EXWebViewNode* create();
    
    void webViewDidFinishLoad();

	void goTop();
    
    CCSize getScreenSize();
    
    CREATE_FUNC(EXWebViewNode);

private:
    int mWebViewLoadCounter;
};

#endif
