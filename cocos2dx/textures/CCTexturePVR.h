/****************************************************************************
Copyright (c) 2011 Jirka Fajfr for cocos2d-x
Copyright (c) 2010 cocos2d-x.org

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#ifndef __CCPVRTEXTURE_H__
#define __CCPVRTEXTURE_H__

#include "CCStdC.h"
#include "CCGL.h"
#include "cocoa/CCObject.h"
#include "cocoa/CCArray.h"
#include "CCPVRParzer.h"

NS_CC_BEGIN

/** CCTexturePVR
     
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
class CCTexturePVR : public CCObject
{
public:
    CCTexturePVR();
    virtual ~CCTexturePVR();

    /** initializes a CCTexturePVR with a path */
    bool initWithContentsOfFile(const char* path);

    /** creates and initializes a CCTexturePVR with a path */
    static CCTexturePVR* create(const char* path);
    
    // properties 
    
    /** texture width */
    inline unsigned int getWidth() { return m_PVRParzer.getWidth(); }
    /** texture height */
    inline unsigned int getHeight() { return m_PVRParzer.getHeight(); }
    /** whether or not the texture has alpha */
    inline bool hasAlpha() { return m_PVRParzer.hasAlpha(); }
    /** whether or not the texture has premultiplied alpha */
    inline bool hasPremultipliedAlpha() { return m_PVRParzer.hasPremultipliedAlpha(); }
    /** whether or not the texture should use hasPremultipliedAlpha instead of global default */
    inline bool isForcePremultipliedAlpha() { return m_PVRParzer.isForcePremultipliedAlpha(); }
    /** how many mipmaps the texture has. 1 means one level (level 0 */
    inline unsigned int getNumberOfMipmaps() { return m_PVRParzer.getNumberOfMipmaps(); }
    inline CCPVRMipmap* getMipmaps() { return m_PVRParzer.getMipmaps(); }
    inline GLenum getInternalFormat() const { return m_PVRParzer.getInternalFormat(); }
    inline GLenum getPixelDataFormat() const { return m_PVRParzer.getPixelDataFormat(); }
    inline GLenum getPixelDataType() const { return m_PVRParzer.getPixelDataType(); }
    inline bool isCompressed() const { return m_PVRParzer.isCompressed(); }
    inline CCTexture2DPixelFormat getFormat() { return m_PVRParzer.getFormat(); }

private:
    bool unpackPVRv2Data(unsigned char* data, unsigned int len);
    bool unpackPVRv3Data(unsigned char* dataPointer, unsigned int dataLength);
    
protected:
    CCPVRParzer m_PVRParzer;
    unsigned char* m_PvrData;
};

// end of textures group
/// @}

NS_CC_END


#endif //__CCPVRTEXTURE_H__