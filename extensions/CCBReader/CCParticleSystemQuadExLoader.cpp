//
//  CCParticleSystemQuadExLoader.cpp
//  game01
//
//  Created by Archer on 14/2/13.
//
//

#include "CCParticleSystemQuadExLoader.h"

#define PROPERTY_EMITERMODE "emitterMode"
#define PROPERTY_POSVAR "posVar"
#define PROPERTY_EMISSIONRATE "emissionRate"
#define PROPERTY_DURATION "duration"
#define PROPERTY_TOTALPARTICLES "totalParticles"
#define PROPERTY_LIFE "life"
#define PROPERTY_STARTSPIN "startSpin"
#define PROPERTY_ENDSPIN "endSpin"
#define PROPERTY_ANGLE "angle"
#define PROPERTY_BLENDFUNC "blendFunc"

#define PROPERTY_KEYPOINTTYPE "keyPointType" //关键点类型
#define PROPERTY_STARTCOLOR "startColor"
#define PROPERTY_STARTSIZE "startSize"
#define PROPERTY_ENDCOLOR "endColor"
#define PROPERTY_ENDSIZE "endSize"
//关键点颜色大小和时间
#define PROPERTY_KEYPOINTACOLOR "keyPointAColor"
#define PROPERTY_KEYPOINTASIZE "keyPointASize"
#define PROPERTY_KEYPOINTALIFEPERCENT "keyPointALifePercent"
#define PROPERTY_KEYPOINTBCOLOR "keyPointBColor"
#define PROPERTY_KEYPOINTBSIZE "keyPointBSize"
#define PROPERTY_KEYPOINTBLIFEPERCENT "keyPointBLifePercent"
#define PROPERTY_KEYPOINTCCOLOR "keyPointCColor"
#define PROPERTY_KEYPOINTCSIZE "keyPointCSize"
#define PROPERTY_KEYPOINTCLIFEPERCENT "keyPointCLifePercent"
#define PROPERTY_KEYPOINTDCOLOR "keyPointDColor"
#define PROPERTY_KEYPOINTDSIZE "keyPointDSize"
#define PROPERTY_KEYPOINTDLIFEPERCENT "keyPointDLifePercent"
#define PROPERTY_KEYPOINTECOLOR "keyPointEColor"
#define PROPERTY_KEYPOINTESIZE "keyPointESize"

#define PROPERTY_GRAVITY "gravity"
#define PROPERTY_SPEED "speed"
#define PROPERTY_TANGENTIALACCEL "tangentialAccel"
#define PROPERTY_RADIALACCEL "radialAccel"

#define PROPERTY_STARTRADIUS "startRadius"
#define PROPERTY_ENDRADIUS "endRadius"
#define PROPERTY_ROTATEPERSECOND "rotatePerSecond"

#define PROPERTY_TEXTURE "texture"

#define PROPERTY_POSITIONTYPE "particlePositionType"
#define PROPERTY_DELAYTIME "delayTime"

#define PROPERTY_OUTLINETYPE "outlineType"
#define PROPERTY_RADIUSX "radiusX"
#define PROPERTY_RADIUSY "radiusY"

#define PROPERTY_FRAMETYPE "particleFrameType"
#define PROPERTY_TEXTURECARVEUPX "textureCarveUpX"
#define PROPERTY_TEXTURECARVEUPY "textureCarveUpY"
#define PROPERTY_TOFRAME "toFrame"
#define PROPERTY_FRAMETIME "frameTime"

#define PROPERTY_TEXTURESCALEX "textureScaleX"
#define PROPERTY_TEXTURESCALEY "textureScaleY"

#define PROPERTY_TEXTUREANGLETYPE "textureAngleType"
#define PROPERTY_TEXTUREANGLE "textureAngle"

NS_CC_EXT_BEGIN

void CCParticleSystemQuadExLoader::onHandlePropTypeIntegerLabeled(CCNode * pNode, CCNode * pParent, const char * pPropertyName, int pIntegerLabeled, CCBReader * pCCBReader) {
    if(strcmp(pPropertyName, PROPERTY_EMITERMODE) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setEmitterMode(pIntegerLabeled);
    } else if (strcmp(pPropertyName, PROPERTY_POSITIONTYPE) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setPositionType((tCCPositionType)pIntegerLabeled);
    } else if (strcmp(pPropertyName, PROPERTY_KEYPOINTTYPE) == 0) {//关键点类型
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointType(pIntegerLabeled);
    } else if (strcmp(pPropertyName, PROPERTY_OUTLINETYPE) == 0) {//外形类型
        ((CCParticleSystemQuadEx *)pNode)->setOutlineType(pIntegerLabeled);
    } else if (strcmp(pPropertyName, PROPERTY_FRAMETYPE) == 0) {//序列帧类型
        ((CCParticleSystemQuadEx *)pNode)->setParticleFrameType(pIntegerLabeled);
    } else if (strcmp(pPropertyName, PROPERTY_TEXTUREANGLETYPE) == 0){//贴图角度类型
        ((CCParticleSystemQuadEx *)pNode)->setTextureAngleType(pIntegerLabeled);
    } else {
        CCNodeLoader::onHandlePropTypeIntegerLabeled(pNode, pParent, pPropertyName, pIntegerLabeled, pCCBReader);
    }
}

void CCParticleSystemQuadExLoader::onHandlePropTypePoint(CCNode * pNode, CCNode * pParent, const char * pPropertyName, CCPoint pPoint, CCBReader * pCCBReader) {
    if(strcmp(pPropertyName, PROPERTY_POSVAR) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setPosVar(pPoint);
    } else if(strcmp(pPropertyName, PROPERTY_GRAVITY) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setGravity(pPoint);
    } else {
        CCNodeLoader::onHandlePropTypePoint(pNode, pParent, pPropertyName, pPoint, pCCBReader);
    }
}

void CCParticleSystemQuadExLoader::onHandlePropTypeFloat(CCNode * pNode, CCNode * pParent, const char * pPropertyName, float pFloat, CCBReader * pCCBReader) {
    if(strcmp(pPropertyName, PROPERTY_EMISSIONRATE) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setEmissionRate(pFloat);
    } else if(strcmp(pPropertyName, PROPERTY_DURATION) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setDuration(pFloat);
    } else if(strcmp(pPropertyName, PROPERTY_KEYPOINTALIFEPERCENT) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointALifePercent(pFloat);
    } else if(strcmp(pPropertyName, PROPERTY_KEYPOINTBLIFEPERCENT) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointBLifePercent(pFloat);
    } else if(strcmp(pPropertyName, PROPERTY_KEYPOINTCLIFEPERCENT) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointCLifePercent(pFloat);
    } else if(strcmp(pPropertyName, PROPERTY_KEYPOINTDLIFEPERCENT) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointDLifePercent(pFloat);
    } else if(strcmp(pPropertyName, PROPERTY_TEXTURESCALEX) == 0) {
        //CCLog("pFloat=%f",pFloat);
        ((CCParticleSystemQuadEx *)pNode)->setTextureScaleX(pFloat);
    } else if(strcmp(pPropertyName, PROPERTY_TEXTURESCALEY) == 0) {
        //CCLog("pFloat=%f",pFloat);
        ((CCParticleSystemQuadEx *)pNode)->setTextureScaleY(pFloat);
    }  else {
        CCNodeLoader::onHandlePropTypeFloat(pNode, pParent, pPropertyName, pFloat, pCCBReader);
    }
}

void CCParticleSystemQuadExLoader::onHandlePropTypeInteger(CCNode * pNode, CCNode * pParent, const char * pPropertyName, int pInteger, CCBReader * pCCBReader) {
    if(strcmp(pPropertyName, PROPERTY_TOTALPARTICLES) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setTotalParticles(pInteger);
    } else {
        CCNodeLoader::onHandlePropTypeInteger(pNode, pParent, pPropertyName, pInteger, pCCBReader);
    }
}

void CCParticleSystemQuadExLoader::onHandlePropTypeFloatVar(CCNode * pNode, CCNode * pParent, const char * pPropertyName, float * pFloatVar, CCBReader * pCCBReader) {
    if(strcmp(pPropertyName, PROPERTY_LIFE) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setLife(pFloatVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setLifeVar(pFloatVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_STARTSIZE) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setStartSize(pFloatVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setStartSizeVar(pFloatVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_ENDSIZE) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setEndSize(pFloatVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setEndSizeVar(pFloatVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_STARTSPIN) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setStartSpin(pFloatVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setStartSpinVar(pFloatVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_ENDSPIN) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setEndSpin(pFloatVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setEndSpinVar(pFloatVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_ANGLE) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setAngle(pFloatVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setAngleVar(pFloatVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_SPEED) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setSpeed(pFloatVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setSpeedVar(pFloatVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_TANGENTIALACCEL) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setTangentialAccel(pFloatVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setTangentialAccelVar(pFloatVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_RADIALACCEL) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setRadialAccel(pFloatVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setRadialAccelVar(pFloatVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_STARTRADIUS) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setStartRadius(pFloatVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setStartRadiusVar(pFloatVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_ENDRADIUS) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setEndRadius(pFloatVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setEndRadiusVar(pFloatVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_ROTATEPERSECOND) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setRotatePerSecond(pFloatVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setRotatePerSecondVar(pFloatVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_KEYPOINTASIZE) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointASize(pFloatVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointASizeVar(pFloatVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_KEYPOINTBSIZE) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointBSize(pFloatVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointBSizeVar(pFloatVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_KEYPOINTCSIZE) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointCSize(pFloatVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointCSizeVar(pFloatVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_KEYPOINTDSIZE) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointDSize(pFloatVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointDSizeVar(pFloatVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_KEYPOINTESIZE) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointESize(pFloatVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointESizeVar(pFloatVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_RADIUSX) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setRadiusX(pFloatVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setRadiusXVar(pFloatVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_RADIUSY) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setRadiusY(pFloatVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setRadiusYVar(pFloatVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_TEXTUREANGLE) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setTextureAngle(pFloatVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setTextureAngleVar(pFloatVar[1]);
    } else {
        CCNodeLoader::onHandlePropTypeFloatVar(pNode, pParent, pPropertyName, pFloatVar, pCCBReader);
    }
}

void CCParticleSystemQuadExLoader::onHandlePropTypeColor4FVar(CCNode * pNode, CCNode * pParent, const char * pPropertyName, ccColor4F * pCCColor4FVar, CCBReader * pCCBReader) {
    if(strcmp(pPropertyName, PROPERTY_STARTCOLOR) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setStartColor(pCCColor4FVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setStartColorVar(pCCColor4FVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_ENDCOLOR) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setEndColor(pCCColor4FVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setEndColorVar(pCCColor4FVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_KEYPOINTACOLOR) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointAColor(pCCColor4FVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointAColorVar(pCCColor4FVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_KEYPOINTBCOLOR) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointBColor(pCCColor4FVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointBColorVar(pCCColor4FVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_KEYPOINTCCOLOR) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointCColor(pCCColor4FVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointCColorVar(pCCColor4FVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_KEYPOINTDCOLOR) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointDColor(pCCColor4FVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointDColorVar(pCCColor4FVar[1]);
    } else if(strcmp(pPropertyName, PROPERTY_KEYPOINTECOLOR) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointEColor(pCCColor4FVar[0]);
        ((CCParticleSystemQuadEx *)pNode)->setKeyPointEColorVar(pCCColor4FVar[1]);
    } else {
        CCNodeLoader::onHandlePropTypeColor4FVar(pNode, pParent, pPropertyName, pCCColor4FVar, pCCBReader);
    }
}

void CCParticleSystemQuadExLoader::onHandlePropTypeBlendFunc(CCNode * pNode, CCNode * pParent, const char * pPropertyName, ccBlendFunc pCCBlendFunc, CCBReader * pCCBReader) {
    if(strcmp(pPropertyName, PROPERTY_BLENDFUNC) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setBlendFunc(pCCBlendFunc);
    } else {
        CCNodeLoader::onHandlePropTypeBlendFunc(pNode, pParent, pPropertyName, pCCBlendFunc, pCCBReader);
    }
}

void CCParticleSystemQuadExLoader::onHandlePropTypeTexture(CCNode * pNode, CCNode * pParent, const char * pPropertyName, CCTexture2D * pCCTexture2D, CCBReader * pCCBReader) {
    if(strcmp(pPropertyName, PROPERTY_TEXTURE) == 0) 
	{
		if (NULL != pCCTexture2D)
		{
			((CCParticleSystemQuadEx *)pNode)->setTexture(pCCTexture2D);
		}
		else
		{
			CCLOG("ERROR: CCParticleSystemQuadExLoader::onHandlePropTypeTexture Texture NULL");
			assert(false);
		}        
    } else {
        CCNodeLoader::onHandlePropTypeTexture(pNode, pParent, pPropertyName, pCCTexture2D, pCCBReader);
    }
}

/*
 * Degrees类型属性解释器
 */
void CCParticleSystemQuadExLoader::onHandlePropTypeDegrees(CCNode * pNode, CCNode * pParent, const char* pPropertyName, float pDegrees, CCBReader * pCCBReader)
{
    if(strcmp(pPropertyName, PROPERTY_DELAYTIME) == 0) {
        if(pDegrees == 0)
            return;
        
        ((CCParticleSystemQuadEx *)pNode)->setDelayTime(pDegrees);
    } else if(strcmp(pPropertyName, PROPERTY_TEXTURECARVEUPX) == 0){
        ((CCParticleSystemQuadEx *)pNode)->setTextureCarveUpX(pDegrees);
    } else if(strcmp(pPropertyName, PROPERTY_TEXTURECARVEUPY) == 0){
        ((CCParticleSystemQuadEx *)pNode)->setTextureCarveUpY(pDegrees);
    } else if(strcmp(pPropertyName, PROPERTY_TOFRAME) == 0){
        ((CCParticleSystemQuadEx *)pNode)->setToFrame(pDegrees);
    } else {
        CCNodeLoader::onHandlePropTypeDegrees(pNode, pParent, pPropertyName, pDegrees, pCCBReader);
    }
}

/*
 * String类型属性解释
 */
void CCParticleSystemQuadExLoader::onHandlePropTypeString(CCNode * pNode, CCNode * pParent, const char* pPropertyName, const char * pString, CCBReader * pCCBReader)
{
    if(strcmp(pPropertyName, PROPERTY_FRAMETIME) == 0) {
        ((CCParticleSystemQuadEx *)pNode)->setFrameTime(pString);
    }
}

NS_CC_EXT_END