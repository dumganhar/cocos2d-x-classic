//
//  CCParticleSystemEx.cpp
//  cocos2dx
//
//  Created by Archer on 14/2/12.
//  Copyright (c) 2014年 cocos2d-x. All rights reserved.
//

#include "CCParticleSystemEx.h"
#include "textures/CCTexture2D.h"
#include "support/CCPointExtension.h"
#include "CCParticleBatchNode.h"
#include "support/CCProfiling.h"

NS_CC_BEGIN

CCParticleSystemEx::CCParticleSystemEx()
:m_outlineType(0),
m_keyPointType(0),
m_toFrame(0),
m_strFrameTime(""),
m_particleFrameType(0),
m_totalFrameId(0),
m_textureScaleX(1),
m_textureScaleY(1)
{
    m_particleTextureFrame.carveUpX = 0;
    m_particleTextureFrame.carveUpY = 0 ;
    m_particleTextureFrame.totalFrame = 0;
    
    //outlineType初始化设置
    m_outlineType = kCCParticleOutlineRadius;
    //关键点类型初始化
    m_keyPointType = kCCParticleKeyPointNormal;
}
CCParticleSystemEx::~CCParticleSystemEx()
{
    CC_SAFE_FREE(m_pKeyPoints);
    CC_SAFE_FREE(m_pTimeToFrame);
    CC_SAFE_FREE(m_pParticleLifeTime);
    CC_SAFE_FREE(m_pParticleCurrentFrame);
    CC_SAFE_FREE(m_pParticleTextureAngle);
    CC_SAFE_FREE(m_pRadius);
    CC_SAFE_FREE(m_particleTextureFrame.frameRectMap);
    
    for (unsigned int ids = 0; ids < m_uAllocatedParticles; ids++) {
        CC_SAFE_FREE(m_pKeyPointsRandom[ids].deltaSize);
        CC_SAFE_FREE(m_pKeyPointsRandom[ids].deltaColor);
    }
    
    CC_SAFE_FREE( m_pKeyPointsRandom);
}

bool CCParticleSystemEx::initWithDictionary(CCDictionary *dictionary, const char *dirname)
{
    //outline属性设置
    m_outlineType = dictionary->valueForKey("outlineType")->intValue();
    if (m_outlineType == kCCParticleOutlineRadius) {
        _outline.circle.radiusX = dictionary->valueForKey("radiusX")->floatValue();
        _outline.circle.radiusXVar = dictionary->valueForKey("radiusXVar")->floatValue();
        
        _outline.circle.radiusY = dictionary->valueForKey("radiusY")->floatValue();
        _outline.circle.radiusYVar = dictionary->valueForKey("radiusYVar")->floatValue();
    }
    
    float r,g,b,a;
    m_keyPointType = dictionary->valueForKey("keyPointType")->intValue();
    //keyPointA
    r = dictionary->valueForKey("keyPointAColorRed")->floatValue();
    g = dictionary->valueForKey("keyPointAColorGreen")->floatValue();
    b = dictionary->valueForKey("keyPointAColorBlue")->floatValue();
    a = dictionary->valueForKey("keyPointAColorAlpha")->floatValue();
    m_pKeyPoints[0].color.r = r;
    m_pKeyPoints[0].color.g = g;
    m_pKeyPoints[0].color.b = b;
    m_pKeyPoints[0].color.a = a;

    r = dictionary->valueForKey("keyPointAColorVarRed")->floatValue();
    g = dictionary->valueForKey("keyPointAColorVarGreen")->floatValue();
    b = dictionary->valueForKey("keyPointAColorVarBlue")->floatValue();
    a = dictionary->valueForKey("keyPointAColorVarAlpha")->floatValue();
    m_pKeyPoints[0].colorVar.r = r;
    m_pKeyPoints[0].colorVar.g = g;
    m_pKeyPoints[0].colorVar.b = b;
    m_pKeyPoints[0].colorVar.a = a;
    
    m_pKeyPoints[0].size = (float)dictionary->valueForKey("keyPointASize")->intValue();
    m_pKeyPoints[0].sizeVar = (float)dictionary->valueForKey("keyPointASizeVar")->intValue();
    m_pKeyPoints[0].lifePercent = (float)dictionary->valueForKey("keyPointALifePercent")->intValue();
    
    //keyPointB
    r = dictionary->valueForKey("keyPointBColorRed")->floatValue();
    g = dictionary->valueForKey("keyPointBColorGreen")->floatValue();
    b = dictionary->valueForKey("keyPointBColorBlue")->floatValue();
    a = dictionary->valueForKey("keyPointBColorAlpha")->floatValue();
    m_pKeyPoints[1].color.r = r;
    m_pKeyPoints[1].color.g = g;
    m_pKeyPoints[1].color.b = b;
    m_pKeyPoints[1].color.a = a;
    
    r = dictionary->valueForKey("keyPointBColorVarRed")->floatValue();
    g = dictionary->valueForKey("keyPointBColorVarGreen")->floatValue();
    b = dictionary->valueForKey("keyPointBColorVarBlue")->floatValue();
    a = dictionary->valueForKey("keyPointBColorVarAlpha")->floatValue();
    m_pKeyPoints[1].colorVar.r = r;
    m_pKeyPoints[1].colorVar.g = g;
    m_pKeyPoints[1].colorVar.b = b;
    m_pKeyPoints[1].colorVar.a = a;
    
    m_pKeyPoints[1].size = (float)dictionary->valueForKey("keyPointBSize")->intValue();
    m_pKeyPoints[1].sizeVar = (float)dictionary->valueForKey("keyPointBSizeVar")->intValue();
    m_pKeyPoints[1].lifePercent = (float)dictionary->valueForKey("keyPointBLifePercent")->intValue();
    
    //keyPointC
    r = dictionary->valueForKey("keyPointCColorRed")->floatValue();
    g = dictionary->valueForKey("keyPointCColorGreen")->floatValue();
    b = dictionary->valueForKey("keyPointCColorBlue")->floatValue();
    a = dictionary->valueForKey("keyPointCColorAlpha")->floatValue();
    m_pKeyPoints[2].color.r = r;
    m_pKeyPoints[2].color.g = g;
    m_pKeyPoints[2].color.b = b;
    m_pKeyPoints[2].color.a = a;
    
    r = dictionary->valueForKey("keyPointCColorVarRed")->floatValue();
    g = dictionary->valueForKey("keyPointCColorVarGreen")->floatValue();
    b = dictionary->valueForKey("keyPointCColorVarBlue")->floatValue();
    a = dictionary->valueForKey("keyPointCColorVarAlpha")->floatValue();
    m_pKeyPoints[2].colorVar.r = r;
    m_pKeyPoints[2].colorVar.g = g;
    m_pKeyPoints[2].colorVar.b = b;
    m_pKeyPoints[2].colorVar.a = a;
    
    m_pKeyPoints[2].size = (float)dictionary->valueForKey("keyPointCSize")->intValue();
    m_pKeyPoints[2].sizeVar = (float)dictionary->valueForKey("keyPointCSizeVar")->intValue();
    m_pKeyPoints[2].lifePercent = (float)dictionary->valueForKey("keyPointCLifePercent")->intValue();
    
    //keyPointD
    r = dictionary->valueForKey("keyPointDColorRed")->floatValue();
    g = dictionary->valueForKey("keyPointDColorGreen")->floatValue();
    b = dictionary->valueForKey("keyPointDColorBlue")->floatValue();
    a = dictionary->valueForKey("keyPointDColorAlpha")->floatValue();
    m_pKeyPoints[3].color.r = r;
    m_pKeyPoints[3].color.g = g;
    m_pKeyPoints[3].color.b = b;
    m_pKeyPoints[3].color.a = a;

    r = dictionary->valueForKey("keyPointDColorVarRed")->floatValue();
    g = dictionary->valueForKey("keyPointDColorVarGreen")->floatValue();
    b = dictionary->valueForKey("keyPointDColorVarBlue")->floatValue();
    a = dictionary->valueForKey("keyPointDColorVarAlpha")->floatValue();
    m_pKeyPoints[3].colorVar.r = r;
    m_pKeyPoints[3].colorVar.g = g;
    m_pKeyPoints[3].colorVar.b = b;
    m_pKeyPoints[3].colorVar.a = a;

    m_pKeyPoints[3].size = (float)dictionary->valueForKey("keyPointDSize")->intValue();
    m_pKeyPoints[3].sizeVar = (float)dictionary->valueForKey("keyPointDSizeVar")->intValue();
    m_pKeyPoints[3].lifePercent = (float)dictionary->valueForKey("keyPointDLifePercent")->intValue();
    
    //keyPointE
    r = dictionary->valueForKey("keyPointEColorRed")->floatValue();
    g = dictionary->valueForKey("keyPointEColorGreen")->floatValue();
    b = dictionary->valueForKey("keyPointEColorBlue")->floatValue();
    a = dictionary->valueForKey("keyPointEColorAlpha")->floatValue();
    m_pKeyPoints[4].color.r = r;
    m_pKeyPoints[4].color.g = g;
    m_pKeyPoints[4].color.b = b;
    m_pKeyPoints[4].color.a = a;

    r = dictionary->valueForKey("keyPointEColorVarRed")->floatValue();
    g = dictionary->valueForKey("keyPointEColorVarGreen")->floatValue();
    b = dictionary->valueForKey("keyPointEColorVarBlue")->floatValue();
    a = dictionary->valueForKey("keyPointEColorVarAlpha")->floatValue();
    m_pKeyPoints[4].colorVar.r = r;
    m_pKeyPoints[4].colorVar.g = g;
    m_pKeyPoints[4].colorVar.b = b;
    m_pKeyPoints[4].colorVar.a = a;
    
    m_pKeyPoints[4].size = (float)dictionary->valueForKey("keyPointESize")->intValue();
    m_pKeyPoints[4].sizeVar = (float)dictionary->valueForKey("keyPointESizeVar")->intValue();
    m_pKeyPoints[4].lifePercent = (float)dictionary->valueForKey("keyPointELifePercent")->intValue();
    
    CCParticleSystem::initWithDictionary(dictionary, dirname);
    
    return true;
}

//-(id) initWithTotalParticles:(NSUInteger) numberOfParticles
bool CCParticleSystemEx::initWithTotalParticles(unsigned int numberOfParticles)
{
    //calloc分配内存
    m_pKeyPoints            = (tCCKeyPoint*)calloc( PARTICLE_KEYPOINT_TOTALL, sizeof(tCCKeyPoint) );
    
    m_pTimeToFrame          = (double*)calloc(1, sizeof(double));
    m_pParticleLifeTime     = (float*)calloc(numberOfParticles, sizeof(float));
    m_pParticleCurrentFrame = (double*)calloc(numberOfParticles, sizeof(double));
    m_pParticleTextureAngle = (float*)calloc(numberOfParticles, sizeof(float));
    m_pRadius               = (tCCRadius*)calloc(numberOfParticles, sizeof(tCCRadius));
    m_pKeyPointsRandom      = (tCCKyePointRandom*)calloc(numberOfParticles, sizeof(tCCKyePointRandom) * PARTICLE_KEYPOINT_TOTALL);
    //初始化
    for (unsigned int i = 0; i < numberOfParticles; i++)
    {
        m_pParticleLifeTime[i]              = 0;
        m_pParticleCurrentFrame[i]          = -1.0;
        m_pParticleTextureAngle[i]          = 0;
        m_pRadius[i].radiusX                = 0 ;
        m_pRadius[i].radiusY                = 0;
        m_pKeyPointsRandom[i].deltaSize     = (float*)calloc( PARTICLE_KEYPOINT_TOTALL, sizeof(float) );
        m_pKeyPointsRandom[i].deltaColor    = (ccColor4F*)calloc( PARTICLE_KEYPOINT_TOTALL, sizeof(ccColor4F) );
        for (unsigned int j = 0; j < PARTICLE_KEYPOINT_TOTALL; j++) {
            m_pKeyPointsRandom[i].deltaSize[j]      = 0;
            m_pKeyPointsRandom[i].deltaColor[j].r   = 0;
            m_pKeyPointsRandom[i].deltaColor[j].g   = 0;
            m_pKeyPointsRandom[i].deltaColor[j].b   = 0;
            m_pKeyPointsRandom[i].deltaColor[j].a   = 0;
        }
    }
    
    CCParticleSystem::initWithTotalParticles(numberOfParticles);
    
    return true;
}

bool CCParticleSystemEx::addParticle()
{
    if (this->isFull())
    {
        return false;
    }
    
    tCCParticle * particle = &m_pParticles[ m_uParticleCount ];
    this->initParticle(particle);
    ++m_uParticleCount;
    
    return true;
}

void CCParticleSystemEx::initParticle(tCCParticle* particle)
{
	// timeToLive
	// no negative life. prevent division by 0
	particle->timeToLive    = m_fLife + m_fLifeVar * CCRANDOM_MINUS1_1();
	particle->timeToLive    = MAX(0, particle->timeToLive);
    
    m_pParticleLifeTime[m_uParticleCount]       = particle->timeToLive;
    m_pParticleCurrentFrame[m_uParticleCount]   = floor(CCRANDOM_0_1()*m_particleTextureFrame.totalFrame);
    
	// position
	particle->pos.x = m_tSourcePosition.x + m_tPosVar.x * CCRANDOM_MINUS1_1();
	particle->pos.y = m_tSourcePosition.y + m_tPosVar.y * CCRANDOM_MINUS1_1();
    
    //当关键点类型为kCCParticleKeyPointNormal时，使用startColor，endColor，startSize，endSize；
    if(m_keyPointType == kCCParticleKeyPointNormal){
        // Color
        ccColor4F start;
        start.r = clampf( m_tStartColor.r + m_tStartColorVar.r * CCRANDOM_MINUS1_1(), 0, 1);
        start.g = clampf( m_tStartColor.g + m_tStartColorVar.g * CCRANDOM_MINUS1_1(), 0, 1);
        start.b = clampf( m_tStartColor.b + m_tStartColorVar.b * CCRANDOM_MINUS1_1(), 0, 1);
        start.a = clampf( m_tStartColor.a + m_tStartColorVar.a * CCRANDOM_MINUS1_1(), 0, 1);
        
        ccColor4F end;
        end.r = clampf( m_tEndColor.r + m_tEndColorVar.r * CCRANDOM_MINUS1_1(), 0, 1);
        end.g = clampf( m_tEndColor.g + m_tEndColorVar.g * CCRANDOM_MINUS1_1(), 0, 1);
        end.b = clampf( m_tEndColor.b + m_tEndColorVar.b * CCRANDOM_MINUS1_1(), 0, 1);
        end.a = clampf( m_tEndColor.a + m_tEndColorVar.a * CCRANDOM_MINUS1_1(), 0, 1);
        
        particle->color = start;
        particle->deltaColor.r = (end.r - start.r) / particle->timeToLive;
        particle->deltaColor.g = (end.g - start.g) / particle->timeToLive;
        particle->deltaColor.b = (end.b - start.b) / particle->timeToLive;
        particle->deltaColor.a = (end.a - start.a) / particle->timeToLive;
        
        // size
        float startS = m_fStartSize + m_fStartSizeVar * CCRANDOM_MINUS1_1();
        startS = MAX(0, startS); // No negative value
        
        particle->size = startS;
        if( m_fEndSize == kCCParticleStartSizeEqualToEndSize ){
            particle->deltaSize = 0;
        } else {
            float endS = m_fEndSize + m_fEndSizeVar * CCRANDOM_MINUS1_1();
            endS = MAX(0, endS);	// No negative values
            particle->deltaSize = (endS - startS) / particle->timeToLive;
        }
    }else if(m_keyPointType == kCCParticleKeyPointMulti){//当关键点类型为kCCParticleKeyPointMulti时使用多节点数据
        ccColor4F* colors = (ccColor4F*)calloc( PARTICLE_KEYPOINT_TOTALL, sizeof(ccColor4F));
        float*      sizes = (float*)calloc(PARTICLE_KEYPOINT_TOTALL, sizeof(float));
        
        for (unsigned int ids = 0; ids < PARTICLE_KEYPOINT_TOTALL; ids++) {
            tCCKeyPoint* keyPoint = &m_pKeyPoints[ids];
            colors[ids].r = clampf( keyPoint->color.r + keyPoint->colorVar.r * CCRANDOM_MINUS1_1(), 0, 1);
            colors[ids].g = clampf( keyPoint->color.g + keyPoint->colorVar.g * CCRANDOM_MINUS1_1(), 0, 1);
            colors[ids].b = clampf( keyPoint->color.b + keyPoint->colorVar.b * CCRANDOM_MINUS1_1(), 0, 1);
            colors[ids].a = clampf( keyPoint->color.a + keyPoint->colorVar.a * CCRANDOM_MINUS1_1(), 0, 1);
            
            sizes[ids] = MAX(0,(keyPoint->size + keyPoint->sizeVar * CCRANDOM_MINUS1_1()));
        }
        //初始颜色大小设置
        particle->color = colors[0];
        particle->size = sizes[0];
        
        m_pKeyPoints[0].totalPercent   = m_pKeyPoints[0].lifePercent;
        //计算颜色大小变化率
        float particleLifePercent = 0;
        tCCKyePointRandom* keyPointsRandom = &m_pKeyPointsRandom[m_uParticleCount];
        
        for (unsigned int ids = 1; ids < PARTICLE_KEYPOINT_TOTALL; ids++) {
            ccColor4F fromColor     = colors[ids-1];
            ccColor4F toColor       = colors[ids];
                         
            tCCKeyPoint* keyPoint   = &m_pKeyPoints[ids-1];
            particleLifePercent     = keyPoint->lifePercent;
            ccColor4F* keyPointsRanCol  = &(keyPointsRandom->deltaColor[ids -1]);
            
            if (particleLifePercent == 0) {
                keyPointsRanCol->r = 0;
                keyPointsRanCol->g = 0;
                keyPointsRanCol->b = 0;
                keyPointsRanCol->a = 0;
                keyPointsRandom->deltaSize[ids - 1]    = 0;
            }else{
                keyPointsRanCol->r = (toColor.r - fromColor.r) / ((particle->timeToLive)*particleLifePercent);
                keyPointsRanCol->g = (toColor.g - fromColor.g) / ((particle->timeToLive)*particleLifePercent);
                keyPointsRanCol->b = (toColor.b - fromColor.b) / ((particle->timeToLive)*particleLifePercent);
                keyPointsRanCol->a = (toColor.a - fromColor.a) / ((particle->timeToLive)*particleLifePercent);
                keyPointsRandom->deltaSize[ids - 1]    = (sizes[ids] - sizes[ids-1])/((particle->timeToLive)*particleLifePercent);
            }
            
            m_pKeyPoints[ids].totalPercent   = keyPoint->totalPercent + m_pKeyPoints[ids].lifePercent;
        }
        
        if( m_fEndSize == kCCParticleStartSizeEqualToEndSize ){
            m_pKeyPoints[0].deltaSize = 0;
            m_pKeyPoints[1].deltaSize = 0;
            m_pKeyPoints[2].deltaSize = 0;
            m_pKeyPoints[3].deltaSize = 0;
        }

		CC_SAFE_FREE(colors);
		CC_SAFE_FREE(sizes);
    }
    
	// rotation
	float startA = m_fStartSpin + m_fStartSpinVar * CCRANDOM_MINUS1_1();
	float endA = m_fEndSpin + m_fEndSpinVar * CCRANDOM_MINUS1_1();
	particle->rotation = startA;
	particle->deltaRotation = (endA - startA) / particle->timeToLive;
    
	// direction
	float a = CC_DEGREES_TO_RADIANS( m_fAngle + m_fAngleVar * CCRANDOM_MINUS1_1() );
    
    if (m_textureAngleType == kCCParticleAngle) {
        //贴图角度和粒子的初始角度一致
        m_pParticleTextureAngle[m_uParticleCount] = (float)(M_PI * 0.5 - a);
    } else if (m_textureAngleType == kCCToAngle){
        //随机贴图角度
        m_pParticleTextureAngle[m_uParticleCount] = CC_DEGREES_TO_RADIANS(m_textureAngle + m_textureAngleVar * CCRANDOM_MINUS1_1());
    }
    
    //修改粒子初始位置
    CCPoint v (cosf( a ), sinf( a ));
    CCPoint outlinePoint;
    float radiusX = (_outline.circle.radiusX + CCRANDOM_0_1()*_outline.circle.radiusXVar);
    float radiusY = (_outline.circle.radiusY + CCRANDOM_0_1()*_outline.circle.radiusYVar);
    if (m_outlineType == kCCParticleOutlineRadius) {
        outlinePoint.x = radiusX * cosf(a);
        outlinePoint.y = radiusY * sinf(a);
    }else{
        outlinePoint = CCPointZero;
    }
    //根据_positionType类型设置粒子初始坐标位置
    if( m_ePositionType == kCCPositionTypeFree ){
		particle->startPos = ccpAdd(convertToWorldSpace(CCPointZero), ccpAdd(v,outlinePoint));
	}else if( m_ePositionType == kCCPositionTypeRelative ){
		particle->startPos = ccpAdd(m_obPosition,ccpAdd(v,outlinePoint));
    }else if( m_ePositionType == kCCPositionTypeGrouped){
        particle->pos = ccpAdd(particle->pos,ccpAdd(v,outlinePoint));
    }
    
	// Mode Gravity: A
	if( m_nEmitterMode == kCCParticleModeGravity ) {
        
		//CGPoint v = {cosf( a ), sinf( a )};
		float s = modeA.speed + modeA.speedVar * CCRANDOM_MINUS1_1();
        
		// direction
		particle->modeA.dir = ccpMult( v, s );
        
		// radial accel
		particle->modeA.radialAccel = modeA.radialAccel + modeA.radialAccelVar * CCRANDOM_MINUS1_1();
        
		// tangential accel
		particle->modeA.tangentialAccel = modeA.tangentialAccel + modeA.tangentialAccelVar * CCRANDOM_MINUS1_1();
	}
    
	// Mode Radius: B
	else {
		// Set the default diameter of the particle from the source position
		float startRadius = modeB.startRadius + modeB.startRadiusVar * CCRANDOM_MINUS1_1();
		float endRadius = modeB.endRadius + modeB.endRadiusVar * CCRANDOM_MINUS1_1();
        
        m_pRadius[m_uParticleCount].radiusX = radiusX;
        m_pRadius[m_uParticleCount].radiusY = radiusY;
        
		particle->modeB.radius = startRadius;
        
		if( modeB.endRadius == kCCParticleStartRadiusEqualToEndRadius ){
			particle->modeB.deltaRadius = 0;
        }else{
                particle->modeB.deltaRadius = (endRadius - startRadius) / particle->timeToLive;
        }
        
        particle->modeB.angle = a;
        particle->modeB.degreesPerSecond = CC_DEGREES_TO_RADIANS(modeB.rotatePerSecond + modeB.rotatePerSecondVar * CCRANDOM_MINUS1_1());
    }
}
void CCParticleSystemEx::update(float dt)
{
	CC_PROFILER_HELPER;
	CC_PROFILER_START_CATEGORY(kCCProfilerCategoryParticles , @"CCParticleSystem - update");
	addRunningCount();
	if (!m_bVisible)
	{
		return;
	}

	if( m_bIsActive && m_fEmissionRate ) {
		float rate = 1.0f / m_fEmissionRate;
		
		//issue #1201, prevent bursts of particles, due to too high emitCounter
		if (m_uParticleCount < m_uTotalParticles)
			m_fEmitCounter += dt;
            
            while( m_uParticleCount < m_uTotalParticles && m_fEmitCounter > rate ) {
                this->addParticle();
                m_fEmitCounter -= rate;
            }
        
		m_fElapsed += dt;
        
		if(m_fDuration != -1 && m_fDuration < m_fElapsed){
            this->stopSystem();
        }
	}
    
	m_uParticleIdx = 0;
    
	CCPoint currentPosition = CCPointZero;
	if( m_ePositionType == kCCPositionTypeFree )
		currentPosition = convertToWorldSpace(CCPointZero);
        
        else if( m_ePositionType == kCCPositionTypeRelative )
            currentPosition = m_obPosition;
            
            if (m_bVisible)
            {
                while( m_uParticleIdx < m_uParticleCount )
                {
                    tCCParticle *p = &m_pParticles[m_uParticleIdx];
                    
                    // life
                    p->timeToLive -= dt;
                    
                    if( p->timeToLive > 0 ) {
                        // Mode A: gravity, direction, tangential accel & radial accel
                        if( m_nEmitterMode == kCCParticleModeGravity ) {
                            CCPoint tmp, radial, tangential;
                            
                            radial = CCPointZero;
                            // radial acceleration
                            if(p->pos.x || p->pos.y)
                                radial = ccpNormalize(p->pos);
                                
                                tangential = radial;
                                radial = ccpMult(radial, p->modeA.radialAccel);
                                
                                // tangential acceleration
                                float newy = tangential.x;
                                tangential.x = -tangential.y;
                                tangential.y = newy;
                                tangential = ccpMult(tangential, p->modeA.tangentialAccel);
                                
                                // (gravity + radial + tangential) * dt
                                tmp = ccpAdd( ccpAdd( radial, tangential), modeA.gravity);
                                tmp = ccpMult( tmp, dt);
                                p->modeA.dir = ccpAdd( p->modeA.dir, tmp);
                                tmp = ccpMult(p->modeA.dir, dt);
                                p->pos = ccpAdd( p->pos, tmp );
                        }
                        
                        // Mode B: radius movement
                        else {
                            // Update the angle and radius of the particle.
                            p->modeB.angle += p->modeB.degreesPerSecond * dt;
                            p->modeB.radius += p->modeB.deltaRadius * dt;
                            
                            //添加椭圆控制
                            p->pos.x = - cosf(p->modeB.angle) * (p->modeB.radius + m_pRadius[m_uParticleIdx].radiusX);
                            p->pos.y = - sinf(p->modeB.angle) * (p->modeB.radius + m_pRadius[m_uParticleIdx].radiusY);
                        }
                        
                        if(m_keyPointType == kCCParticleKeyPointNormal){
                            // color
                            p->color.r += (p->deltaColor.r * dt);
                            p->color.g += (p->deltaColor.g * dt);
                            p->color.b += (p->deltaColor.b * dt);
                            p->color.a += (p->deltaColor.a * dt);
                            
                            // size
                            p->size += (p->deltaSize * dt);
                            p->size = MAX( 0, p->size );
                        }else if(m_keyPointType == kCCParticleKeyPointMulti){
                            ccColor4F oldColor  = p->color;
                            float oldSize       = p->size;
                            float totalPercent  = 1 - p->timeToLive/m_pParticleLifeTime[m_uParticleIdx];
                            
                            m_iColorUpdated     = 0;
                            m_bIsSizeUpdated    = false;
                            //当前粒子对应的关键点颜色和大小变化信息
                            tCCKyePointRandom* kyePointRandom = &(m_pKeyPointsRandom[m_uParticleIdx]);
                            
                            for (unsigned int ids = 0;  ids < PARTICLE_KEYPOINT_TOTALL ; ids++) {
                                tCCKeyPoint* keyPoint   = &m_pKeyPoints[ids];
                                //关键点颜色变化信息
                                ccColor4F* keyPointsRanCol  = &kyePointRandom->deltaColor[ids];
                                
                                p->color.r += updateColor(keyPointsRanCol->r, keyPoint->totalPercent, totalPercent, dt, ids);
                                p->color.g += updateColor(keyPointsRanCol->g, keyPoint->totalPercent, totalPercent, dt, ids);
                                p->color.b += updateColor(keyPointsRanCol->b, keyPoint->totalPercent, totalPercent, dt, ids);
                                p->color.a += updateColor(keyPointsRanCol->a, keyPoint->totalPercent, totalPercent, dt, ids);
                                
                                p->size += updateSize(kyePointRandom->deltaSize[ids],
                                                      m_pKeyPoints[ids].totalPercent,
                                                      totalPercent,
                                                      dt,
                                                      ids);
                                
                                if (!compareWithColor(oldColor,p->color) || oldSize != p->size) {
                                    break;
                                }
                            }
                            
                            p->color.r = clampf(p->color.r, 0, 1);
                            p->color.g = clampf(p->color.g, 0, 1);
                            p->color.b = clampf(p->color.b, 0, 1);
                            p->color.a = clampf(p->color.a, 0, 1);
                            
                            p->size = MAX( 0, p->size );
                        }
                        
                        // angle
                        p->rotation += (p->deltaRotation * dt);
                        
                        //
                        // update values in quad
                        //
                        
                        CCPoint	newPos;
                        
                        if( m_ePositionType == kCCPositionTypeFree || m_ePositionType == kCCPositionTypeRelative )
                        {
                            CCPoint diff = ccpSub( currentPosition, p->startPos );
                            newPos = ccpSub(p->pos, diff);
                        } else
                            newPos = p->pos;
                            
                            // translate newPos to correct position, since matrix transform isn't performed in batchnode
                            // don't update the particle with the new position information, it will interfere with the radius and tangential calculations
                            if (m_pBatchNode)
                            {
                                newPos.x += m_obPosition.x;
                                newPos.y += m_obPosition.y;
                            }
                        
						//设置透明度
						ccColor4F origColor4F = p->color;
						if(m_fOpacityScale < 1.0f)
						{
							p->color.r *= m_fOpacityScale;
							p->color.g *= m_fOpacityScale;
							p->color.b *= m_fOpacityScale;
							p->color.a *= m_fOpacityScale;
						}
						//float fOpacity = p->color.a;
						//if(m_fOpacityScale < 1.0f)
						//{
						//	p->color.a *= m_fOpacityScale;
						//}
                        updateQuadWithParticle(p, newPos, m_uParticleIdx);
						//还原初始透明度
						if(m_fOpacityScale < 1.0f)
						{
							//p->color.a = fOpacity;
							p->color = origColor4F;
						}
                        
                        // update particle counter
                        m_uParticleIdx++;
                        
                    } else {
                        // life < 0
                        int currentIndex = p->atlasIndex;
                        //粒子消亡替换
                        if( m_uParticleIdx != m_uParticleCount-1 ){
                            m_pParticleCurrentFrame[m_uParticleIdx]= m_pParticleCurrentFrame[m_uParticleCount-1];
                            m_pParticleLifeTime[m_uParticleIdx] = m_pParticleLifeTime[m_uParticleCount - 1];
                            m_pParticleTextureAngle[m_uParticleIdx] = m_pParticleTextureAngle[m_uParticleCount - 1];
                            m_pRadius[m_uParticleIdx] = m_pRadius[m_uParticleCount-1];
                            for (unsigned int ids = 0; ids < PARTICLE_KEYPOINT_TOTALL; ids++) {
                                m_pKeyPointsRandom[m_uParticleIdx].deltaSize[ids]   = m_pKeyPointsRandom[m_uParticleCount - 1].deltaSize[ids];
                                m_pKeyPointsRandom[m_uParticleIdx].deltaColor[ids]  = m_pKeyPointsRandom[m_uParticleCount - 1].deltaColor[ids];
                            }
                            m_pParticles[m_uParticleIdx] = m_pParticles[m_uParticleCount-1];
                        }
                        
                        if (m_pBatchNode)
                        {                            
                            //disable the switched particle
                            m_pBatchNode->disableParticle(m_uAtlasIndex+currentIndex);
                            
                            //switch indexes
                            m_pParticles[m_uParticleCount-1].atlasIndex = currentIndex;
                        }
                        
                        --m_uParticleCount;
                        
                        if( m_uParticleCount == 0 && m_bIsAutoRemoveOnFinish )
                        {
                            this->unscheduleUpdate();
                            m_pParent->removeChild(this, true);
                            return;
                        }
                    }
                }//while
                m_bTransformSystemDirty = false;
            }
    
	if (!m_pBatchNode){
		postStep();
    }
    
	CC_PROFILER_STOP_CATEGORY(kCCProfilerCategoryParticles , @"CCParticleSystem - update");
}

//设置outline半径
void CCParticleSystemEx::setRadiusX(float r)
{
    CCAssert( m_outlineType == kCCParticleOutlineRadius, "Particle outline type should be Circle");
    _outline.circle.radiusX = r;
}
float CCParticleSystemEx::getRadiusX()
{
    CCAssert( m_outlineType == kCCParticleOutlineRadius, "Particle outline type should be Circle");
	return _outline.circle.radiusX;
}

void CCParticleSystemEx::setRadiusXVar(float r)
{
    CCAssert( m_outlineType == kCCParticleOutlineRadius, "Particle outline type should be Circle");
    _outline.circle.radiusXVar = r;
}
float CCParticleSystemEx::getRadiusXVar()
{
    CCAssert( m_outlineType == kCCParticleOutlineRadius, "Particle outline type should be Circle");
    return _outline.circle.radiusXVar;
}

void CCParticleSystemEx::setRadiusY(float r)
{
    CCAssert( m_outlineType == kCCParticleOutlineRadius, "Particle outline type should be Circle");
    _outline.circle.radiusY = r;
}
float CCParticleSystemEx::getRadiusY()
{
    CCAssert( m_outlineType == kCCParticleOutlineRadius, "Particle outline type should be Circle");
    return _outline.circle.radiusY;
}
void CCParticleSystemEx::setRadiusYVar(float r)
{
    CCAssert( m_outlineType == kCCParticleOutlineRadius, "Particle outline type should be Circle");
    _outline.circle.radiusYVar = r;
}
float CCParticleSystemEx::getRadiusYVar()
{
    CCAssert( m_outlineType == kCCParticleOutlineRadius, "Particle outline type should be Circle");
    return _outline.circle.radiusYVar;
}

//keyPointA关键点数据设置
void CCParticleSystemEx::setKeyPointASize(float keyPointASize)
{
	m_pKeyPoints[0].size = keyPointASize;
}
float CCParticleSystemEx::getKeyPointASize()
{
	return m_pKeyPoints[0].size;
}

void CCParticleSystemEx::setKeyPointASizeVar(float size)
{
	m_pKeyPoints[0].sizeVar = size;
}
float CCParticleSystemEx::getKeyPointASizeVar()
{
	return m_pKeyPoints[0].sizeVar;
}

void CCParticleSystemEx::setKeyPointAColor(ccColor4F keyPointAColor)
{
	m_pKeyPoints[0].color = keyPointAColor;
}
ccColor4F CCParticleSystemEx::getKeyPointAColor()
{
	return m_pKeyPoints[0].color;
}
void CCParticleSystemEx::setKeyPointAColorVar(ccColor4F keyPointAColorVar)
{
	m_pKeyPoints[0].colorVar = keyPointAColorVar;
}
ccColor4F CCParticleSystemEx::getKeyPointAColorVar()
{
	return m_pKeyPoints[0].colorVar;
}

void CCParticleSystemEx::setKeyPointALifePercent(float keyPointALifePercent)
{
	m_pKeyPoints[0].lifePercent = keyPointALifePercent;
}
float CCParticleSystemEx::getKeyPointALifePercent()
{
	return m_pKeyPoints[0].lifePercent;
}
//keyPointB关键点数据设置
void CCParticleSystemEx::setKeyPointBSize(float keyPointBSize)
{
	m_pKeyPoints[1].size = keyPointBSize;
}
float CCParticleSystemEx::getKeyPointBSize()
{
	return m_pKeyPoints[1].size;
}
void CCParticleSystemEx::setKeyPointBSizeVar(float keyPointBSizeVar)
{
	m_pKeyPoints[1].sizeVar = keyPointBSizeVar;
}
float CCParticleSystemEx::getKeyPointBSizeVar()
{
	return m_pKeyPoints[1].sizeVar;
}

void CCParticleSystemEx::setKeyPointBColor(ccColor4F keyPointBColor)
{
	m_pKeyPoints[1].color = keyPointBColor;
}
ccColor4F CCParticleSystemEx::getKeyPointBColor()
{
	return m_pKeyPoints[1].color;
}
void CCParticleSystemEx::setKeyPointBColorVar(ccColor4F keyPointBColorVar)
{
	m_pKeyPoints[1].colorVar = keyPointBColorVar;
}
ccColor4F CCParticleSystemEx::getKeyPointBColorVar()
{
	return m_pKeyPoints[1].colorVar;
}

void CCParticleSystemEx::setKeyPointBLifePercent(float keyPointBLifePercent)
{
	m_pKeyPoints[1].lifePercent = keyPointBLifePercent;
}
float CCParticleSystemEx::getKeyPointBLifePercent()
{
	return m_pKeyPoints[1].lifePercent;
}
//keyPointC关键点数据设置
void CCParticleSystemEx::setKeyPointCSize(float keyPointCSize)
{
	m_pKeyPoints[2].size = keyPointCSize;
}
float CCParticleSystemEx::getKeyPointCSize()
{
	return m_pKeyPoints[2].size;
}
void CCParticleSystemEx::setKeyPointCSizeVar(float keyPointCSizeVar)
{
	m_pKeyPoints[2].sizeVar = keyPointCSizeVar;
}
float CCParticleSystemEx::getKeyPointCSizeVar()
{
	return m_pKeyPoints[2].sizeVar;
}

void CCParticleSystemEx::setKeyPointCColor(ccColor4F keyPointCColor)
{
	m_pKeyPoints[2].color = keyPointCColor;
}
ccColor4F CCParticleSystemEx::getKeyPointCColor()
{
	return m_pKeyPoints[2].color;
}
void CCParticleSystemEx::setKeyPointCColorVar(ccColor4F keyPointCColorVar)
{
	m_pKeyPoints[2].colorVar = keyPointCColorVar;
}
ccColor4F CCParticleSystemEx::getKeyPointCColorVar()
{
	return m_pKeyPoints[2].colorVar;
}

void CCParticleSystemEx::setKeyPointCLifePercent(float keyPointCLifePercent)
{
	m_pKeyPoints[2].lifePercent = keyPointCLifePercent;
}
float CCParticleSystemEx::getKeyPointCLifePercent()
{
	return m_pKeyPoints[2].lifePercent;
}
//keyPointD关键点数据设置
void CCParticleSystemEx::setKeyPointDSize(float keyPointDSize)
{
	m_pKeyPoints[3].size = keyPointDSize;
}
float CCParticleSystemEx::getKeyPointDSize()
{
	return m_pKeyPoints[3].size;
}
void CCParticleSystemEx::setKeyPointDSizeVar(float keyPointDSizeVar)
{
	m_pKeyPoints[3].sizeVar = keyPointDSizeVar;
}
float CCParticleSystemEx::getKeyPointDSizeVar()
{
	return m_pKeyPoints[3].sizeVar;
}

void CCParticleSystemEx::setKeyPointDColor(ccColor4F keyPointDColor)
{
	m_pKeyPoints[3].color = keyPointDColor;
}
ccColor4F CCParticleSystemEx::getKeyPointDColor()
{
	return m_pKeyPoints[3].color;
}
void CCParticleSystemEx::setKeyPointDColorVar(ccColor4F keyPointDColorVar)
{
	m_pKeyPoints[3].colorVar = keyPointDColorVar;
}
ccColor4F CCParticleSystemEx::getKeyPointDColorVar()
{
	return m_pKeyPoints[3].colorVar;
}

void CCParticleSystemEx::setKeyPointDLifePercent(float keyPointDLifePercent)
{
	m_pKeyPoints[3].lifePercent = keyPointDLifePercent;
}
float CCParticleSystemEx::getKeyPointDLifePercent()
{
	return m_pKeyPoints[3].lifePercent;
}
//keyPointE关键点数据设置
void CCParticleSystemEx::setKeyPointESize(float keyPointESize)
{
	m_pKeyPoints[4].size = keyPointESize;
}
float CCParticleSystemEx::getKeyPointESize()
{
	return m_pKeyPoints[4].size;
}
void CCParticleSystemEx::setKeyPointESizeVar(float keyPointESizeVar)
{
	m_pKeyPoints[4].sizeVar = keyPointESizeVar;
}
float CCParticleSystemEx::getKeyPointESizeVar()
{
	return m_pKeyPoints[4].sizeVar;
}

void CCParticleSystemEx::setKeyPointEColor(ccColor4F keyPointEColor)
{
	m_pKeyPoints[4].color = keyPointEColor;
}
ccColor4F CCParticleSystemEx::getKeyPointEColor()
{
	return m_pKeyPoints[4].color;
}
void CCParticleSystemEx::setKeyPointEColorVar(ccColor4F keyPointEColorVar)
{
	m_pKeyPoints[4].colorVar = keyPointEColorVar;
}
ccColor4F CCParticleSystemEx::getKeyPointEColorVar()
{
	return m_pKeyPoints[4].colorVar;
}
/**设置贴图在x轴的切分值（即在x轴上平均切分成多少块）*/
void CCParticleSystemEx::setTextureCarveUpX(int textureCarveUpX)
{
    if (m_particleTextureFrame.carveUpX == textureCarveUpX){
        return;
    }
    
    m_particleTextureFrame.carveUpX = textureCarveUpX;
    
    this->textureCarveUp();
}
int CCParticleSystemEx::getTextureCarveUpX()
{
    return m_particleTextureFrame.carveUpX;
}

/**设置贴图在y轴的切分值（即在y轴上平均切分成多少块）*/
void CCParticleSystemEx::setTextureCarveUpY(int textureCarveUpY)
{
    if (m_particleTextureFrame.carveUpY == textureCarveUpY){
        return;
    }
    
    m_particleTextureFrame.carveUpY = textureCarveUpY;
    
    this->textureCarveUp();
}
int CCParticleSystemEx::getTextureCarveUpY()
{
    return m_particleTextureFrame.carveUpY;
}
//贴图序列图切分计算
void CCParticleSystemEx::textureCarveUp()
{
    if (!m_pTexture || m_particleTextureFrame.carveUpX <= 0 || m_particleTextureFrame.carveUpY <= 0){
        return;
    }
    
    float _w = (float)(m_pTexture->getPixelsWide()/m_particleTextureFrame.carveUpX);
    float _h = (float)(m_pTexture->getPixelsHigh()/m_particleTextureFrame.carveUpY);
    
    m_particleTextureFrame.frameSize = CCSizeMake(_w, _h);
    
    m_particleTextureFrame.totalFrame = m_particleTextureFrame.carveUpX * m_particleTextureFrame.carveUpY;
    
    m_particleTextureFrame.frameRectMap = (CCPoint*)calloc(m_particleTextureFrame.totalFrame, sizeof(CCPoint));
    
    CCPoint _point;
    for (int ids = 0; ids<m_particleTextureFrame.totalFrame ; ids++) {
        _point.x = (float)(ids%m_particleTextureFrame.carveUpX);
        _point.y = ceilf((float)(ids/m_particleTextureFrame.carveUpX));
        m_particleTextureFrame.frameRectMap[ids] = _point;
        //CCLog("ids=%d  _point.x=%f  _point.y=%f",ids,_point.x,_point.y);
    }
}

//比较颜色是否相同
bool CCParticleSystemEx::compareWithColor(ccColor4F a, ccColor4F b)
{
    return ((a.r == b.r) + (a.r == b.r) + (a.g == b.g) + (a.b == b.b)) == 4;
}
//更新粒子颜色
float CCParticleSystemEx::updateColor(float color, float percent, float tpercent, float dt, int ids)
{
    if(tpercent > percent || (m_iColorUpdated == 4)){
        return 0;
    }
    
    m_iColorUpdated ++;
    
    float rfc = color*dt;
    
    return rfc;
}
//更新粒子大小
float CCParticleSystemEx::updateSize(float size, float percent, float tpercent, float dt, int ids)
{
    if(tpercent > percent || m_bIsSizeUpdated){
        return 0;
    }
    
    m_bIsSizeUpdated = true;
    
    GLfloat rfc = size*dt;
    
    return rfc;
}
//获取粒子生命周期
float CCParticleSystemEx::getParticleElapsedTime(int index)
{
    return m_pParticleLifeTime[index];
}

void CCParticleSystemEx::updateQuadWithParticle(tCCParticle* particle, const CCPoint& newPosition, int index)
{
    CC_UNUSED_PARAM(particle);
    CC_UNUSED_PARAM(newPosition);
    CC_UNUSED_PARAM(index);
	// should be overriden
}
//设置帧时间
void CCParticleSystemEx::setFrameTime(const char *time)
{
    int strSize = strlen(time);
    
	m_strFrameTime = time;
// 	if (m_pFrameTime != time)
// 	{
// 		//CC_SAFE_DELETE_ARRAY(m_pFrameTime);
// 		memcpy(&m_pFrameTime, time, strSize);
// 	}
    
    int ids = 0;
    const char* str = ",";
    char* timeStr = new char[strSize + 1];
    double* timeArray = (double*)calloc(strSize, sizeof(double));
    memcpy(timeStr, time, strSize);
	timeStr[strSize] = '\0';

    
    char* strArray = strtok(timeStr, str);
    while (strArray) {
        timeArray[ids] = atof(strArray);
        ids++;
        strArray = strtok(NULL, str);
    }
    
    m_totalFrameId = ids - 1;
    
    
    size_t timeToFrameSize = (m_totalFrameId + 1) * sizeof(double);
    double* timeToFrameNew = (double*)realloc(m_pTimeToFrame, timeToFrameSize);
    if (timeToFrameNew) {
        m_pTimeToFrame = timeToFrameNew;
        memset(m_pTimeToFrame, 0, timeToFrameSize);
    }
    
    double _time = 0;
    for (unsigned int ids = 0; ids <= m_totalFrameId; ids++) {
        _time += timeArray[ids];
        m_pTimeToFrame[ids] = _time;
    }
    
    m_earchFrameTime = m_pTimeToFrame[0];
    
    //CC_SAFE_DELETE_ARRAY(str);
    CC_SAFE_DELETE_ARRAY(timeStr);
    //CC_SAFE_FREE(strArray);
    CC_SAFE_FREE(timeArray);
}
//根据当前时间获取当前帧
int CCParticleSystemEx::getFrameId(double time)
{
    int frameId = 0;
    for (unsigned int ids = m_totalFrameId;ids > -1;ids--)
    {
        if(time > m_pTimeToFrame[ids]){
            frameId = ids + 1;
            break;
        }
    }
    
    return frameId;
}
//重写设置visible
void CCParticleSystemEx::setVisible(bool var)
{

	if(m_bVisible == var)
	{
		return;
	}

    if (var)
    {
        resetSystem();
    }else
    {
        stopSystem();
    }
    
    CCParticleSystem::setVisible(var);
}

NS_CC_END