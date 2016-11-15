/****************************************************************************
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

#include "platform/CCImage.h"
#include "textures/CCTexture2D.h"
#include "textures/CCPVRParzer.h"
#include "ccMacros.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

NS_CC_BEGIN

bool CCImage::_initWithPvrData(void *pData, int nDataLen)
{
    CC_SAFE_DELETE_ARRAY(m_pData);

    m_pData = new unsigned char[nDataLen];

    if (m_pData == NULL)
    {
        return false;
    }

    memcpy(m_pData, pData, nDataLen);

    CCPVRParzer pvrParzer;

    if (pvrParzer.parzeData(m_pData, nDataLen))
    {
        m_nWidth  = (unsigned short)pvrParzer.getWidth();
        m_nHeight = (unsigned short)pvrParzer.getHeight();
        m_nBitsPerComponent = (int)pvrParzer.getBitsPerPixel();
        m_bHasAlpha = pvrParzer.hasAlpha();
        m_bPreMulti = pvrParzer.hasPremultipliedAlpha();
        m_bCompressed = pvrParzer.isCompressed();
        // 此处设置一个临时参数nMipmapArraySize来向memcpy传递数据。
        // 这是因为在ios x86模拟器环境下，memcpy在此处会被转换为rep mov指令。
        // 而运行时rep mov指令的执行结果是错误的，只会拷贝4个字节。
        // 因此使用临时参数的方式阻止编译器把memcpy编译为rep mov。
        // 这种转换当前只出现在x86模拟器环境下。
        size_t nMipmapArraySize = sizeof(m_asMipmaps);
        memcpy(m_asMipmaps, pvrParzer.getMipmaps(), nMipmapArraySize);
        m_uNumberOfMipmaps = pvrParzer.getNumberOfMipmaps();
        m_eInternalFormat = pvrParzer.getInternalFormat();
        m_ePixelDataformat = pvrParzer.getPixelDataFormat();
        m_ePixelDataType = pvrParzer.getPixelDataType();
        m_eCCFormat = pvrParzer.getFormat();
        return true;
    }

    CC_SAFE_DELETE_ARRAY(m_pData);
    return false;
}

NS_CC_END
