//
//  EXUIWebViewBridge.h
//  COG
//
//  Created by Liu Yanghui on 12-12-28.
//  Copyright (c) 2012年 BoyoJoy. All rights reserved.
//


#import <Foundation/Foundation.h>
#import <CoreLocation/CoreLocation.h>
#import <UIKit/UIKit.h>
#import "EXWebViewNode.h"

@interface EXUIWebViewBridge : NSObject<UIWebViewDelegate,UIAlertViewDelegate>{
    EXWebViewNode * mLayerWebView;
    UIWebView *mWebView;

}

-(void) setLayerWebView : (EXWebViewNode*) iLayerWebView URLString:(const char*) urlString  CCPoint:(CCPoint) pPoint CCSize:(CCSize) sSize Boolean:(Boolean) isLocalPage;
-(BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType;
-(void)webViewDidStartLoad:(UIWebView *)thisWebView;
-(void)webViewDidFinishLoad:(UIWebView *)thisWebView;
-(void)webView:(UIWebView *)thisWebView didFailLoadWithError:(NSError *)error;
-(void) remove;
-(void) goTop;

@end


