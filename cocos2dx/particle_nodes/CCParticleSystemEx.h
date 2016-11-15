//
//  CCParticleSystemEx.h
//  cocos2dx
//
//  Created by Archer on 14/2/12.
//  Copyright (c) 2014年 cocos2d-x. All rights reserved.
//

#ifndef __CCPARTICLE_SYSTEM_EX_H__
#define __CCPARTICLE_SYSTEM_EX_H__

#include  "CCParticleSystem.h"

NS_CC_BEGIN

#define PARTICLE_KEYPOINT_TOTALL 5

//外形类型枚举
enum {
    kCCParticleOutlineRadius,
};
//关键点类型枚举
enum {
    kCCParticleKeyPointNormal,
    kCCParticleKeyPointMulti,
};
//序列帧类型
enum {
    //循环播放序列帧
    kCCParticleLoopFrame,
    //循环播放序列帧(倒播)
    kCCParticleLoopFrameRollback,
    //播放一次
    kCCParticlePlayFrame,
    //播放一次(倒播)
    kCCParticlePlayFrameRollback,
    //随即跳到序列帧
    kCCParticleRandomFrame,
    //跳到指定序列帧
    kCCParticleToFrame,
};
//贴图angle类型
enum {
    //贴图角度和粒子角度一样
    kCCParticleAngle,
    //指定贴图角度
    kCCToAngle,
    //随即贴图角度
    kCCRandomAngle,
};
//用于支持关键点随机颜色和大小
typedef struct sCCKyePointRandom {
    float*      deltaSize;//大小变化速度(每帧)
    ccColor4F*  deltaColor;//颜色变化速度(每帧)
} tCCKyePointRandom;

//粒子关键点数据结构
typedef struct sCCKeyPoint {
    float       size;//大小
    float       sizeVar;//大小变量
    float       deltaSize;//大小变化速度(每帧)
    ccColor4F	color;//颜色
    ccColor4F	colorVar;//颜色变量
    ccColor4F	deltaColor;//颜色变化速度(每帧)
    float       lifePercent;//关键点生命周期(占粒子生命周期的percent)
    float       totalPercent;//关键点生命周期在粒子生命周期上的percent
} tCCKeyPoint;
//贴图序列帧数据结构
typedef struct sCCTextureFrame
{
    //贴图在x轴的切分值（即在x轴上平均切分成多少块）
    int                 carveUpX;
    //贴图在y轴的切分值（即在y轴上平均切分成多少块）
    int                 carveUpY;
    //贴图帧区域大小
    CCSize              frameSize;
    //贴图总帧数
    int                 totalFrame;
    //贴图区域数组
    CCPoint				*frameRectMap;
} tCCTextureFrame;
//半径数据结构
typedef struct sCCRadius
{
    //x轴半径
    float radiusX;
    //y轴半径
    float radiusY;
} tCCRadius;

class CC_DLL CCParticleSystemEx : public CCParticleSystem {
    
    //外形数据结构
    union {
        struct{
            //半径（用两个半径则可以实现椭圆）
            float radiusX;
            float radiusXVar;
            float radiusY;
            float radiusYVar;
        } circle;
    } _outline;
public:
    
    CCParticleSystemEx();
    virtual ~CCParticleSystemEx();
    
    //关键点
    tCCKeyPoint     *m_pKeyPoints;
    //
    tCCRadius       *m_pRadius;
    //
    tCCKyePointRandom *m_pKeyPointsRandom;
    
    //发射器轮廓类型
    CC_SYNTHESIZE(int, m_outlineType, OutlineType);
    
    //关键点类型
    CC_SYNTHESIZE(int, m_keyPointType, KeyPointType);
    
    //颜色是否已更新
    int             m_iColorUpdated;
    //大小是否已更新
    bool            m_bIsSizeUpdated;
    
    //粒子声明周期数组（用于纪录每个列子的生命周期，该数据在计算粒子生存帧中使用）
    float           *m_pParticleLifeTime;
    //用于纪录粒子当前帧
    double          *m_pParticleCurrentFrame;
    //跳到指定帧
    CC_SYNTHESIZE(int, m_toFrame, ToFrame);
    //每帧的时间
    CC_SYNTHESIZE_READONLY(std::string, m_strFrameTime, FrameTime);
    //序列帧类型
    CC_SYNTHESIZE(float, m_particleFrameType, ParticleFrameType);
    
    //贴图x轴缩放比例
    CC_SYNTHESIZE(float,m_textureScaleX,TextureScaleX);
    //贴图y轴缩放比例
    CC_SYNTHESIZE(float,m_textureScaleY,TextureScaleY);
    //帧时间最大id
    unsigned int        m_totalFrameId;
    //每帧对应时间的数组
    double          *m_pTimeToFrame;
    //每帧平均时间
    double          m_earchFrameTime;
    //贴图旋转角度类型
    CC_SYNTHESIZE(int, m_textureAngleType, TextureAngleType);
    //指定贴图旋转角度
    CC_SYNTHESIZE(float, m_textureAngle, TextureAngle);
    CC_SYNTHESIZE(float, m_textureAngleVar, TextureAngleVar);
    //存储贴图旋转角度数据
    float           *m_pParticleTextureAngle;
    
    virtual void setFrameTime(const char *time);
    virtual int getFrameId(double time);
    
    tCCTextureFrame m_particleTextureFrame;
    
    virtual bool initWithDictionary(CCDictionary *dictionary, const char *dirname);
    virtual bool initWithTotalParticles(unsigned int numberOfParticles);
    virtual void initParticle(tCCParticle* particle);
    virtual void update(float dt);
	virtual void setVisible(bool var);
    
    bool addParticle();
    
    //设置outline半径
    virtual float           getRadiusX();
    virtual void            setRadiusX(float r);
    virtual float           getRadiusXVar();
    virtual void            setRadiusXVar(float r);
    virtual float           getRadiusY();
    virtual void            setRadiusY(float r);
    virtual float           getRadiusYVar();
    virtual void            setRadiusYVar(float r);
    
    //关键点数据设置
    //A Size
    virtual float           getKeyPointASize();
    virtual void            setKeyPointASize(float size);
    virtual float           getKeyPointASizeVar();
    virtual void            setKeyPointASizeVar(float size);
    //A Color
    virtual ccColor4F       getKeyPointAColor();
    virtual void            setKeyPointAColor(ccColor4F color);
    virtual ccColor4F       getKeyPointAColorVar();
    virtual void            setKeyPointAColorVar(ccColor4F color);
    //A Percent
    virtual float           getKeyPointALifePercent();
    virtual void            setKeyPointALifePercent(float percent);
    
    //B Size
    virtual float           getKeyPointBSize();
    virtual void            setKeyPointBSize(float size);
    virtual float           getKeyPointBSizeVar();
    virtual void            setKeyPointBSizeVar(float size);
    //B Color
    virtual ccColor4F       getKeyPointBColor();
    virtual void            setKeyPointBColor(ccColor4F color);
    virtual ccColor4F       getKeyPointBColorVar();
    virtual void            setKeyPointBColorVar(ccColor4F color);
    //B Percent
    virtual float           getKeyPointBLifePercent();
    virtual void            setKeyPointBLifePercent(float percent);
    
    //C Size
    virtual float           getKeyPointCSize();
    virtual void            setKeyPointCSize(float size);
    virtual float           getKeyPointCSizeVar();
    virtual void            setKeyPointCSizeVar(float size);
    //C Color
    virtual ccColor4F       getKeyPointCColor();
    virtual void            setKeyPointCColor(ccColor4F color);
    virtual ccColor4F       getKeyPointCColorVar();
    virtual void            setKeyPointCColorVar(ccColor4F color);
    //C Percent
    virtual float           getKeyPointCLifePercent();
    virtual void            setKeyPointCLifePercent(float percent);
    
    //D Size
    virtual float           getKeyPointDSize();
    virtual void            setKeyPointDSize(float size);
    virtual float           getKeyPointDSizeVar();
    virtual void            setKeyPointDSizeVar(float size);
    //D Color
    virtual ccColor4F       getKeyPointDColor();
    virtual void            setKeyPointDColor(ccColor4F color);
    virtual ccColor4F       getKeyPointDColorVar();
    virtual void            setKeyPointDColorVar(ccColor4F color);
    //D Percent
    virtual float           getKeyPointDLifePercent();
    virtual void            setKeyPointDLifePercent(float percent);
    
    //E Size
    virtual float           getKeyPointESize();
    virtual void            setKeyPointESize(float size);
    virtual float           getKeyPointESizeVar();
    virtual void            setKeyPointESizeVar(float size);
    //E Color
    virtual ccColor4F       getKeyPointEColor();
    virtual void            setKeyPointEColor(ccColor4F color);
    virtual ccColor4F       getKeyPointEColorVar();
    virtual void            setKeyPointEColorVar(ccColor4F color);
    
    //设置贴图在x轴的切分值（即在x轴上平均切分成多少块）
    virtual int             getTextureCarveUpX();
    virtual void            setTextureCarveUpX(int x);
    
    //设置贴图在y轴的切分值（即在y轴上平均切分成多少块）
    virtual int             getTextureCarveUpY();
    virtual void            setTextureCarveUpY(int y);
    
    //比较颜色是否相同
    bool compareWithColor(ccColor4F a, ccColor4F b);
    //更新粒子颜色
    float updateColor(float color, float percent, float tpercent, float dt, int ids);
    //更新粒子大小
    float updateSize(float size, float percent, float tpercent, float dt, int ids);
    //获取粒子生命周期
    float getParticleElapsedTime(int index);
    
    virtual void updateQuadWithParticle(tCCParticle* particle, const CCPoint& newPosition, int index);
    //贴图序列图切分计算
    virtual void textureCarveUp();
    
    //纪录粒子当前帧
    bool setParticleCurrentFrame(int index, int frame);
};

NS_CC_END

#endif //__CCPARTICLE_SYSTEM_EX_H__
