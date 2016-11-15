//
//  CCParticleSystemQuadEx.h
//  cocos2dx
//
//  Created by Archer on 14/2/12.
//  Copyright (c) 2014年 cocos2d-x. All rights reserved.
//

#ifndef __CC_PARTICLE_SYSTEM_QUAD_EX_H__
#define __CC_PARTICLE_SYSTEM_QUAD_EX_H__

#include "CCParticleSystemEx.h"

USING_NS_CC;

NS_CC_BEGIN

class CCSpriteFrame;

class CC_DLL CCParticleSystemQuadEx : public CCParticleSystemEx {
protected:
    ccV3F_C4B_T2F_Quad      *m_pQuads;        // quads to be rendered
    GLushort                *m_pIndices;    // indices
    
    GLfloat                 m_textureWidth;
    GLfloat                 m_textureHeight;
    //粒子坐标系类型（0:使用世界坐标系；1:使用父级坐标系；2:使用发射器坐标系）
    //CC_SYNTHESIZE_READONLY(int, m_particlePositionType,ParticlePositionType);
    //延迟发射时间
    CC_SYNTHESIZE_READONLY(int, m_delayTime,DelayTime);
    
#if CC_TEXTURE_ATLAS_USE_VAO
    GLuint                m_uVAOname;
#endif
    
    GLuint                m_pBuffersVBO[2]; //0: vertex  1: indices
public:
    CCParticleSystemQuadEx();
    virtual ~CCParticleSystemQuadEx();
    
    /** creates an initializes a CCParticleSystemQuad from a plist file.
     This plist files can be created manually or with Particle Designer:
     */
    static CCParticleSystemQuadEx * create(const char *plistFile);
    
    /** initializes the indices for the vertices*/
    void initIndices();
    
    /** initializes the texture with a rectangle measured Points */
    void initTexCoordsWithRect(const CCRect& rect);
    
    /** Sets a new CCSpriteFrame as particle.
     WARNING: this method is experimental. Use setTexture:withRect instead.
     @since v0.99.4
     */
    void setDisplayFrame(CCSpriteFrame *spriteFrame);
    
    /** Sets a new texture with a rect. The rect is in Points.
     @since v0.99.4
     */
    void setTextureWithRect(CCTexture2D *texture, const CCRect& rect);
    // super methods
    virtual bool initWithTotalParticles(unsigned int numberOfParticles);
    virtual void setTexture(CCTexture2D* texture);
    virtual void updateQuadWithParticle(tCCParticle* particle, const CCPoint& newPosition, int index);
    virtual void postStep();
    virtual void draw();
    virtual void setBatchNode(CCParticleBatchNode* batchNode);
    virtual void setTotalParticles(unsigned int tp);
    
    /** listen the event that coming to foreground on Android
     */
    void listenBackToForeground(CCObject *obj);
    //设置延迟时间
    void setDelayTime(float fTime);
	//延迟结束事件
	void delayTimeComplete(float fTime);
    
//    virtual void setParticlePositionType(int type);
    
    static CCParticleSystemQuadEx * create();
    static CCParticleSystemQuadEx * createWithTotalParticles(unsigned int numberOfParticles);
    
    //设置粒子序列帧
    virtual void setTextureFrameRect(ccV3F_C4B_T2F_Quad* quad, tCCParticle* p, int index);
    
private:
#if CC_TEXTURE_ATLAS_USE_VAO
    void setupVBOandVAO();
#else
    void setupVBO();
#endif
    bool allocMemory();
};


NS_CC_END

#endif /* defined(__cocos2dx__CCParticleSystemQuadEx__) */
