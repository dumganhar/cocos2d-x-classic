//
//  EXUIWebViewBridge.cpp
//  COG
//
//  Created by Liu Yanghui on 12-12-28.
//  Copyright (c) 2012年 BoyoJoy. All rights reserved.
//

#import "EXUIWebViewBridge.h"
#import "EAGLView.h"

#include "cocos2d.h"

USING_NS_CC;

@implementation EXUIWebViewBridge

- (id)init{
    self = [super init];
    if (self) {
        // init code here.
        mLayerWebView = nil;
        mWebView = nil;
    }
    return self;
}

- (void)dealloc{
    mLayerWebView = nil;
    [mWebView release];
    [super dealloc];
}

-(void) setLayerWebView : (EXWebViewNode*) iLayerWebView URLString:(const char*) urlString CCPoint:(CCPoint) iosWebPos CCSize:(CCSize) iosWebSize Boolean:(Boolean) isLocalPage
{


    mLayerWebView = iLayerWebView;
    
    mWebView = [[UIWebView alloc] initWithFrame:CGRectMake(iosWebPos.x, iosWebPos.y, iosWebSize.width, iosWebSize.height)];
    
    [mWebView setBackgroundColor:[UIColor colorWithRed:227 green:227 blue:227 alpha:1]];
    [mWebView setOpaque:NO];
    [mWebView setHidden:YES];
    
    mWebView.delegate = self;
    
    NSString *urlBase = [NSString stringWithCString:urlString encoding:NSUTF8StringEncoding];
    
    if (isLocalPage) {
        urlBase = [urlBase substringToIndex: [urlBase rangeOfString:@".html"].location];
        NSURL *url = [NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:urlBase ofType:@"html"]];
        [mWebView loadRequest:[NSURLRequest requestWithURL:url]];
    }else
    {
        [mWebView loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:urlBase ]]];
    }
    [mWebView setUserInteractionEnabled:NO]; //don't let the user scroll while things are
    
    [[EAGLView sharedEGLView] addSubview:mWebView];
    
    
    for (id subview in mWebView.subviews)
    {
        if ([[subview class] isSubclassOfClass: [UIScrollView class]])
        {
            ((UIScrollView *)subview).bounces = NO;
        }
    }
}

// 如果返回NO，代表不允许加载这个请求
-(BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType {
    
    // NSLog(@"scheme = '%@'", request.URL.scheme);
    // NSLog(@"absoluteString = '%@'", request.URL.absoluteString);
   
    //int ret = cocos2d::CCDirector::sharedDirector()->fireeEvent_BrowserWillOpen([request.URL.absoluteString UTF8String]);
    int ret = cocos2d::CCDirector::sharedDirector()->fireBrowserEventMessage(CCBrowserEventListener::CCE_MSG_STARTOPENURL,[request.URL.absoluteString UTF8String]);
    if(1==ret)
    {
        return NO;
    }
    
    // 说明协议头是ios
    /*****
    if ([@"ios" isEqualToString:request.URL.scheme]) {
        NSString *url = request.URL.absoluteString;
        
        return NO;
    }
    *****/
    
    return YES;
}

-(void)webViewDidStartLoad:(UIWebView *)thisWebView {
    
    // NSString * ns1 = thisWebView->_backgroundColorSystemColorName;
    
    cocos2d::CCDirector::sharedDirector()->fireBrowserEventMessage(CCBrowserEventListener::CCE_MSG_RECEIVETITLE,[[thisWebView stringByEvaluatingJavaScriptFromString:@"document.title"] UTF8String]);
    
    cocos2d::CCDirector::sharedDirector()->fireBrowserEventMessage(CCBrowserEventListener::CCE_MSG_LOADINGURL,[thisWebView.request.URL.absoluteString UTF8String]);
    
    [mWebView setUserInteractionEnabled:NO];
}

-(void)webViewDidFinishLoad:(UIWebView *)thisWebView{
    
    // NSLog(@"web local='%@'", mWebView.request.URL.absoluteString);
    
    NSLog(@"title = '%@', scheme = '%@', absoluteString = '%@'", [thisWebView stringByEvaluatingJavaScriptFromString:@"document.title"], thisWebView.request.URL.scheme, thisWebView.request.URL.absoluteString);
    
    cocos2d::CCDirector::sharedDirector()->fireBrowserEventMessage(CCBrowserEventListener::CCE_MSG_RECEIVETITLE,[[thisWebView stringByEvaluatingJavaScriptFromString:@"document.title"] UTF8String]);
    
    cocos2d::CCDirector::sharedDirector()->fireBrowserEventMessage(CCBrowserEventListener::CCE_MSG_OPENURLCOMPLETED,[thisWebView.request.URL.absoluteString UTF8String]);
    
    [mWebView setHidden:NO];
    [mWebView setUserInteractionEnabled:YES];
    mLayerWebView->webViewDidFinishLoad();
}

-(void)webView:(UIWebView *)thisWebView didFailLoadWithError:(NSError *)error {
    
    if ([error code] != -999 && error != NULL) { //error -999 happens when the user clicks on something before it's done loading.
        
        cocos2d::CCDirector::sharedDirector()->fireBrowserEventMessage(CCBrowserEventListener::CCE_MSG_WEBKITERROR,[[error domain] UTF8String]);
        
        /***
        UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Network Error" message:@"Unable to load the page. Please keep network connection."
                              
                                                       delegate:self cancelButtonTitle:nil otherButtonTitles:@"OK", nil];
        
        [alert show];
        
        [alert release];
        ***/
        // [error domain]
    }
    
}

-(void) remove {
    
    mWebView.delegate = nil; //keep the webview from firing off any extra messages
    
    [mWebView removeFromSuperview];
   
}

-(void) goTop{
    UIScrollView* scrollView = [[mWebView subviews] objectAtIndex:0];
    [scrollView setContentOffset:CGPointMake(0,0) animated:YES];
}

@end

