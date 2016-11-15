/****************************************************************************
 Copyright (c) 2013-2014 Chukong Technologies Inc.

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

#ifndef _CC_QUADCOMMAND_H_
#define _CC_QUADCOMMAND_H_

#include "CCRenderCommand.h"
#include "CCRenderCommandPool.h"
#include "kazmath/kazmath.h"

#include "shaders/CCGLProgram.h"
//#include "base/ccTypes.h"

#include "include/ccTypes.h"

NS_CC_BEGIN

/** Command used to render one or more Quads */
class CC_DLL QuadCommand : public RenderCommand
{
public:
    QuadCommand();
    ~QuadCommand();

    /** Initializes the command with a globalZOrder, a texture ID, a `GLProgram`, a blending function, a pointer to quads,
     * quantity of quads, and the Model View transform to be used for the quads */
    void init(float globalOrder, GLuint texutreID, CCGLProgram* shader, ccBlendFunc blendType, ccV3F_C4B_T2F_Quad* quads, int quadCount,
              kmMat4& mv);

    void useMaterial() ;

    //TODO use material to decide if it is translucent
    inline bool isTranslucent() const { return true; }

    inline uint32_t getMaterialID() const { return _materialID; }

    inline GLuint getTextureID() const { return _textureID; }

    inline ccV3F_C4B_T2F_Quad* getQuads() const { return _quads; }

    inline int getQuadCount() const { return _quadsCount; }

    inline CCGLProgram* getShader() const { return _shader; }

    inline ccBlendFunc getBlendType() const { return _blendType; }

    inline const kmMat4& getModelView() const { return _mv; }
    
private:
    void generateMaterialID();
    
protected:    
    uint32_t _materialID;

    GLuint _textureID;

    CCGLProgram* _shader;

    ccBlendFunc _blendType;

    ccV3F_C4B_T2F_Quad* _quads;
    int _quadsCount;

    kmMat4 _mv;

	CCRect rect;
};

NS_CC_END

#endif //_CC_QUADCOMMAND_H_
