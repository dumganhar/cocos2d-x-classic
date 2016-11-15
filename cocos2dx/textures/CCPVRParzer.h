#ifndef __CCPVRPARZER_H__
#define __CCPVRPARZER_H__

#include "CCStdC.h"
#include "CCGL.h"
#include "cocoa/CCObject.h"
#include "cocoa/CCArray.h"

NS_CC_BEGIN

/**
 * @addtogroup textures
 * @{
 */

/**
 @brief Structure which can tell where mipmap begins and how long is it
*/
struct CCPVRMipmap {
    unsigned char *address;
    unsigned int len;
};

/**
 @brief Determine how many mipmaps can we have. 
 Its same as define but it respects namespaces
*/
enum {
    CC_PVRMIPMAP_MAX = 16,
};

/** CCPVRParzer
     
 Object that loads PVR images.

 Supported PVR formats:
    - RGBA8888
    - BGRA8888
    - RGBA4444
    - RGBA5551
    - RGB565
    - A8
    - I8
    - AI88
    - PVRTC 4BPP
    - PVRTC 2BPP
     
 Limitations:
    Pre-generated mipmaps, such as PVR textures with mipmap levels embedded in file,
    are only supported if all individual sprites are of _square_ size. 
    To use mipmaps with non-square textures, instead call CCTexture2D#generateMipmap on the sheet texture itself
    (and to save space, save the PVR sprite sheet without mip maps included).
*/
class CCPVRParzer
{
public:
    CCPVRParzer();
    ~CCPVRParzer();

    /** parze pvr data */
    bool parzeData(const void *data, unsigned int dataLen);
    
    // properties 
    
    /** texture width */
    inline unsigned int getWidth() { return m_uWidth; }
    /** texture height */
    inline unsigned int getHeight() { return m_uHeight; }
    inline unsigned int getBitsPerPixel() { return m_uBitsPerPixel; }
    /** whether or not the texture has alpha */
    inline bool hasAlpha() { return m_bHasAlpha; }
    /** whether or not the texture has premultiplied alpha */
    inline bool hasPremultipliedAlpha() { return m_bHasPremultipliedAlpha; }
    /** whether or not the texture should use hasPremultipliedAlpha instead of global default */
    inline bool isForcePremultipliedAlpha() { return m_bForcePremultipliedAlpha; }
    /** how many mipmaps the texture has. 1 means one level (level 0 */
    inline unsigned int getNumberOfMipmaps() { return m_uNumberOfMipmaps; }
    inline CCPVRMipmap* getMipmaps() { return m_asMipmaps; }
    inline GLenum getInternalFormat() const { return m_eInternalFormat; }
    inline GLenum getPixelDataFormat() const { return m_ePixelDataformat; }
    inline GLenum getPixelDataType() const { return m_ePixelDataType; }
    inline bool isCompressed() const { return m_bCompressed; }
    inline CCTexture2DPixelFormat getFormat() { return m_eCCFormat; }

private:
    bool unpackPVRv2Data(unsigned char* data, unsigned int len);
    bool unpackPVRv3Data(unsigned char* dataPointer, unsigned int dataLength);
    
protected:
    struct CCPVRMipmap m_asMipmaps[CC_PVRMIPMAP_MAX];   // pointer to mipmap images
    unsigned int m_uNumberOfMipmaps;                    // number of mipmap used
    
    unsigned int m_uWidth, m_uHeight;
    unsigned int m_uBitsPerPixel;
    GLenum m_eInternalFormat;
    GLenum m_ePixelDataformat;
    GLenum m_ePixelDataType;
    bool m_bCompressed;
    bool m_bHasAlpha;
    bool m_bHasPremultipliedAlpha;
    bool m_bForcePremultipliedAlpha;
    CCTexture2DPixelFormat m_eCCFormat;
};

// end of textures group
/// @}

NS_CC_END


#endif //__CCPVRPARZER_H__