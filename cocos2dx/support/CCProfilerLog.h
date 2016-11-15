#ifndef __CCPROFILERLOG_H__
#define __CCPROFILERLOG_H__
#include "cocoa/CCObject.h"
#include "platform/platform.h"
#include "cocoa/CCDictionary.h"
#include <list>

NS_CC_BEGIN

class CCCallTree;
class CC_DLL CCProfilerLog : public CCObject
{
public:
	CCProfilerLog(void);
	virtual ~CCProfilerLog(void);

	void mergeCallTree(CCCallTree* callTree);
	void saveProfilerLog();
private:	
};
NS_CC_END
#endif