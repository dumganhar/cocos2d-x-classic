#pragma once
#include "CCNode.h"
#include <memory>
USING_NS_CC;

namespace metis
{
	
	class CCBatchNodeMgr
	{
	public:
		explicit CCBatchNodeMgr();
		~CCBatchNodeMgr();

		bool CacheNode(cocos2d::CCNode* srcNode);
		void FlushDraw();
		bool Empty();
		static CCBatchNodeMgr* GetInstance();
	private:
		static CCBatchNodeMgr* ms_pObjectT;
	private:
		struct Pimpl;
		std::auto_ptr<Pimpl> pimpl_;
	};
	
}