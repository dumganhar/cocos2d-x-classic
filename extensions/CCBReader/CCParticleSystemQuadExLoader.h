//
//  CCParticleSystemQuadExLoader.h
//  game01
//
//  Created by Archer on 14/2/13.
//
//

#ifndef _CCB_CCPARTICLESYSTEMQUADEXLOADER_H_
#define _CCB_CCPARTICLESYSTEMQUADEXLOADER_H_

#include "CCNodeLoader.h"

NS_CC_EXT_BEGIN

/* Forward declaration. */
class CCBReader;

class CCParticleSystemQuadExLoader : public CCNodeLoader {
public:
    virtual ~CCParticleSystemQuadExLoader() {};
    CCB_STATIC_NEW_AUTORELEASE_OBJECT_METHOD(CCParticleSystemQuadExLoader, loader);
    
protected:
    CCB_VIRTUAL_NEW_AUTORELEASE_CREATECCNODE_METHOD(CCParticleSystemQuadEx);
    
    virtual void onHandlePropTypeIntegerLabeled(CCNode * pNode, CCNode * pParent, const char * pPropertyName, int pIntegerLabeled, CCBReader * pCCBReader);
    virtual void onHandlePropTypePoint(CCNode * pNode, CCNode * pParent, const char * pPropertyName, CCPoint pPoint, CCBReader * pCCBReader);
    virtual void onHandlePropTypeFloat(CCNode * pNode, CCNode * pParent, const char * pPropertyName, float pFloat, CCBReader * pCCBReader);
    virtual void onHandlePropTypeInteger(CCNode * pNode, CCNode * pParent, const char * pPropertyName, int pInteger, CCBReader * pCCBReader);
    virtual void onHandlePropTypeFloatVar(CCNode * pNode, CCNode * pParent, const char * pPropertyName, float * pFloatVar, CCBReader * pCCBReader);
    virtual void onHandlePropTypeColor4FVar(CCNode * pNode, CCNode * pParent, const char * pPropertyName, ccColor4F * pCCColor4FVar, CCBReader * pCCBReader);
    virtual void onHandlePropTypeBlendFunc(CCNode * pNode, CCNode * pParent, const char * pPropertyName, ccBlendFunc pCCBlendFunc, CCBReader * pCCBReader);
    virtual void onHandlePropTypeTexture(CCNode * pNode, CCNode * pParent, const char * pPropertyName, CCTexture2D * pCCTexture2D, CCBReader * pCCBReader);
    //新增Degrees类型属性解释
    virtual void onHandlePropTypeDegrees(CCNode * pNode, CCNode * pParent, const char* pPropertyName, float pDegrees, CCBReader * pCCBReader);
    //解释string类型的属性
    virtual void onHandlePropTypeString(CCNode * pNode, CCNode * pParent, const char* pPropertyName, const char * pString, CCBReader * pCCBReader);
};

NS_CC_EXT_END

#endif

