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

#include "CCRenderer.h"

#include <algorithm>
#include "include/ccMacros.h"
#include "renderer/CCQuadCommand.h"
#include "renderer/CCBatchCommand.h"
#include "renderer/CCCustomCommand.h"
#include "renderer/CCGroupCommand.h"

//#include "CCShaderCache.h"
#include "shaders/ccGLStateCache.h"
#include "CCConfiguration.h"
#include "CCDirector.h"
//#include "CCEventDispatcher.h"
//#include "CCEventListenerCustom.h"
//#include "CCEventType.h"

#include "kazmath/kazmath.h"
#include "kazmath/vec4.h"
#include "kazmath/mat4.h"
NS_CC_BEGIN

// helper
static bool compareRenderCommand(RenderCommand* a, RenderCommand* b)
{
    return a->getGlobalOrder() < b->getGlobalOrder();
}


static bool compareRenderCommandMaterial(RenderCommand* a, RenderCommand* b)
{
	QuadCommand* a1 = (QuadCommand*)a;
	QuadCommand* b1 = (QuadCommand*)b;

	return a1->getMaterialID() < b1->getMaterialID();
}
// queue

void RenderQueue::push_back(RenderCommand* command)
{
    float z = command->getGlobalOrder();
    if(z < 0)
        _queueNegZ.push_back(command);
    else if(z > 0)
        _queuePosZ.push_back(command);
    else
        _queue0.push_back(command);
}

int RenderQueue::size() const
{
    return _queueNegZ.size() + _queue0.size() + _queuePosZ.size();
}

void RenderQueue::sort()
{
    // Don't sort _queue0, it already comes sorted
    std::sort(std::begin(_queueNegZ), std::end(_queueNegZ), compareRenderCommand);
    std::sort(std::begin(_queuePosZ), std::end(_queuePosZ), compareRenderCommand);
}


void RenderQueue::sortMaterial()
{
	// Don't sort _queue0, it already comes sorted
	std::sort(std::begin(_queue0), std::end(_queue0), compareRenderCommandMaterial);
}


RenderCommand* RenderQueue::operator[](int index) const
{
    if(index < static_cast<int>(_queueNegZ.size()))
        return _queueNegZ[index];

    index -= _queueNegZ.size();

    if(index < static_cast<int>(_queue0.size()))
        return _queue0[index];

    index -= _queue0.size();

    if(index < static_cast<int>(_queuePosZ.size()))
        return _queuePosZ[index];

    CCAssert(false, "invalid index");
    return nullptr;
}

void RenderQueue::clear()
{
    _queueNegZ.clear();
    _queue0.clear();
    _queuePosZ.clear();
}

//
//
//
static const int DEFAULT_RENDER_QUEUE = 0;

//
// constructors, destructors, init
//
Renderer::Renderer()
:_lastMaterialID(0)
,_numQuads(0)
,_glViewAssigned(false)
,_isRendering(false)
#if CC_ENABLE_CACHE_TEXTURE_DATA
,_cacheTextureListener(nullptr)
#endif
{
    _groupCommandManager = new GroupCommandManager();
    
    _commandGroupStack.push(DEFAULT_RENDER_QUEUE);
    
    RenderQueue defaultRenderQueue;
    _renderGroups.push_back(defaultRenderQueue);
    _batchedQuadCommands.reserve(BATCH_QUADCOMMAND_RESEVER_SIZE);
}

Renderer::~Renderer()
{
    _renderGroups.clear();
    _groupCommandManager->release();
    
    glDeleteBuffers(2, _buffersVBO);
    
	
    if (CCConfiguration::sharedConfiguration()->supportsShareableVAO())
    {
        glDeleteVertexArrays(1, &_quadVAO);
        ccGLBindVAO(0);
    }
#if CC_ENABLE_CACHE_TEXTURE_DATA
    Director::getInstance()->getEventDispatcher()->removeEventListener(_cacheTextureListener);
#endif
}

void Renderer::initGLView()
{
#if CC_ENABLE_CACHE_TEXTURE_DATA
    _cacheTextureListener = EventListenerCustom::create(EVENT_COME_TO_FOREGROUND, [this](EventCustom* event){
        /** listen the event that coming to foreground on Android */
        this->setupBuffer();
    });
    
    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(_cacheTextureListener, -1);
#endif

    setupIndices();
    
    setupBuffer();
    
    _glViewAssigned = true;
}

void Renderer::setupIndices()
{
    for( int i=0; i < VBO_SIZE; i++)
    {
        _indices[i*6+0] = (GLushort) (i*4+0);
        _indices[i*6+1] = (GLushort) (i*4+1);
        _indices[i*6+2] = (GLushort) (i*4+2);
        _indices[i*6+3] = (GLushort) (i*4+3);
        _indices[i*6+4] = (GLushort) (i*4+2);
        _indices[i*6+5] = (GLushort) (i*4+1);
    }
}

void Renderer::setupBuffer()
{
    if(CCConfiguration::sharedConfiguration()->supportsShareableVAO())
    {
        setupVBOAndVAO();
    }
    else
    {
        setupVBO();
    }
}

void Renderer::setupVBOAndVAO()
{
    glGenVertexArrays(1, &_quadVAO);
    ccGLBindVAO(_quadVAO);

    glGenBuffers(2, &_buffersVBO[0]);

    glBindBuffer(GL_ARRAY_BUFFER, _buffersVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(_quads[0]) * VBO_SIZE, _quads, GL_DYNAMIC_DRAW);

    // vertices
	
    glEnableVertexAttribArray(kCCVertexAttrib_Position);
    glVertexAttribPointer(kCCVertexAttrib_Position, 3, GL_FLOAT, GL_FALSE, sizeof(ccV3F_C4B_T2F), (GLvoid*) offsetof( ccV3F_C4B_T2F, vertices));
    // colors
    glEnableVertexAttribArray(kCCVertexAttrib_Color);
    glVertexAttribPointer(kCCVertexAttrib_Color, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ccV3F_C4B_T2F), (GLvoid*) offsetof( ccV3F_C4B_T2F, colors));

    // tex coords
    glEnableVertexAttribArray(kCCVertexAttrib_TexCoords);
    glVertexAttribPointer(kCCVertexAttrib_TexCoords, 2, GL_FLOAT, GL_FALSE, sizeof(ccV3F_C4B_T2F), (GLvoid*) offsetof( ccV3F_C4B_T2F, texCoords));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffersVBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(_indices[0]) * VBO_SIZE * 6, _indices, GL_STATIC_DRAW);

    // Must unbind the VAO before changing the element buffer.
    ccGLBindVAO(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //CHECK_GL_ERROR_DEBUG();
}

void Renderer::setupVBO()
{
    glGenBuffers(2, &_buffersVBO[0]);

    mapBuffers();
}

void Renderer::mapBuffers()
{
    // Avoid changing the element buffer for whatever VAO might be bound.
    ccGLBindVAO(0);

    glBindBuffer(GL_ARRAY_BUFFER, _buffersVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(_quads[0]) * VBO_SIZE, _quads, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffersVBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(_indices[0]) * VBO_SIZE * 6, _indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

//    CHECK_GL_ERROR_DEBUG();
}

void Renderer::addCommand(RenderCommand* command)
{
	if (command->getType() == RenderCommand::QUAD_COMMAND)
	{
		int renderQueue =_commandGroupStack.top();
		addCommand(command, renderQueue);
	}
	
    
}

void Renderer::addCommand(RenderCommand* command, int renderQueue)
{
    CCAssert(!_isRendering, "Cannot add command while rendering");
    CCAssert(renderQueue >=0, "Invalid render queue");
    CCAssert(command->getType() != RenderCommand::UNKNOWN_COMMAND, "Invalid Command Type");
    _renderGroups[renderQueue].push_back(command);
}

void Renderer::pushGroup(int renderQueueID)
{
    CCAssert(!_isRendering, "Cannot change render queue while rendering");
    _commandGroupStack.push(renderQueueID);
}

void Renderer::popGroup()
{
    CCAssert(!_isRendering, "Cannot change render queue while rendering");
    _commandGroupStack.pop();
}

int Renderer::createRenderQueue()
{
    RenderQueue newRenderQueue;
    _renderGroups.push_back(newRenderQueue);
    return (int)_renderGroups.size() - 1;
}

void Renderer::visitRenderQueue(const RenderQueue& queue)
{
    int size = queue.size();
    for (int index = 0; index < size; ++index)
    {
        auto command = queue[index];
        auto commandType = command->getType();
        if(RenderCommand::QUAD_COMMAND == commandType)
        {
            auto cmd = static_cast<QuadCommand*>(command);
            //Batch quads
            if(_numQuads + cmd->getQuadCount() > VBO_SIZE)
            {
                CCAssert(cmd->getQuadCount()>= 0 && cmd->getQuadCount() < VBO_SIZE, "VBO is not big enough for quad data, please break the quad data down or use customized render command");
                
                //Draw batched quads if VBO is full
                drawBatchedQuads();
            }
            
            _batchedQuadCommands.push_back(cmd);
            
            memcpy(_quads + _numQuads, cmd->getQuads(), sizeof(ccV3F_C4B_T2F_Quad) * cmd->getQuadCount());
            convertToWorldCoordinates(_quads + _numQuads, cmd->getQuadCount(), cmd->getModelView());
            
            _numQuads += cmd->getQuadCount();

        }
		else if(RenderCommand::BATCH_COMMAND == commandType)
		{
			flush();
			auto cmd = static_cast<BatchCommand*>(command);
			cmd->execute();
		}

		else if(RenderCommand::GROUP_COMMAND == commandType)
		{
			flush();
			int renderQueueID = ((GroupCommand*) command)->getRenderQueueID();
			visitRenderQueue(_renderGroups[renderQueueID]);
		}
		else if(RenderCommand::CUSTOM_COMMAND == commandType)
		{
			flush();
			auto cmd = static_cast<CustomCommand*>(command);
			cmd->execute();
		}

        else
        {
            CCLOGERROR("Unknown commands in renderQueue");
        }
    }
}

void Renderer::render()
{
    //Uncomment this once everything is rendered by new renderer
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //TODO setup camera or MVP
    _isRendering = true;
    
    if (_glViewAssigned)
    {
        // cleanup
        _drawnBatches = _drawnVertices = 0;

        //Process render commands
        //1. Sort render commands based on ID

		//std::vector<RenderQueue> _renderGroups
		for (std::vector<RenderQueue>::iterator renderqueue = _renderGroups.begin() ; renderqueue != _renderGroups.end(); ++renderqueue)
        //for (auto &renderqueue : _renderGroups)
        {
			(*renderqueue).sort();
            //renderqueue.sort();
        }


		/*ssize_t size = _renderGroups[0].size();
		for (ssize_t index = 0; index < size; ++index)
		{
		auto command = _renderGroups[0][index];
		auto commandType = command->getType();
		if(RenderCommand::Type::QUAD_COMMAND == commandType)
		{
		auto cmd = static_cast<QuadCommand*>(command);

		cmd->getQuads()->tl.vertices.z = index;
		cmd->getQuads()->bl.vertices.z = index;
		cmd->getQuads()->tr.vertices.z = index;
		cmd->getQuads()->br.vertices.z = index;
		}
		}

		_renderGroups[0].sortMaterial();*/
		//renderZOrder
        visitRenderQueue(_renderGroups[0]);
        flush();
    }
    clean();
    _isRendering = false;
}

void Renderer::clean()
{
    // Clear render group
    for (size_t j = 0 ; j < _renderGroups.size(); j++)
    {
        //commands are owned by nodes
        // for (const auto &cmd : _renderGroups[j])
        // {
        //     cmd->releaseToCommandPool();
        // }
        _renderGroups[j].clear();
    }

    // Clear batch quad commands
    _batchedQuadCommands.clear();
    _numQuads = 0;

    _lastMaterialID = 0;
}

void Renderer::convertToWorldCoordinates(_ccV3F_C4B_T2F_Quad* quads, int quantity, const kmMat4& modelView)
{
//    kmMat4 matrixP, mvp;
//    kmGLGetMatrix(KM_GL_PROJECTION, &matrixP);
//    kmMat4Multiply(&mvp, &matrixP, &modelView);

    for(int i=0; i<quantity; ++i) {
        _ccV3F_C4B_T2F_Quad *q = &quads[i];

        kmVec3 *vec1 = (kmVec3*)&q->bl.vertices;
        kmVec3Transform(vec1, vec1, &modelView);

        kmVec3 *vec2 = (kmVec3*)&q->br.vertices;
        kmVec3Transform(vec2, vec2, &modelView);

        kmVec3 *vec3 = (kmVec3*)&q->tr.vertices;
        kmVec3Transform(vec3, vec3, &modelView);

        kmVec3 *vec4 = (kmVec3*)&q->tl.vertices;
        kmVec3Transform(vec4, vec4, &modelView);
    }
}

void Renderer::drawBatchedQuads()
{
    //TODO we can improve the draw performance by insert material switching command before hand.

    int quadsToDraw = 0;
    int startQuad = 0;

    //Upload buffer to VBO
    if(_numQuads <= 0 || _batchedQuadCommands.empty())
    {
        return;
    }

    if (CCConfiguration::sharedConfiguration()->supportsShareableVAO())
    {
        //Set VBO data
        glBindBuffer(GL_ARRAY_BUFFER, _buffersVBO[0]);

        // option 1: subdata
//        glBufferSubData(GL_ARRAY_BUFFER, sizeof(_quads[0])*start, sizeof(_quads[0]) * n , &_quads[start] );

        // option 2: data
//        glBufferData(GL_ARRAY_BUFFER, sizeof(quads_[0]) * (n-start), &quads_[start], GL_DYNAMIC_DRAW);

        // option 3: orphaning + glMapBuffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(_quads[0]) * (_numQuads), nullptr, GL_DYNAMIC_DRAW);
        void *buf = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        memcpy(buf, _quads, sizeof(_quads[0])* (_numQuads));
        glUnmapBuffer(GL_ARRAY_BUFFER);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        //Bind VAO
        ccGLBindVAO(_quadVAO);
    }
    else
    {
#define kQuadSize sizeof(_quads[0].bl)
        glBindBuffer(GL_ARRAY_BUFFER, _buffersVBO[0]);

        glBufferData(GL_ARRAY_BUFFER, sizeof(_quads[0]) * _numQuads , _quads, GL_DYNAMIC_DRAW);

        //ccGLEnableVertexAttribs(GL::VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);
		ccGLEnableVertexAttribs(kCCVertexAttribFlag_PosColorTex);
		
        // vertices
        glVertexAttribPointer(kCCVertexAttrib_Position, 3, GL_FLOAT, GL_FALSE, kQuadSize, (GLvoid*) offsetof(ccV3F_C4B_T2F, vertices));

        // colors
        glVertexAttribPointer(kCCVertexAttrib_Color, 4, GL_UNSIGNED_BYTE, GL_TRUE, kQuadSize, (GLvoid*) offsetof(ccV3F_C4B_T2F, colors));

        // tex coords
        glVertexAttribPointer(kCCVertexAttrib_TexCoords, 2, GL_FLOAT, GL_FALSE, kQuadSize, (GLvoid*) offsetof(ccV3F_C4B_T2F, texCoords));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffersVBO[1]);
    }

    //Start drawing verties in batch
    //for(auto i = _batchedQuadCommands.begin(); i != _batchedQuadCommands.end(); ++i)
	for (std::vector<QuadCommand*>::iterator cmd = _batchedQuadCommands.begin() ; cmd != _batchedQuadCommands.end(); ++cmd)
    //for(const auto& cmd : _batchedQuadCommands)
    {
        if(_lastMaterialID != (*cmd)->getMaterialID())
        {
            //Draw quads
            if(quadsToDraw > 0)
            {
                glDrawElements(GL_TRIANGLES, (GLsizei) quadsToDraw*6, GL_UNSIGNED_SHORT, (GLvoid*) (startQuad*6*sizeof(_indices[0])) );
				CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, quadsToDraw*6);
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
				CCDirector::sharedDirector()->addDrawTextureIDToVec((*cmd)->getTextureID());
#endif
                _drawnBatches++;
                _drawnVertices += quadsToDraw*6;

                startQuad += quadsToDraw;
                quadsToDraw = 0;
            }

            //Use new material
            (*cmd)->useMaterial();
            _lastMaterialID = (*cmd)->getMaterialID();
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
			_textureID = (*cmd)->getTextureID();
#endif
        }

        quadsToDraw += (*cmd)->getQuadCount();
    }

    //Draw any remaining quad
    if(quadsToDraw > 0)
    {
        glDrawElements(GL_TRIANGLES, (GLsizei) quadsToDraw*6, GL_UNSIGNED_SHORT, (GLvoid*) (startQuad*6*sizeof(_indices[0])) );
		CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, quadsToDraw*6);
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
		CCDirector::sharedDirector()->addDrawTextureIDToVec(_textureID);
#endif
        _drawnBatches++;
        _drawnVertices += quadsToDraw*6;
    }

    if (CCConfiguration::sharedConfiguration()->supportsShareableVAO())
    {
        //Unbind VAO
        ccGLBindVAO(0);
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    _batchedQuadCommands.clear();
    _numQuads = 0;

	
}

void Renderer::flush()
{
    drawBatchedQuads();
    _lastMaterialID = 0;
}

// helpers

bool Renderer::checkVisibility(const kmMat4 &transform, const CCSize &size)
{
    // half size of the screen
    CCSize screen_half = CCDirector::sharedDirector()->getWinSize();
    screen_half.width /= 2;
    screen_half.height /= 2;

    float hSizeX = size.width/2;
    float hSizeY = size.height/2;

    kmVec4 v4world;
	kmVec4 v4local;
    kmVec4Fill(&v4local, hSizeX, hSizeY, 0, 1);
    kmVec4MultiplyMat4(&v4world, &v4local, &transform);

    // center of screen is (0,0)
    v4world.x -= screen_half.width;
    v4world.y -= screen_half.height;

    // convert content size to world coordinates
    float wshw = (std::max)(fabsf(hSizeX * transform.mat[0] + hSizeY * transform.mat[4]), fabsf(hSizeX * transform.mat[0] - hSizeY * transform.mat[4]));
    float wshh = (std::max)(fabsf(hSizeX * transform.mat[1] + hSizeY * transform.mat[5]), fabsf(hSizeX * transform.mat[1] - hSizeY * transform.mat[5]));

    // compare if it in the positive quadrant of the screen
    float tmpx = (fabsf(v4world.x)-wshw);
    float tmpy = (fabsf(v4world.y)-wshh);
    bool ret = (tmpx < screen_half.width && tmpy < screen_half.height);

    return ret;
}

NS_CC_END
