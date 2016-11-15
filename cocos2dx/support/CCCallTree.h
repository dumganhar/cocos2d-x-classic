#ifndef __CCCALLTREE_H__
#define __CCCALLTREE_H__
#include "cocoa/CCObject.h"
#include "platform/platform.h"
#include <pthread.h>
NS_CC_BEGIN
class CCCallNode;

class CCCallTree : public CCObject
{
public:
	CCCallTree(void);
	virtual ~CCCallTree(void);

	void beginCallNode(const char* node);
	void endCallNode(const char* node); 

	bool isComplete();
	void save();
	void clear();
	void merge(CCCallTree* callTree);

	void increSaveNodeNum();

	double getCostTime();
	int getMaxDepth();
	CCCallNode* getRoot();
	pthread_t getThread();

	struct cc_timeval getStartTime(){return m_sStartTime;}
	struct cc_timeval getEndTime(){return m_sEndTime;}
private:
	int m_depth;
	int m_maxDepth;
	CCCallNode* m_pRoot;
	CCCallNode* m_pCurCallNode;

	int m_saveNodeNum;

	struct cc_timeval m_sStartTime;
	struct cc_timeval m_sEndTime;

	pthread_t m_thread;
};
NS_CC_END
#endif