/****************************************************************************
Copyright (c) 2011        Jirka Fajfr for cocos2d-x
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2008      Apple Inc. All Rights Reserved.

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

#include "CCTexture2D.h"
#include "CCTexturePVR.h"
#include "ccMacros.h"
#include "CCConfiguration.h"
#include "support/ccUtils.h"
#include "CCStdC.h"
#include "platform/CCFileUtils.h"
#include "support/zip_support/ZipUtils.h"
#include "shaders/ccGLStateCache.h"
#include <ctype.h>
#include <cctype>

NS_CC_BEGIN

CCTexturePVR::CCTexturePVR()
    : m_PvrData(NULL)
{
}

CCTexturePVR::~CCTexturePVR()
{
    CCLOGINFO( "cocos2d: deallocing CCTexturePVR" );

    CC_SAFE_DELETE_ARRAY(m_PvrData);
}

bool CCTexturePVR::initWithContentsOfFile(const char* path)
{
    CC_SAFE_DELETE_ARRAY(m_PvrData);

    int pvrlen = 0;

    std::string lowerCase(path);
    for (unsigned int i = 0; i < lowerCase.length(); ++i)
    {
        lowerCase[i] = tolower(lowerCase[i]);
    }

    if (lowerCase.find(".ccz") != std::string::npos)
    {
        pvrlen = ZipUtils::ccInflateCCZFile(path, &m_PvrData);
    }
    else if (lowerCase.find(".gz") != std::string::npos)
    {
        pvrlen = ZipUtils::ccInflateGZipFile(path, &m_PvrData);
    }
    else
    {
        m_PvrData = CCFileUtils::sharedFileUtils()->getFileData(path, "rb", (unsigned long *)(&pvrlen));
    }

    if (pvrlen < 0)
    {
        return false;
    }

    return m_PVRParzer.parzeData(m_PvrData, pvrlen);
}

CCTexturePVR * CCTexturePVR::create(const char* path)
{
    CCTexturePVR * pTexture = new CCTexturePVR();
    if (pTexture)
    {
        if (pTexture->initWithContentsOfFile(path))
        {
            pTexture->autorelease();
        }
        else
        {
            delete pTexture;
            pTexture = NULL;
        }
    }

    return pTexture;
}

NS_CC_END

