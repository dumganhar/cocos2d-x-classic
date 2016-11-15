#include "CCBatchNodeMgr.h"
#include "sprite_nodes/CCSprite.h"
#include "kazmath/GL/matrix.h"
#include "CCDirector.h"
#include <functional>
#include <vector>
#include <algorithm>

#include "CCEventType.h"
#include "support/CCNotificationCenter.h"
#include "support/CCProfiling.h"
namespace metis
{
	const int VBO_SIZE = 1024 * 6;
	CCBatchNodeMgr* CCBatchNodeMgr::ms_pObjectT = 0;
	struct CCBatchNodeMgr::Pimpl
	{
		typedef class SpriteInfo
		{
		public:
			float				_globalOrder;
			uint32_t			_materialID;
			GLuint				_textureID;
			CCGLProgram*		_shader;
			ccBlendFunc			_blendType;
			ccV3F_C4B_T2F_Quad  _quads;
		public:
			SpriteInfo():_globalOrder(0.0)
				,_materialID(0)
				,_textureID(0)
				,_shader(NULL)
				,_blendType(kCCBlendFuncDisable)
			{
			}

			void init(float globalOrder, GLuint textureID, CCGLProgram* shader, ccBlendFunc& blendType, ccV3F_C4B_T2F_Quad& quads)
			{
				_globalOrder = globalOrder;
				_textureID = textureID;
				_blendType = blendType;
				_shader = shader;
				_quads = quads;

				generateMaterialID();
			}
			void useMaterial() const
			{
				_shader->use();
				_shader->setUniformsForBuiltins();
				ccGLBindTexture2D(_textureID);
				ccGLBlendFunc(_blendType.src, _blendType.dst);
			}

		private:
			void generateMaterialID()
			{
				int blendID = 0;
				if(_blendType.src == GL_ONE && _blendType.dst == GL_ZERO)
				{
					blendID = 0;
				}
				else if(_blendType.src == GL_ONE && _blendType.dst == GL_ONE_MINUS_SRC_ALPHA)
				{
					blendID = 1;
				}
				else if(_blendType.src == GL_SRC_ALPHA && _blendType.dst == GL_ONE_MINUS_SRC_ALPHA)
				{
					blendID = 2;
				}
				else if(_blendType.src == GL_SRC_ALPHA && _blendType.dst == GL_ONE)
				{
					blendID = 3;
				}
				else
				{
					blendID = 4;
				}

				//TODO Material ID should be part of the ID
				//
				// Temporal hack (later, these 32-bits should be packed in 24-bits
				//
				// +---------------------+-------------------+----------------------+
				// | Shader ID (10 bits) | Blend ID (4 bits) | Texture ID (18 bits) |
				// +---------------------+-------------------+----------------------+

				_materialID = (uint32_t)_shader->getProgram() << 22
					| (uint32_t)blendID << 18
					| (uint32_t)_textureID << 0;
			}
		} spriteinfo_t;

		

		class RenderQueue 
		{
		private:
			spriteinfo_t*				_queueNegZ[VBO_SIZE];
			spriteinfo_t*				_queue0[VBO_SIZE];
			spriteinfo_t*				_queuePosZ[VBO_SIZE];
			int							_indexOfNegZ;
			int							_indexOf0;
			int							_indexOfPosZ;
		public:
			RenderQueue():_indexOfNegZ(-1),_indexOf0(-1),_indexOfPosZ(-1)
			{
				memset(_queueNegZ, 0, sizeof(_queueNegZ));
				memset(_queue0, 0, sizeof(_queue0));
				memset(_queuePosZ, 0, sizeof(_queuePosZ));
			}
			void push_back(spriteinfo_t* $spriteInfo)
			{
				float z = $spriteInfo->_globalOrder;
				if(z < 0)
					_queueNegZ[++_indexOfNegZ] = $spriteInfo;
				else if(z > 0)
					_queuePosZ[++_indexOfPosZ] = $spriteInfo;
				else
					_queue0[++_indexOf0] = $spriteInfo;

			}
			int size() const
			{
				 return _indexOf0 + _indexOfNegZ + _indexOfPosZ + 3;
			}

			static bool compareRenderCommand(spriteinfo_t* a, spriteinfo_t* b)
			{
				return a->_globalOrder < b->_globalOrder;
			}
			void sort()
			{
				std::sort(_queueNegZ, _queueNegZ + _indexOfNegZ + 1, compareRenderCommand);
				std::sort(_queuePosZ, _queuePosZ + _indexOfPosZ + 1, compareRenderCommand);
			}
			spriteinfo_t* operator[](int index) const
			{
				if(index < _indexOfNegZ + 1)	
					return _queueNegZ[index];

				index -= (_indexOfNegZ + 1);

				if(index < _indexOf0 + 1)		
					return _queue0[index];

				index -= (_indexOf0 + 1);

				if(index < _indexOfPosZ + 1)	
					return _queuePosZ[index];

				return NULL;
			}
			void clear()
			{
				_indexOfNegZ = -1;
				_indexOf0    = -1;
				_indexOfPosZ = -1;
			}
		};

		struct POD
		{
			RenderQueue									m_renderQueue;

			unsigned int								_lastMaterialID;
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
			GLuint										_textureID;
#endif
			

			ccV3F_C4B_T2F_Quad							_quads[VBO_SIZE];
			GLushort									_indices[6 * VBO_SIZE];
#if CC_TEXTURE_ATLAS_USE_VAO
			GLuint										_VAOname;
#endif
			GLuint										_buffersVBO[2]; //0: vertex  1: indices
			int											_numQuads;

			bool										_bInited;

			kmMat4										_matrixLookup;

			spriteinfo_t								_spriteInfos[VBO_SIZE];
			unsigned int								_spriteIndex;
	
			POD():_lastMaterialID(0),_numQuads(0),_bInited(false),_spriteIndex(0)
			{
			}
			~POD()
			{
			}
		};

		struct Observer:public CCObject
		{
			void init(CCBatchNodeMgr::Pimpl* processor_)
			{
				m_Processor = processor_;

				
#if CC_ENABLE_CACHE_TEXTURE_DATA
				// listen the event when app go to background
				CCNotificationCenter::sharedNotificationCenter()->addObserver(
					this,
					callfuncO_selector(Observer::listenBackToForeground),
					EVENT_COME_TO_FOREGROUND,
					NULL);
#endif
			}

			void unInit()
			{
#if CC_ENABLE_CACHE_TEXTURE_DATA
				CCNotificationCenter::sharedNotificationCenter()->removeObserver(this, EVENT_COME_TO_FOREGROUND);
#endif
			}

			void listenBackToForeground(CCObject *obj)
			{
				if (NULL != m_Processor)
				{
#if CC_TEXTURE_ATLAS_USE_VAO
					m_Processor->setupVBOandVAO();    
#else    
					m_Processor->setupVBO();
#endif
				}
			}

		private:
			CCBatchNodeMgr::Pimpl* m_Processor;
		};
		
		Pimpl()
		{
			Init();
		}
		~Pimpl()
		{
			m_ob.unInit();
			glDeleteBuffers(2, pod._buffersVBO);
#if CC_TEXTURE_ATLAS_USE_VAO
			glDeleteVertexArrays(1, &pod._VAOname);
			ccGLBindVAO(0);
#endif
		}

		void Init()
		{
			if (!pod._bInited)
			{
				m_ob.init(this);			
				setupIndices();
#if CC_TEXTURE_ATLAS_USE_VAO
				setupVBOandVAO();    
#else    
				setupVBO();
#endif
				pod._bInited = true;
			}
			
		}

		void setupIndices()
		{
			for( int i=0; i < VBO_SIZE; i++)
			{
				pod._indices[i*6+0] = (GLushort) (i*4+0);
				pod._indices[i*6+1] = (GLushort) (i*4+1);
				pod._indices[i*6+2] = (GLushort) (i*4+2);
				pod._indices[i*6+3] = (GLushort) (i*4+3);
				pod._indices[i*6+4] = (GLushort) (i*4+2);
				pod._indices[i*6+5] = (GLushort) (i*4+1);
			}
		}

#if CC_TEXTURE_ATLAS_USE_VAO
		void setupVBOandVAO()
		{
			glGenVertexArrays(1, &pod._VAOname);
			ccGLBindVAO(pod._VAOname);

			glGenBuffers(2, &pod._buffersVBO[0]);
			glBindBuffer(GL_ARRAY_BUFFER, pod._buffersVBO[0]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(pod._quads[0]) * VBO_SIZE, pod._quads, GL_DYNAMIC_DRAW);

			glEnableVertexAttribArray(kCCVertexAttrib_Position);
			glVertexAttribPointer(kCCVertexAttrib_Position, 3, GL_FLOAT, GL_FALSE, sizeof(ccV3F_C4B_T2F), (GLvoid*) offsetof( ccV3F_C4B_T2F, vertices));
			glEnableVertexAttribArray(kCCVertexAttrib_Color);
			glVertexAttribPointer(kCCVertexAttrib_Color, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ccV3F_C4B_T2F), (GLvoid*) offsetof( ccV3F_C4B_T2F, colors));
			glEnableVertexAttribArray(kCCVertexAttrib_TexCoords);
			glVertexAttribPointer(kCCVertexAttrib_TexCoords, 2, GL_FLOAT, GL_FALSE, sizeof(ccV3F_C4B_T2F), (GLvoid*) offsetof( ccV3F_C4B_T2F, texCoords));

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pod._buffersVBO[1]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pod._indices[0]) * VBO_SIZE * 6, pod._indices, GL_STATIC_DRAW);

			// Must unbind the VAO before changing the element buffer.
			ccGLBindVAO(0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			CHECK_GL_ERROR_DEBUG();
		}
#else // CC_TEXTURE_ATLAS_USE_VAO
		void setupVBO()
		{
			glGenBuffers(2, &pod._buffersVBO[0]);

			ccGLBindVAO(0);

			glBindBuffer(GL_ARRAY_BUFFER, pod._buffersVBO[0]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(pod._quads[0]) * VBO_SIZE, pod._quads, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pod._buffersVBO[1]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pod._indices[0]) * VBO_SIZE * 6, pod._indices, GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			CHECK_GL_ERROR_DEBUG();
		}
#endif // ! // CC_TEXTURE_ATLAS_USE_VAO

		bool appendChild(CCNode* sprite)
		{
			CCSprite* pSprite = dynamic_cast<CCSprite*>(sprite);
			CCAssert(NULL != pSprite, "AutoBatch must be sprite!");
			if (NULL == pSprite)
			{
				return false;
			}
			
			CCTexture2D* _texture = pSprite->getTexture();
			
			if (NULL != _texture && pod._spriteIndex < VBO_SIZE)
			{
				//int _zOrder = pSprite->getZOrder();
				int _zOrder = 0; //不同层级下的精灵，不能直接用getZOrder来排序，必须使用全局的z，目前2.x版本尚无此属性

				GLuint _name = _texture->getName();
				CCGLProgram* _shader = pSprite->getShaderProgram();
				ccBlendFunc _blendFun = pSprite->getBlendFunc();
				ccV3F_C4B_T2F_Quad _quad = pSprite->getQuad();

				convertToWorldCoordinates(&_quad, pSprite->_modelViewTransform);

				spriteinfo_t* tmp = &pod._spriteInfos[pod._spriteIndex++];
				tmp->init(_zOrder, _name, _shader, _blendFun, _quad);
				AddSpriteInfoToRenderQueue(tmp);

				return true;
			}
			return false;
		}

		void convertToWorldCoordinates(ccV3F_C4B_T2F_Quad* quads, const kmMat4& modelView)
		{
				kmVec3 *vec1 = (kmVec3*)&quads->bl.vertices;
				kmVec3Transform(vec1, vec1, &modelView);

				kmVec3 *vec2 = (kmVec3*)&quads->br.vertices;
				kmVec3Transform(vec2, vec2, &modelView);

				kmVec3 *vec3 = (kmVec3*)&quads->tr.vertices;
				kmVec3Transform(vec3, vec3, &modelView);

				kmVec3 *vec4 = (kmVec3*)&quads->tl.vertices;
				kmVec3Transform(vec4, vec4, &modelView);
		}

		void AddSpriteInfoToRenderQueue(spriteinfo_t* $spriteInfo)
		{
			pod.m_renderQueue.push_back($spriteInfo);

			if (VBO_SIZE == pod.m_renderQueue.size())
			{
				Draw();
			}
		}

		void Draw(void)
		{
			do 
			{
				if(0 == pod.m_renderQueue.size()) break;

				//不同层级下的精灵，不能直接用getZOrder来排序，必须使用全局的z，目前2.x版本尚无此属性
				//所以也无需排序
				//pod.m_renderQueue.sort();

				for (int i = 0; i < pod.m_renderQueue.size(); i++)
				{
					spriteinfo_t* command = pod.m_renderQueue[i];
					memcpy(pod._quads + pod._numQuads, &(command->_quads), sizeof(ccV3F_C4B_T2F_Quad));
					pod._numQuads++;
				}

				kmGLPushMatrix();
				kmGLLoadIdentity();

#if CC_TEXTURE_ATLAS_USE_VAO
				glBindBuffer(GL_ARRAY_BUFFER, pod._buffersVBO[0]);
				glBufferData(GL_ARRAY_BUFFER, sizeof( pod._quads[0]) * pod._numQuads ,  pod._quads, GL_DYNAMIC_DRAW);

				void *buf = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
				memcpy(buf, pod._quads, sizeof(pod._quads[0])* (pod._numQuads));
				glUnmapBuffer(GL_ARRAY_BUFFER);

				glBindBuffer(GL_ARRAY_BUFFER, 0);

				//Bind VAO
				ccGLBindVAO(pod._VAOname);
#else // ! CC_TEXTURE_ATLAS_USE_VAO
#define kQuadSize sizeof(pod._quads[0].bl)
				{
					glBindBuffer(GL_ARRAY_BUFFER, pod._buffersVBO[0]);
					glBufferData(GL_ARRAY_BUFFER, sizeof( pod._quads[0]) * pod._numQuads ,  pod._quads, GL_DYNAMIC_DRAW);


					ccGLEnableVertexAttribs(kCCVertexAttribFlag_PosColorTex);
					glVertexAttribPointer(kCCVertexAttrib_Position, 3, GL_FLOAT, GL_FALSE, kQuadSize, (GLvoid*) offsetof(ccV3F_C4B_T2F, vertices));
					glVertexAttribPointer(kCCVertexAttrib_Color, 4, GL_UNSIGNED_BYTE, GL_TRUE, kQuadSize, (GLvoid*) offsetof(ccV3F_C4B_T2F, colors));
					glVertexAttribPointer(kCCVertexAttrib_TexCoords, 2, GL_FLOAT, GL_FALSE, kQuadSize, (GLvoid*) offsetof(ccV3F_C4B_T2F, texCoords));

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,  pod._buffersVBO[1]);
				}
				
#endif // CC_TEXTURE_ATLAS_USE_VAO
				int startQuad = 0;
				int quadsToDraw = 0;
				for (int i = 0; i < pod.m_renderQueue.size(); i++)
				{
					 spriteinfo_t* command = pod.m_renderQueue[i];

					 if (pod._lastMaterialID != command->_materialID)
					 {
						 if(quadsToDraw > 0)
						 {
							 glDrawElements(GL_TRIANGLES, (GLsizei) quadsToDraw*6, GL_UNSIGNED_SHORT, (GLvoid*) (startQuad*6*sizeof(pod._indices[0])) );
							 CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, quadsToDraw*6);

							 #if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
							 CCDirector::sharedDirector()->addDrawTextureIDToVec(command->_textureID);
							 #endif

							 startQuad += quadsToDraw;
							 quadsToDraw = 0;
						 }

						 command->useMaterial();
						 pod._lastMaterialID = command->_materialID;
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
						pod._textureID = command->_textureID;
#endif
					 }

					 quadsToDraw++;
				}

				//Draw any remaining quad
				if(quadsToDraw > 0)
				{
					glDrawElements(GL_TRIANGLES, (GLsizei) quadsToDraw*6, GL_UNSIGNED_SHORT, (GLvoid*) (startQuad*6*sizeof(pod._indices[0])) );
					CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, quadsToDraw*6);

					#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
					CCDirector::sharedDirector()->addDrawTextureIDToVec(pod._textureID);
					#endif
				}

#if CC_TEXTURE_ATLAS_USE_VAO
				ccGLBindVAO(0);
#else
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
				kmGLPopMatrix();

				pod._numQuads = 0;
				pod._lastMaterialID = 0;
				pod.m_renderQueue.clear();
				pod._spriteIndex = 0;
			} while (false);
		}

		bool Empty()
		{
			return pod._spriteIndex == 0;
		}
		//////////////////////////////////////////////////////////////////////////
		POD	pod;
		Observer m_ob;
	};

	CCBatchNodeMgr::CCBatchNodeMgr():pimpl_(new CCBatchNodeMgr::Pimpl()){}
	CCBatchNodeMgr::~CCBatchNodeMgr(){}

	bool CCBatchNodeMgr::CacheNode(CCNode* srcNode)
	{
		if (CCDirector::sharedDirector()->isEnableAutoBatch())
		{
			return pimpl_->appendChild(srcNode);
		}
		return false;		
	}

	void CCBatchNodeMgr::FlushDraw()
	{
		CC_PROFILER_HELPER;
		pimpl_->Draw();
	}

	CCBatchNodeMgr* CCBatchNodeMgr::GetInstance()
	{
		if (!ms_pObjectT)
		{
			ms_pObjectT = new CCBatchNodeMgr;
		}
		return ms_pObjectT;
	}

	bool CCBatchNodeMgr::Empty()
	{
		return pimpl_->Empty();
	}

}