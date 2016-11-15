/********************************************************************
	created:	2013/10/02
	created:	2:10:2013   14:21
	filename: 	F:\weshoothd\client\trunk\weshoothd\Src\GameLogic\WSPkManager.h
	file ext:	h
	author:		TOMLV
	
	purpose:	
*********************************************************************/
#ifndef CCRender_H_
#define CCRender_H_
#include "platform/CCPlatformMacros.h"
#include "CCRenderCommand.h"
//#include "CCGLProgram.h"
#include "shaders/CCGLProgram.h"
#include "CCGL.h"
#include <vector>
#include <stack>
#include "include/ccTypes.h"
#include "kazmath/kazmath.h"

NS_CC_BEGIN


class QuadCommand;

class RenderQueue {

public:
	void push_back(RenderCommand* command);
	int size() const;
	void sort();
	void sortMaterial();
	RenderCommand* operator[](int index) const;
	void clear();

protected:
	std::vector<RenderCommand*> _queueNegZ;
	std::vector<RenderCommand*> _queue0;
	std::vector<RenderCommand*> _queuePosZ;
};


struct RenderStackElement
{
    int renderQueueID;
    int currentIndex;
};

class GroupCommandManager;

/* Class responsible for the rendering in.

Whenever possible prefer to use `QuadCommand` objects since the renderer will automatically batch them.
 */
class CC_DLL  Renderer
{
public:
    static const int VBO_SIZE = 65536 / 6;
    static const int BATCH_QUADCOMMAND_RESEVER_SIZE = 64;

    Renderer();
    ~Renderer();

    //TODO manage GLView inside Render itself
    void initGLView();

    /** Adds a `RenderComamnd` into the renderer */
    void addCommand(RenderCommand* command);

    /** Adds a `RenderComamnd` into the renderer specifying a particular render queue ID */
    void addCommand(RenderCommand* command, int renderQueue);

    /** Pushes a group into the render queue */
    void pushGroup(int renderQueueID);

    /** Pops a group from the render queue */
    void popGroup();

    /** Creates a render queue and returns its Id */
    int createRenderQueue();

    /** Renders into the GLView all the queued `RenderCommand` objects */
    void render();

    /** Cleans all `RenderCommand`s in the queue */
    void clean();

    /* returns the number of drawn batches in the last frame */
    int getDrawnBatches() const { return _drawnBatches; }
    /* RenderCommands (except) QuadCommand should update this value */
    void addDrawnBatches(int number) { _drawnBatches += number; };
    /* returns the number of drawn triangles in the last frame */
    int getDrawnVertices() const { return _drawnVertices; }
    /* RenderCommands (except) QuadCommand should update this value */
    void addDrawnVertices(int number) { _drawnVertices += number; };

    inline GroupCommandManager* getGroupCommandManager() const { return _groupCommandManager; };

    /** returns whether or not a rectangle is visible or not */
    bool checkVisibility(const kmMat4& transform, const CCSize& size);

	//Draw the previews queued quads and flush previous context
	void flush();

protected:

    void setupIndices();
    //Setup VBO or VAO based on OpenGL extensions
    void setupBuffer();
    void setupVBOAndVAO();
    void setupVBO();
    void mapBuffers();

    void drawBatchedQuads();

   
    
    void visitRenderQueue(const RenderQueue& queue);

    void convertToWorldCoordinates(_ccV3F_C4B_T2F_Quad* quads, int quantity, const kmMat4& modelView);

    std::stack<int> _commandGroupStack;
    
    std::vector<RenderQueue> _renderGroups;

    uint32_t _lastMaterialID;
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	GLuint	_textureID;
#endif

    std::vector<QuadCommand*> _batchedQuadCommands;

    _ccV3F_C4B_T2F_Quad _quads[VBO_SIZE];
    GLushort _indices[6 * VBO_SIZE];
    GLuint _quadVAO;
    GLuint _buffersVBO[2]; //0: vertex  1: indices

    int _numQuads;
    
    bool _glViewAssigned;

    // stats
    int _drawnBatches;
    int _drawnVertices;
    //the flag for checking whether renderer is rendering
    bool _isRendering;
    
    GroupCommandManager* _groupCommandManager;
    


#if CC_ENABLE_CACHE_TEXTURE_DATA
    EventListenerCustom* _cacheTextureListener;
#endif

public:
};

NS_CC_END
#endif
