//
//  EXWebViewLayer.mm
//  COG
//
//  Created by Liu Yanghui on 12-12-28.
//  Copyright (c) 2012年 BoyoJoy. All rights reserved.
//

#include "EXWebViewNode.h"
#include "EXUIWebViewBridge.h"

static EXUIWebViewBridge *g_EXUIWebViewBridge=nil;

EXWebViewNode::EXWebViewNode(){
    
}

EXWebViewNode::~EXWebViewNode(){
    [g_EXUIWebViewBridge remove];
    [g_EXUIWebViewBridge release];
    
    g_EXUIWebViewBridge = nil;
}

void EXWebViewNode::webViewDidFinishLoad(){
    
}
void EXWebViewNode::goTop()
{
	[g_EXUIWebViewBridge goTop];
}

CCSize EXWebViewNode::getScreenSize()
{
    CCSize size;
    CGRect screenRect = [ UIScreen mainScreen ].bounds;
    size.width = screenRect.size.width;
    size.height = screenRect.size.height;
    return size;
}

bool EXWebViewNode::loadUrl(std::string url,CCPoint pPosition ,CCSize sSize,bool isLocalPage)
{
    if ( !CCNode::init() ){
        return false;
    }
 
    if(g_EXUIWebViewBridge!=nil)
    {
        [g_EXUIWebViewBridge remove];
        [g_EXUIWebViewBridge release];
        
        g_EXUIWebViewBridge = nil;
    }
    
    g_EXUIWebViewBridge = [[EXUIWebViewBridge alloc] init];
    [g_EXUIWebViewBridge setLayerWebView : this URLString:url.c_str() CCPoint:pPosition CCSize:sSize Boolean:isLocalPage];
    return true;
}

/*
EXWebViewNode *EXWebViewNode::create(){
    EXWebViewNode *exWebViewNode = new EXWebViewNode();
    exWebViewNode->autorelease();
    return exWebViewNode;
}
 */
