//
//  CCParticleSystemEx.h
//  cocos2dx
//
//  Created by Archer on 14/2/12.
//  Copyright (c) 2014�� cocos2d-x. All rights reserved.
//

#ifndef __CCPARTICLE_SYSTEM_EX_H__
#define __CCPARTICLE_SYSTEM_EX_H__

#include  "CCParticleSystem.h"

NS_CC_BEGIN

#define PARTICLE_KEYPOINT_TOTALL 5

//��������ö��
enum {
    kCCParticleOutlineRadius,
};
//�ؼ�������ö��
enum {
    kCCParticleKeyPointNormal,
    kCCParticleKeyPointMulti,
};
//����֡����
enum {
    //ѭ����������֡
    kCCParticleLoopFrame,
    //ѭ����������֡(����)
    kCCParticleLoopFrameRollback,
    //����һ��
    kCCParticlePlayFrame,
    //����һ��(����)
    kCCParticlePlayFrameRollback,
    //�漴��������֡
    kCCParticleRandomFrame,
    //����ָ������֡
    kCCParticleToFrame,
};
//��ͼangle����
enum {
    //��ͼ�ǶȺ����ӽǶ�һ��
    kCCParticleAngle,
    //ָ����ͼ�Ƕ�
    kCCToAngle,
    //�漴��ͼ�Ƕ�
    kCCRandomAngle,
};
//����֧�ֹؼ��������ɫ�ʹ�С
typedef struct sCCKyePointRandom {
    float*      deltaSize;//��С�仯�ٶ�(ÿ֡)
    ccColor4F*  deltaColor;//��ɫ�仯�ٶ�(ÿ֡)
} tCCKyePointRandom;

//���ӹؼ������ݽṹ
typedef struct sCCKeyPoint {
    float       size;//��С
    float       sizeVar;//��С����
    float       deltaSize;//��С�仯�ٶ�(ÿ֡)
    ccColor4F	color;//��ɫ
    ccColor4F	colorVar;//��ɫ����
    ccColor4F	deltaColor;//��ɫ�仯�ٶ�(ÿ֡)
    float       lifePercent;//�ؼ�����������(ռ�����������ڵ�percent)
    float       totalPercent;//�ؼ��������������������������ϵ�percent
} tCCKeyPoint;
//��ͼ����֡���ݽṹ
typedef struct sCCTextureFrame
{
    //��ͼ��x����з�ֵ������x����ƽ���зֳɶ��ٿ飩
    int                 carveUpX;
    //��ͼ��y����з�ֵ������y����ƽ���зֳɶ��ٿ飩
    int                 carveUpY;
    //��ͼ֡�����С
    CCSize              frameSize;
    //��ͼ��֡��
    int                 totalFrame;
    //��ͼ��������
    CCPoint				*frameRectMap;
} tCCTextureFrame;
//�뾶���ݽṹ
typedef struct sCCRadius
{
    //x��뾶
    float radiusX;
    //y��뾶
    float radiusY;
} tCCRadius;

class CC_DLL CCParticleSystemEx : public CCParticleSystem {
    
    //�������ݽṹ
    union {
        struct{
            //�뾶���������뾶�����ʵ����Բ��
            float radiusX;
            float radiusXVar;
            float radiusY;
            float radiusYVar;
        } circle;
    } _outline;
public:
    
    CCParticleSystemEx();
    virtual ~CCParticleSystemEx();
    
    //�ؼ���
    tCCKeyPoint     *m_pKeyPoints;
    //
    tCCRadius       *m_pRadius;
    //
    tCCKyePointRandom *m_pKeyPointsRandom;
    
    //��������������
    CC_SYNTHESIZE(int, m_outlineType, OutlineType);
    
    //�ؼ�������
    CC_SYNTHESIZE(int, m_keyPointType, KeyPointType);
    
    //��ɫ�Ƿ��Ѹ���
    int             m_iColorUpdated;
    //��С�Ƿ��Ѹ���
    bool            m_bIsSizeUpdated;
    
    //���������������飨���ڼ�¼ÿ�����ӵ��������ڣ��������ڼ�����������֡��ʹ�ã�
    float           *m_pParticleLifeTime;
    //���ڼ�¼���ӵ�ǰ֡
    double          *m_pParticleCurrentFrame;
    //����ָ��֡
    CC_SYNTHESIZE(int, m_toFrame, ToFrame);
    //ÿ֡��ʱ��
    CC_SYNTHESIZE_READONLY(std::string, m_strFrameTime, FrameTime);
    //����֡����
    CC_SYNTHESIZE(float, m_particleFrameType, ParticleFrameType);
    
    //��ͼx�����ű���
    CC_SYNTHESIZE(float,m_textureScaleX,TextureScaleX);
    //��ͼy�����ű���
    CC_SYNTHESIZE(float,m_textureScaleY,TextureScaleY);
    //֡ʱ�����id
    unsigned int        m_totalFrameId;
    //ÿ֡��Ӧʱ�������
    double          *m_pTimeToFrame;
    //ÿ֡ƽ��ʱ��
    double          m_earchFrameTime;
    //��ͼ��ת�Ƕ�����
    CC_SYNTHESIZE(int, m_textureAngleType, TextureAngleType);
    //ָ����ͼ��ת�Ƕ�
    CC_SYNTHESIZE(float, m_textureAngle, TextureAngle);
    CC_SYNTHESIZE(float, m_textureAngleVar, TextureAngleVar);
    //�洢��ͼ��ת�Ƕ�����
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
    
    //����outline�뾶
    virtual float           getRadiusX();
    virtual void            setRadiusX(float r);
    virtual float           getRadiusXVar();
    virtual void            setRadiusXVar(float r);
    virtual float           getRadiusY();
    virtual void            setRadiusY(float r);
    virtual float           getRadiusYVar();
    virtual void            setRadiusYVar(float r);
    
    //�ؼ�����������
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
    
    //������ͼ��x����з�ֵ������x����ƽ���зֳɶ��ٿ飩
    virtual int             getTextureCarveUpX();
    virtual void            setTextureCarveUpX(int x);
    
    //������ͼ��y����з�ֵ������y����ƽ���зֳɶ��ٿ飩
    virtual int             getTextureCarveUpY();
    virtual void            setTextureCarveUpY(int y);
    
    //�Ƚ���ɫ�Ƿ���ͬ
    bool compareWithColor(ccColor4F a, ccColor4F b);
    //����������ɫ
    float updateColor(float color, float percent, float tpercent, float dt, int ids);
    //�������Ӵ�С
    float updateSize(float size, float percent, float tpercent, float dt, int ids);
    //��ȡ������������
    float getParticleElapsedTime(int index);
    
    virtual void updateQuadWithParticle(tCCParticle* particle, const CCPoint& newPosition, int index);
    //��ͼ����ͼ�зּ���
    virtual void textureCarveUp();
    
    //��¼���ӵ�ǰ֡
    bool setParticleCurrentFrame(int index, int frame);
};

NS_CC_END

#endif //__CCPARTICLE_SYSTEM_EX_H__
