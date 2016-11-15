//
//  CCParticleSystemQuadEx.cpp
//  cocos2dx
//
//  Created by Archer on 14/2/12.
//  Copyright (c) 2014年 cocos2d-x. All rights reserved.
//

#include "CCParticleSystemQuadEx.h"
#include "CCGL.h"
#include "sprite_nodes/CCSpriteFrame.h"
#include "CCDirector.h"
#include "CCParticleBatchNode.h"
#include "textures/CCTextureAtlas.h"
#include "shaders/CCShaderCache.h"
#include "shaders/ccGLStateCache.h"
#include "shaders/CCGLProgram.h"
#include "support/TransformUtils.h"
#include "support/CCNotificationCenter.h"
#include "CCEventType.h"

// extern
#include "kazmath/GL/matrix.h"
#include "support/CCProfiling.h"

NS_CC_BEGIN

//implementation CCParticleSystemQuadEx
// overriding the init method
bool CCParticleSystemQuadEx::initWithTotalParticles(unsigned int numberOfParticles)
{
    // base initialization
    if( CCParticleSystemEx::initWithTotalParticles(numberOfParticles) )
    {
        // allocating data space
        if( ! this->allocMemory() ) {
            this->release();
            return false;
        }
        
        initIndices();
#if CC_TEXTURE_ATLAS_USE_VAO
        setupVBOandVAO();
#else
        setupVBO();
#endif
        
        setShaderProgram(CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor));
        
        
#if CC_ENABLE_CACHE_TEXTURE_DATA
        // Need to listen the event only when not use batchnode, because it will use VBO
        CCNotificationCenter::sharedNotificationCenter()->addObserver(this,
                                                                      callfuncO_selector(CCParticleSystemQuadEx::listenBackToForeground),
                                                                      EVENT_COME_TO_FOREGROUND,
                                                                      NULL);
#endif
        
        return true;
    }
    return false;
}

CCParticleSystemQuadEx::CCParticleSystemQuadEx()
:m_pQuads(NULL)
,m_pIndices(NULL)
#if CC_TEXTURE_ATLAS_USE_VAO
,m_uVAOname(0)
#endif
{
    memset(m_pBuffersVBO, 0, sizeof(m_pBuffersVBO));
}

CCParticleSystemQuadEx::~CCParticleSystemQuadEx()
{
    if (NULL == m_pBatchNode)
    {
        CC_SAFE_FREE(m_pQuads);
        CC_SAFE_FREE(m_pIndices);
        glDeleteBuffers(2, &m_pBuffersVBO[0]);
#if CC_TEXTURE_ATLAS_USE_VAO
        glDeleteVertexArrays(1, &m_uVAOname);
        ccGLBindVAO(0);
#endif
    }
    
#if CC_ENABLE_CACHE_TEXTURE_DATA
    CCNotificationCenter::sharedNotificationCenter()->removeObserver(this, EVENT_COME_TO_FOREGROUND);
#endif
}

// implementation CCParticleSystemQuadEx

CCParticleSystemQuadEx * CCParticleSystemQuadEx::create(const char *plistFile)
{
    CCParticleSystemQuadEx *pRet = new CCParticleSystemQuadEx();
    if (pRet && pRet->initWithFile(plistFile))
    {
        pRet->autorelease();
        return pRet;
    }
    CC_SAFE_DELETE(pRet);
    return pRet;
}

CCParticleSystemQuadEx * CCParticleSystemQuadEx::createWithTotalParticles(unsigned int numberOfParticles) {
    CCParticleSystemQuadEx *pRet = new CCParticleSystemQuadEx();
    if (pRet && pRet->initWithTotalParticles(numberOfParticles))
    {
        pRet->autorelease();
        return pRet;
    }
    CC_SAFE_DELETE(pRet);
    return pRet;
}


// pointRect should be in Texture coordinates, not pixel coordinates
void CCParticleSystemQuadEx::initTexCoordsWithRect(const CCRect& pointRect)
{
    // convert to Tex coords
    
    CCRect rect = CCRectMake(
                             pointRect.origin.x * CC_CONTENT_SCALE_FACTOR(),
                             pointRect.origin.y * CC_CONTENT_SCALE_FACTOR(),
                             pointRect.size.width * CC_CONTENT_SCALE_FACTOR(),
                             pointRect.size.height * CC_CONTENT_SCALE_FACTOR());
    
    m_textureWidth = (GLfloat) pointRect.size.width;
    m_textureHeight = (GLfloat) pointRect.size.height;
    
    if (m_pTexture)
    {
        m_textureWidth = (GLfloat)m_pTexture->getPixelsWide();
        m_textureHeight = (GLfloat)m_pTexture->getPixelsHigh();
    }
    
#if CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL
    GLfloat left = (rect.origin.x*2+1) / (m_textureWidth*2);
    GLfloat bottom = (rect.origin.y*2+1) / (m_textureHeight*2);
    GLfloat right = left + (rect.size.width*2-2) / (m_textureWidth*2);
    GLfloat top = bottom + (rect.size.height*2-2) / (m_textureHeight*2);
#else
    GLfloat left = rect.origin.x / m_textureWidth;
    GLfloat bottom = rect.origin.y / m_textureHeight;
    GLfloat right = left + rect.size.width / m_textureWidth;
    GLfloat top = bottom + rect.size.height / m_textureHeight;
#endif // ! CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL
    
    // Important. Texture in cocos2d are inverted, so the Y component should be inverted
    CC_SWAP( top, bottom, float);
    
    ccV3F_C4B_T2F_Quad *quads = NULL;
    unsigned int start = 0, end = 0;
    if (m_pBatchNode)
    {
        quads = m_pBatchNode->getTextureAtlas()->getQuads();
        start = m_uAtlasIndex;
        end = m_uAtlasIndex + m_uTotalParticles;
    }
    else
    {
        quads = m_pQuads;
        start = 0;
        end = m_uTotalParticles;
    }
    
    for(unsigned int i=start; i<end; i++)
    {
        // bottom-left vertex:
        quads[i].bl.texCoords.u = left;
        quads[i].bl.texCoords.v = bottom;
        // bottom-right vertex:
        quads[i].br.texCoords.u = right;
        quads[i].br.texCoords.v = bottom;
        // top-left vertex:
        quads[i].tl.texCoords.u = left;
        quads[i].tl.texCoords.v = top;
        // top-right vertex:
        quads[i].tr.texCoords.u = right;
        quads[i].tr.texCoords.v = top;
    }
}
void CCParticleSystemQuadEx::setTextureWithRect(CCTexture2D *texture, const CCRect& rect)
{
    // Only update the texture if is different from the current one
    if( !m_pTexture || texture->getName() != m_pTexture->getName() )
    {
        CCParticleSystemEx::setTexture(texture);
    }
    
    this->initTexCoordsWithRect(rect);
}
void CCParticleSystemQuadEx::setTexture(CCTexture2D* texture)
{
    const CCSize& s = texture->getContentSize();
    this->setTextureWithRect(texture, CCRectMake(0, 0, s.width, s.height));
}
void CCParticleSystemQuadEx::setDisplayFrame(CCSpriteFrame *spriteFrame)
{
    CCAssert(spriteFrame->getOffsetInPixels().equals(CCPointZero),
             "QuadParticle only supports SpriteFrames with no offsets");
    
    // update texture before updating texture rect
    if ( !m_pTexture || spriteFrame->getTexture()->getName() != m_pTexture->getName())
    {
        this->setTexture(spriteFrame->getTexture());
    }
}

void CCParticleSystemQuadEx::initIndices()
{
    for(unsigned int i = 0; i < m_uTotalParticles; ++i)
    {
        const unsigned int i6 = i*6;
        const unsigned int i4 = i*4;
        m_pIndices[i6+0] = (GLushort) i4+0;
        m_pIndices[i6+1] = (GLushort) i4+1;
        m_pIndices[i6+2] = (GLushort) i4+2;
        
        m_pIndices[i6+5] = (GLushort) i4+1;
        m_pIndices[i6+4] = (GLushort) i4+2;
        m_pIndices[i6+3] = (GLushort) i4+3;
    }
}
void CCParticleSystemQuadEx::updateQuadWithParticle(tCCParticle* particle, const CCPoint& newPosition, int index)
{
    ccV3F_C4B_T2F_Quad *quad;
    
    if (m_pBatchNode)
    {
        ccV3F_C4B_T2F_Quad *batchQuads = m_pBatchNode->getTextureAtlas()->getQuads();
        quad = &(batchQuads[m_uAtlasIndex+particle->atlasIndex]);
    }
    else
    {
        quad = &(m_pQuads[m_uParticleIdx]);
    }
    ccColor4B color = (m_bOpacityModifyRGB)
    ? ccc4( (GLubyte)(particle->color.r*particle->color.a*255), (GLubyte)(particle->color.g*particle->color.a*255), (GLubyte)(particle->color.b*particle->color.a*255), (GLubyte)(particle->color.a*255))
    : ccc4( (GLubyte)(particle->color.r*255), (GLubyte)(particle->color.g*255), (GLubyte)(particle->color.b*255), (GLubyte)(particle->color.a*255));
    
    quad->bl.colors = color;
    quad->br.colors = color;
    quad->tl.colors = color;
    quad->tr.colors = color;
    
    // vertices
    GLfloat size_x = (m_particleTextureFrame.frameSize.width/2)*(particle->size/100)*m_textureScaleX;
    GLfloat size_y = (m_particleTextureFrame.frameSize.height/2)*(particle->size/100)*m_textureScaleY;
    
    GLfloat x1 = -size_x;
    GLfloat y1 = -size_y;
    
    GLfloat x2 = size_x;
    GLfloat y2 = size_y;
    GLfloat x = newPosition.x;
    GLfloat y = newPosition.y;
    
    GLfloat r = 0.0 ;

    if (m_textureAngleType == kCCRandomAngle) {
        r = (GLfloat)- CC_DEGREES_TO_RADIANS(particle->rotation) - CC_DEGREES_TO_RADIANS(360 * CCRANDOM_0_1());
    } else {
        r = (GLfloat)- CC_DEGREES_TO_RADIANS(particle->rotation) - m_pParticleTextureAngle[index];
    }

    GLfloat cr = cosf(r);
    GLfloat sr = sinf(r);
    GLfloat ax = x1 * cr - y1 * sr + x;
    GLfloat ay = x1 * sr + y1 * cr + y;
    GLfloat bx = x2 * cr - y1 * sr + x;
    GLfloat by = x2 * sr + y1 * cr + y;
    GLfloat cx = x2 * cr - y2 * sr + x;
    GLfloat cy = x2 * sr + y2 * cr + y;
    GLfloat dx = x1 * cr - y2 * sr + x;
    GLfloat dy = x1 * sr + y2 * cr + y;
    
    // bottom-left
    quad->bl.vertices.x = ax;
    quad->bl.vertices.y = ay;
    
    // bottom-right vertex:
    quad->br.vertices.x = bx;
    quad->br.vertices.y = by;
    
    // top-left vertex:
    quad->tl.vertices.x = dx;
    quad->tl.vertices.y = dy;
    
    // top-right vertex:
    quad->tr.vertices.x = cx;
    quad->tr.vertices.y = cy;
    
    setTextureFrameRect(quad, particle, index);
}
//设置粒子序列帧
void CCParticleSystemQuadEx::setTextureFrameRect(ccV3F_C4B_T2F_Quad* quad, tCCParticle* p, int index)
{
    int frameId = 0;
    
    if (m_particleFrameType == kCCParticleLoopFrame || m_particleFrameType == kCCParticleLoopFrameRollback) {
        if (m_totalFrameId == 0) {
            frameId = (int)(floor((this->getParticleElapsedTime(index) - p->timeToLive)/m_earchFrameTime))%m_particleTextureFrame.totalFrame;
        }else{
            //由于浮点数不能使用%运算，因此先将浮点放大为NSInteger类型，再使用%运算符。
            int particleElapsedTime = (int)((this->getParticleElapsedTime(index) - p->timeToLive) * 100000);
            int particleTotalFrameTime = (int)(m_pTimeToFrame[m_totalFrameId] * 100000);
            double particleNowTime = (particleElapsedTime%particleTotalFrameTime) * 0.00001;
            frameId = this->getFrameId(particleNowTime);
        }
        if (m_particleFrameType == kCCParticleLoopFrameRollback) {
            frameId = m_particleTextureFrame.totalFrame - frameId - 1;
        }
    } else if(m_particleFrameType == kCCParticlePlayFrame || m_particleFrameType == kCCParticlePlayFrameRollback){
        if(m_totalFrameId == 0){
            frameId = frameId = (int)(floor((this->getParticleElapsedTime(index) - p->timeToLive)/m_earchFrameTime));
        }else{
            frameId =this->getFrameId((this->getParticleElapsedTime(index) - p->timeToLive));
        }
        if (m_particleFrameType == kCCParticlePlayFrameRollback) {
            frameId = m_particleTextureFrame.totalFrame - frameId - 1;
        }
    } else if(m_particleFrameType == kCCParticleRandomFrame){
        frameId = (int)(m_pParticleCurrentFrame[index]);
    } else if(m_particleFrameType == kCCParticleToFrame){
        frameId = m_toFrame;
    }
    
//    CCLog("frameId=%d  m_particleTextureFrame.totalFrame=%f p->timeToLive=%f",frameId,this->getParticleElapsedTime(index),p->timeToLive);
    
    if (frameId < 0 || frameId >= m_particleTextureFrame.totalFrame){
        return;
    }
    
    CCRect rect = CCRectMake(m_particleTextureFrame.frameSize.width * m_particleTextureFrame.frameRectMap[frameId].x,
                             m_particleTextureFrame.frameSize.height* m_particleTextureFrame.frameRectMap[frameId].y,
                             m_particleTextureFrame.frameSize.width,
                             m_particleTextureFrame.frameSize.height);
    
#if CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL
    GLfloat left = (rect.origin.x*2+1) / (m_textureWidth*2);
    GLfloat bottom = (rect.origin.y*2+1) / (m_textureHeight*2);
    GLfloat right = left + (rect.size.width*2-2) / (m_textureWidth*2);
    GLfloat top = bottom + (rect.size.height*2-2) / (m_textureHeight*2);
#else
    GLfloat left = rect.origin.x / m_textureWidth;
    GLfloat bottom = rect.origin.y / m_textureHeight;
    GLfloat right = left + rect.size.width / m_textureWidth;
    GLfloat top = bottom + rect.size.height / m_textureHeight;
#endif // ! CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL
    
    // Important. Texture in cocos2d are inverted, so the Y component should be inverted
    CC_SWAP( top, bottom, float);
    
    ccV3F_C4B_T2F_Quad *quads = NULL;
    unsigned int start = 0, end = 0;
    if (m_pBatchNode)
    {
        quads = m_pBatchNode->getTextureAtlas()->getQuads();
        start = m_uAtlasIndex;
        end = m_uAtlasIndex + m_uTotalParticles;
    }
    else
    {
        quads = m_pQuads;
        start = 0;
        end = m_uTotalParticles;
    }
    
    // bottom-left vertex: 左下
    quad->bl.texCoords.u = left;
    quad->bl.texCoords.v = bottom;
    // bottom-right vertex: 右下
    quad->br.texCoords.u = right;
    quad->br.texCoords.v = bottom;
    // top-right vertex: 右上
    quad->tr.texCoords.u = right;
    quad->tr.texCoords.v = top;
    // top-left vertex: 左上
    quad->tl.texCoords.u = left;
    quad->tl.texCoords.v = top;
}

void CCParticleSystemQuadEx::postStep()
{
	CC_PROFILER_HELPER;
	if (!isVisible())
	{
		return;
	}
    glBindBuffer(GL_ARRAY_BUFFER, m_pBuffersVBO[0]);
	
	// Option 1: Sub Data
    //glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(m_pQuads[0])*m_uTotalParticles, m_pQuads);
	

	glBufferData(GL_ARRAY_BUFFER, sizeof(m_pQuads[0])*m_uTotalParticles, m_pQuads, GL_DYNAMIC_DRAW);

	// Option 2: Data
    //	glBufferData(GL_ARRAY_BUFFER, sizeof(quads_[0]) * particleCount, quads_, GL_DYNAMIC_DRAW);
	
	// Option 3: Orphaning + glMapBuffer
	// glBufferData(GL_ARRAY_BUFFER, sizeof(m_pQuads[0])*m_uTotalParticles, NULL, GL_STREAM_DRAW);
	// void *buf = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	// memcpy(buf, m_pQuads, sizeof(m_pQuads[0])*m_uTotalParticles);
	// glUnmapBuffer(GL_ARRAY_BUFFER);
    
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    
	CHECK_GL_ERROR_DEBUG();
}

// overriding draw method
void CCParticleSystemQuadEx::draw()
{
	CC_PROFILER_HELPER;
	// 待渲染图元数目为0，无需渲染
	if (m_uParticleIdx == 0)
	{
		return;
	}

    CCAssert(!m_pBatchNode,"draw should not be called when added to a particleBatchNode");
    
	CCDirector::sharedDirector()->flushDraw();

    CC_NODE_DRAW_SETUP();
    
    ccGLBindTexture2D( m_pTexture->getName() );
    ccGLBlendFunc( m_tBlendFunc.src, m_tBlendFunc.dst );
    
    CCAssert( m_uParticleIdx == m_uParticleCount, "Abnormal error in particle quad");
    
#if CC_TEXTURE_ATLAS_USE_VAO
    //
    // Using VBO and VAO
    //
    ccGLBindVAO(m_uVAOname);
    
#if CC_REBIND_INDICES_BUFFER
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_pBuffersVBO[1]);
#endif
    
    glDrawElements(GL_TRIANGLES, (GLsizei) m_uParticleIdx*6, GL_UNSIGNED_SHORT, 0);
    
#if CC_REBIND_INDICES_BUFFER
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
    
#else
    //
    // Using VBO without VAO
    //
    
#define kQuadSize sizeof(m_pQuads[0].bl)
    
    ccGLEnableVertexAttribs( kCCVertexAttribFlag_PosColorTex );
    
    glBindBuffer(GL_ARRAY_BUFFER, m_pBuffersVBO[0]);
    // vertices
    glVertexAttribPointer(kCCVertexAttrib_Position, 3, GL_FLOAT, GL_FALSE, kQuadSize, (GLvoid*) offsetof( ccV3F_C4B_T2F, vertices));
    // colors
    glVertexAttribPointer(kCCVertexAttrib_Color, 4, GL_UNSIGNED_BYTE, GL_TRUE, kQuadSize, (GLvoid*) offsetof( ccV3F_C4B_T2F, colors));
    // tex coords
    glVertexAttribPointer(kCCVertexAttrib_TexCoords, 2, GL_FLOAT, GL_FALSE, kQuadSize, (GLvoid*) offsetof( ccV3F_C4B_T2F, texCoords));
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_pBuffersVBO[1]);
    
    glDrawElements(GL_TRIANGLES, (GLsizei) m_uParticleIdx*6, GL_UNSIGNED_SHORT, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
#endif
    
    CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, m_uParticleIdx*6);

#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	CCDirector::sharedDirector()->addDrawTextureIDToVec(m_pTexture->getName());
#endif
    CHECK_GL_ERROR_DEBUG();
}

void CCParticleSystemQuadEx::setTotalParticles(unsigned int tp)
{
    // If we are setting the total number of particles to a number higher
    // than what is allocated, we need to allocate new arrays
    if( tp > m_uAllocatedParticles )
    {
        // Allocate new memory
        size_t particlesSize        = tp * sizeof(tCCParticle);
        size_t quadsSize            = tp * sizeof(m_pQuads[0]) * 1;
        size_t indicesSize          = tp * sizeof(m_pIndices[0]) * 6 * 1;
        size_t lifeTimeSize         = tp * sizeof(float);
        size_t currentFrameSize     = tp * sizeof(double);
        size_t textureAngleSize     = tp * sizeof(float);
        size_t radiusSize           = tp * sizeof(tCCRadius);
        size_t keyPointRandomSize   = tp * sizeof(tCCKyePointRandom) * PARTICLE_KEYPOINT_TOTALL;
        
        tCCParticle*        particlesNew            = (tCCParticle*)realloc(m_pParticles, particlesSize);
        ccV3F_C4B_T2F_Quad* quadsNew                = (ccV3F_C4B_T2F_Quad*)realloc(m_pQuads, quadsSize);
        GLushort*           indicesNew              = (GLushort*)realloc(m_pIndices, indicesSize);
        float*              particleLifeTimeNew     = (float*)realloc(m_pParticleLifeTime, lifeTimeSize);
        double*             particleCurrentFrameNew = (double*)realloc(m_pParticleCurrentFrame, currentFrameSize);
        float*              particleTextureAngleNew = (float*)realloc(m_pParticleTextureAngle, textureAngleSize);
        tCCRadius*          radiusNew               = (tCCRadius*)realloc(m_pRadius, radiusSize);
        tCCKyePointRandom*  keyPointsRandomNew      = (tCCKyePointRandom*)realloc(m_pKeyPointsRandom, keyPointRandomSize);
        for (unsigned int i = 0; i < tp; i++) {
			if (i >= m_uAllocatedParticles)
			{
				keyPointsRandomNew[i].deltaSize   = (float*)calloc( PARTICLE_KEYPOINT_TOTALL, sizeof(float) );
				keyPointsRandomNew[i].deltaColor  = (ccColor4F*)calloc( PARTICLE_KEYPOINT_TOTALL, sizeof(ccColor4F) );
			}
            for (unsigned j = 0; j < PARTICLE_KEYPOINT_TOTALL; j++) {
                keyPointsRandomNew[i].deltaSize[j]      = 0;
                keyPointsRandomNew[i].deltaColor[j].r   = 0;
                keyPointsRandomNew[i].deltaColor[j].g   = 0;
                keyPointsRandomNew[i].deltaColor[j].b   = 0;
                keyPointsRandomNew[i].deltaColor[j].a   = 0;
            }
        }
        
        if (particlesNew && quadsNew && indicesNew && particleLifeTimeNew && particleCurrentFrameNew
            && particleTextureAngleNew && radiusNew && keyPointsRandomNew)
        {
            // Assign pointers
            m_pParticles            = particlesNew;
            m_pQuads                = quadsNew;
            m_pIndices              = indicesNew;
            m_pParticleLifeTime     = particleLifeTimeNew;
            m_pParticleCurrentFrame = particleCurrentFrameNew;
            m_pParticleTextureAngle = particleTextureAngleNew;
            m_pRadius               = radiusNew;
            m_pKeyPointsRandom        = keyPointsRandomNew;
            for (unsigned int i = 0; i < tp; i++) {
                m_pKeyPointsRandom[i].deltaSize = keyPointsRandomNew[i].deltaSize;
                m_pKeyPointsRandom[i].deltaColor= keyPointsRandomNew[i].deltaColor;
            }
            
            // Clear the memory
            // XXX: Bug? If the quads are cleared, then drawing doesn't work... WHY??? XXX
            memset(m_pParticles, 0, particlesSize);
            memset(m_pQuads, 0, quadsSize);
            memset(m_pIndices, 0, indicesSize);
            memset(m_pParticleLifeTime, 0, lifeTimeSize);
            memset(m_pParticleCurrentFrame, 0, currentFrameSize);
            memset(m_pParticleTextureAngle, 0, textureAngleSize);
            memset(m_pRadius, 0, radiusSize);
            for (unsigned int ids = 0; ids < tp; ids++) {
                memset(m_pKeyPointsRandom[ids].deltaSize, 0, sizeof(float) * PARTICLE_KEYPOINT_TOTALL);
                memset(m_pKeyPointsRandom[ids].deltaColor, 0, sizeof(ccColor4F) * PARTICLE_KEYPOINT_TOTALL);
            }
            
            m_uAllocatedParticles = tp;
        }
        else
        {
            // Out of memory, failed to resize some array
            if (particlesNew) m_pParticles = particlesNew;
            if (quadsNew) m_pQuads = quadsNew;
            if (indicesNew) m_pIndices = indicesNew;
            
            CCLOG("Particle system: out of memory");
            return;
        }
        
        m_uTotalParticles = tp;
        
        // Init particles
        if (m_pBatchNode)
        {
            for (unsigned int i = 0; i < m_uTotalParticles; i++)
            {
                m_pParticles[i].atlasIndex=i;
            }
        }
        
        initIndices();
#if CC_TEXTURE_ATLAS_USE_VAO
        setupVBOandVAO();
#else
        setupVBO();
#endif
    }
    else
    {
        m_uTotalParticles = tp;
    }
    
    resetSystem();
}

#if CC_TEXTURE_ATLAS_USE_VAO
void CCParticleSystemQuadEx::setupVBOandVAO()
{
    // clean VAO
	if (m_pBuffersVBO[0] != 0)
	{
		glDeleteBuffers(1, &m_pBuffersVBO[0]);
	}
	if (m_pBuffersVBO[1] != 0)
	{
		glDeleteBuffers(1, &m_pBuffersVBO[1]);
	}

	//glDeleteBuffers(2, &m_pBuffersVBO[0]);
    glDeleteVertexArrays(1, &m_uVAOname);
    ccGLBindVAO(0);
    
    glGenVertexArrays(1, &m_uVAOname);
    ccGLBindVAO(m_uVAOname);
    
#define kQuadSize sizeof(m_pQuads[0].bl)
    
    glGenBuffers(2, &m_pBuffersVBO[0]);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_pBuffersVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_pQuads[0]) * m_uTotalParticles, m_pQuads, GL_DYNAMIC_DRAW);
    
    // vertices
    glEnableVertexAttribArray(kCCVertexAttrib_Position);
    glVertexAttribPointer(kCCVertexAttrib_Position, 2, GL_FLOAT, GL_FALSE, kQuadSize, (GLvoid*) offsetof( ccV3F_C4B_T2F, vertices));
    
    // colors
    glEnableVertexAttribArray(kCCVertexAttrib_Color);
    glVertexAttribPointer(kCCVertexAttrib_Color, 4, GL_UNSIGNED_BYTE, GL_TRUE, kQuadSize, (GLvoid*) offsetof( ccV3F_C4B_T2F, colors));
    
    // tex coords
    glEnableVertexAttribArray(kCCVertexAttrib_TexCoords);
    glVertexAttribPointer(kCCVertexAttrib_TexCoords, 2, GL_FLOAT, GL_FALSE, kQuadSize, (GLvoid*) offsetof( ccV3F_C4B_T2F, texCoords));
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_pBuffersVBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_pIndices[0]) * m_uTotalParticles * 6, m_pIndices, GL_STATIC_DRAW);
    
    // Must unbind the VAO before changing the element buffer.
    ccGLBindVAO(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    CHECK_GL_ERROR_DEBUG();
}
#else

void CCParticleSystemQuadEx::setupVBO()
{
	if (m_pBuffersVBO[0] != 0)
	{
		glDeleteBuffers(1, &m_pBuffersVBO[0]);
	}
	if (m_pBuffersVBO[1] != 0)
	{
		glDeleteBuffers(1, &m_pBuffersVBO[1]);
	}
	
    //glDeleteBuffers(2, &m_pBuffersVBO[0]);

    glGenBuffers(2, &m_pBuffersVBO[0]);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_pBuffersVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_pQuads[0]) * m_uTotalParticles, m_pQuads, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_pBuffersVBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_pIndices[0]) * m_uTotalParticles * 6, m_pIndices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    CHECK_GL_ERROR_DEBUG();
}

#endif

void CCParticleSystemQuadEx::listenBackToForeground(CCObject *obj)
{
#if CC_TEXTURE_ATLAS_USE_VAO
    setupVBOandVAO();
#else
    setupVBO();
#endif
}

bool CCParticleSystemQuadEx::allocMemory()
{
    CCAssert( ( !m_pQuads && !m_pIndices), "Memory already alloced");
    CCAssert( !m_pBatchNode, "Memory should not be alloced when not using batchNode");
    
    CC_SAFE_FREE(m_pQuads);
    CC_SAFE_FREE(m_pIndices);
    
    m_pQuads = (ccV3F_C4B_T2F_Quad*)malloc(m_uTotalParticles * sizeof(ccV3F_C4B_T2F_Quad));
    m_pIndices = (GLushort*)malloc(m_uTotalParticles * 6 * sizeof(GLushort));
    
    if( !m_pQuads || !m_pIndices)
    {
        CCLOG("cocos2d: Particle system: not enough memory");
        CC_SAFE_FREE(m_pQuads);
        CC_SAFE_FREE(m_pIndices);
        
        return false;
    }
    
    memset(m_pQuads, 0, m_uTotalParticles * sizeof(ccV3F_C4B_T2F_Quad));
    memset(m_pIndices, 0, m_uTotalParticles * 6 * sizeof(GLushort));
    
    return true;
}

void CCParticleSystemQuadEx::setBatchNode(CCParticleBatchNode * batchNode)
{
    if( m_pBatchNode != batchNode )
    {
        CCParticleBatchNode* oldBatch = m_pBatchNode;
        
        CCParticleSystemEx::setBatchNode(batchNode);
        
        // NEW: is self render ?
        if( ! batchNode )
        {
            allocMemory();
            initIndices();
            setTexture(oldBatch->getTexture());
#if CC_TEXTURE_ATLAS_USE_VAO
            setupVBOandVAO();
#else
            setupVBO();
#endif
        }
        // OLD: was it self render ? cleanup
        else if( !oldBatch )
        {
            // copy current state to batch
            ccV3F_C4B_T2F_Quad *batchQuads = m_pBatchNode->getTextureAtlas()->getQuads();
            ccV3F_C4B_T2F_Quad *quad = &(batchQuads[m_uAtlasIndex] );
            memcpy( quad, m_pQuads, m_uTotalParticles * sizeof(m_pQuads[0]) );
            
            CC_SAFE_FREE(m_pQuads);
            CC_SAFE_FREE(m_pIndices);
            
            glDeleteBuffers(2, &m_pBuffersVBO[0]);
            memset(m_pBuffersVBO, 0, sizeof(m_pBuffersVBO));
#if CC_TEXTURE_ATLAS_USE_VAO
            glDeleteVertexArrays(1, &m_uVAOname);
            ccGLBindVAO(0);
            m_uVAOname = 0;
#endif
        }
    }
}

CCParticleSystemQuadEx * CCParticleSystemQuadEx::create() {
    CCParticleSystemQuadEx *pParticleSystemQuad = new CCParticleSystemQuadEx();
    if (pParticleSystemQuad && pParticleSystemQuad->init())
    {
        pParticleSystemQuad->autorelease();
        return pParticleSystemQuad;
    }
    CC_SAFE_DELETE(pParticleSystemQuad);
    return NULL;
}
//设置延迟时间
void CCParticleSystemQuadEx::setDelayTime(float fTime)
{
    this->stopSystem();
    this->scheduleOnce(schedule_selector(CCParticleSystemQuadEx::delayTimeComplete), fTime);
}
//延迟结束事件
void CCParticleSystemQuadEx::delayTimeComplete(float fTime)
{
	this->resetSystem();
}

NS_CC_END
