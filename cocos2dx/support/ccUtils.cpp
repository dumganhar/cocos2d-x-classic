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
#include "ccUtils.h"
#include <algorithm>
#include <sstream>

namespace cocos2d {

unsigned long ccNextPOT(unsigned long x)
{
    x = x - 1;
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >>16);
    return x + 1;
}

std::string ccFileName(const std::string& filePath)
{
    std::string path(filePath);
    std::replace(path.begin(), path.end(), '\\', '/');

    if (filePath.empty() || '/' == filePath[filePath.length()-1])
    {
        return "";
    }

    size_t s = path.find_last_of('/');
    if (std::string::npos == s)
    {
        return filePath;
    }

    size_t e = path.find_last_of('.');
    if (std::string::npos == e)
    {
        return path.substr(s + 1);
    }

    if (s > e)
    {
        return "";
    }

    return path.substr(s+1, e-s-1);
}

int ccHash(const char* s)
{
    int hash = 0;
    int length = strlen(s);
    for (int i=0; i<length; ++i)
    {
        hash += s[i] * 89;
    }

    return hash;
}

char ccBitrand(char byte)
{
    return ((byte&0x0f)<<4) | ((byte&0xf0)>>4);
}

template<class T>
void ccToString(std::string& result, const T& t)
{
	std::ostringstream oss;
	oss << t;

	result = oss.str();
}

}