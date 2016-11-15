/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2010      Stuart Carnie

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
#include "CCProfiling.h"
#include <vector>
#include <list>
#include <algorithm>
#include "CCCallTree.h"
#include "CCProfilerLog.h"
#include "ccUtils.h"

using namespace std;

NS_CC_BEGIN

//#pragma mark - Profiling Categories
/* set to NO the categories that you don't want to profile */
bool kCCProfilerCategorySprite = true;
bool kCCProfilerCategoryBatchSprite = true;
bool kCCProfilerCategoryParticles = true;


static CCProfiler* g_sSharedProfiler = NULL;

CCProfiler* CCProfiler::sharedProfiler(void)
{
    if (! g_sSharedProfiler)
    {
        g_sSharedProfiler = new CCProfiler();
        g_sSharedProfiler->init();
    }

    return g_sSharedProfiler;
}

bool sortByTimer(CCProfilingTimer*  i, CCProfilingTimer*  j)
{
	if (i->getTotalTime() == j->getTotalTime())
	{
		return i->maxTime > j->maxTime;
	}
	return i->getTotalTime() > j->getTotalTime();	
}


void CCProfiler::beginTimingBlock(const char *timerName)
{
	if (m_enable)
	{
		CCProfilingBeginTimingBlock(timerName);

		if (NULL == m_pCallTree)
		{
			m_pCallTree = new CCCallTree();
		}
		
		m_pCallTree->beginCallNode(timerName);
	}	
}

void CCProfiler::endTimingBlock(const char *timerName)
{
	if (m_enable)
	{
		CCProfilingEndTimingBlock(timerName);

		CCAssert(NULL != m_pCallTree, "CCProfiler call tree not found");
		if (NULL != m_pCallTree)
		{
			m_pCallTree->endCallNode(timerName);
		}	
	}	
}

void CCProfiler::resetTimingBlock(const char *timerName)
{
	if (m_enable)
	{
		CCProfilingResetTimingBlock(timerName);
	}
}

CCProfilingTimer* CCProfiler::createAndAddTimerWithName(const char* timerName)
{
    CCProfilingTimer *t = new CCProfilingTimer();
    t->initWithName(timerName);
    m_pActiveTimers->setObject(t, timerName);
    t->release();

    return t;
}

void CCProfiler::releaseTimer(const char* timerName)
{
    m_pActiveTimers->removeObjectForKey(timerName);
}

void CCProfiler::releaseAllTimers()
{
    m_pActiveTimers->removeAllObjects();
}

bool CCProfiler::init()
{
    m_pActiveTimers = new CCDictionary();
	m_enable = false;
	m_enablePerFrameLog = false;
	m_pCallTree = NULL;
	m_pLog = new CCProfilerLog();
	m_logSaveCallBack = NULL;
    return true;
}

CCProfiler::~CCProfiler(void)
{
    CC_SAFE_RELEASE(m_pActiveTimers);
}

void CCProfiler::displayTimers()
{
	if (m_enable)
	{
		std::list<CCProfilingTimer*> listProfilingTimer;

		CCDictElement* pElement = NULL;
		CCDICT_FOREACH(m_pActiveTimers, pElement)
		{
			CCProfilingTimer* timer = (CCProfilingTimer*)pElement->getObject();
			listProfilingTimer.push_back(timer);        
		}

		listProfilingTimer.sort(sortByTimer);
		//std::sort(listProfilingTimer.begin(), listProfilingTimer.end(), sortByTimer);
		for (std::list<CCProfilingTimer*>::iterator iter = listProfilingTimer.begin(); iter != listProfilingTimer.end(); ++iter)
		{
			CCProfilingTimer* timer = *iter;
			CCLog("%s", timer->description());
		}	
	}
}

void CCProfiler::mergeCallTree()
{
	if (m_enable)
	{
		if (NULL != m_pCallTree && m_pCallTree->isComplete())
		{
			m_pLog->mergeCallTree(m_pCallTree);
			m_pCallTree = NULL;
		}	
	}	

	if (NULL != m_pCallTree)
	{
		m_pCallTree->clear();
		delete m_pCallTree;
		m_pCallTree = NULL;
	}	
}

void CCProfiler::setEnable( bool enable )
{
	m_enable = enable;

	if (!m_enable)
	{
		if (NULL != m_pLog)
		{
			m_pLog->saveProfilerLog();
		}	
	}
}

void CCProfiler::enablePerFrameLog( bool enable )
{
	m_enablePerFrameLog = enable;
}




// implementation of CCProfilingTimer

bool CCProfilingTimer::initWithName(const char* timerName)
{
    m_NameStr = timerName;
    numberOfCalls = 0;
    m_dAverageTime = 0.0;
    totalTime = 0.0;
    minTime = 10000.0;
    maxTime = 0.0;
    gettimeofday((struct timeval *)&m_sStartTime, NULL);

    return true;
}

CCProfilingTimer::~CCProfilingTimer(void)
{
    
}

const char* CCProfilingTimer::description()
{
    static char s_szDesciption[256] = {0};
    sprintf(s_szDesciption, "%s: avg time, %fms, max time, %fms, min time, %fms, total time, %fms, calls count %d", m_NameStr.c_str(), m_dAverageTime, maxTime, minTime, totalTime, numberOfCalls);
    return s_szDesciption;
}

void CCProfilingTimer::reset()
{
    numberOfCalls = 0;
    m_dAverageTime = 0;
    totalTime = 0;
    minTime = 10000;
    maxTime = 0;
    gettimeofday((struct timeval *)&m_sStartTime, NULL);
}

void CCProfilingBeginTimingBlock(const char *timerName)
{
    CCProfiler* p = CCProfiler::sharedProfiler();
	if (p->isEnable())
	{
		CCProfilingTimer* timer = (CCProfilingTimer*)p->m_pActiveTimers->objectForKey(timerName);
		if( ! timer )
		{
			timer = p->createAndAddTimerWithName(timerName);
		}

		gettimeofday((struct timeval *)&timer->m_sStartTime, NULL);

		timer->numberOfCalls++;
	}    
}

void CCProfilingEndTimingBlock(const char *timerName)
{
    CCProfiler* p = CCProfiler::sharedProfiler();
	if (p->isEnable())
	{
		CCProfilingTimer* timer = (CCProfilingTimer*)p->m_pActiveTimers->objectForKey(timerName);
		if (NULL == timer)
		{
			return;
		}


		CCAssert(timer, "CCProfilingTimer  not found");

		struct timeval currentTime;
		gettimeofday(&currentTime, NULL);

		double duration = CCTime::timersubCocos2d((struct cc_timeval *)&timer->m_sStartTime, (struct cc_timeval *)&currentTime);

		// milliseconds		
		timer->totalTime += duration;
		timer->m_dAverageTime = timer->totalTime / timer->numberOfCalls;
		timer->maxTime = MAX( timer->maxTime, duration);
		timer->minTime = MIN( timer->minTime, duration);
	}   
}

void CCProfilingResetTimingBlock(const char *timerName)
{
    CCProfiler* p = CCProfiler::sharedProfiler();
	if (p->isEnable())
	{
		CCProfilingTimer *timer = (CCProfilingTimer*)p->m_pActiveTimers->objectForKey(timerName);
		if (NULL == timer)
		{
			return;
		}	

		CCAssert(timer, "CCProfilingTimer not found");

		timer->reset();
	}    
}


CCProfilerHelper::CCProfilerHelper( const char* file, const char* func )
{
	if (CCProfiler::sharedProfiler()->isEnable())
	{
		if (file != NULL && func != NULL)
		{
			std::string fileFullName(file);
			std::string fileName = ccFileName(fileFullName);

			char szInfo[512]= {0};
			sprintf(szInfo, "%s %s", fileName.c_str(), func);
			info = szInfo;
		}
		else
		{
			info = "invalid TimePerf";
		}

		CCProfiler::sharedProfiler()->beginTimingBlock(info.c_str());
	}
}

CCProfilerHelper::~CCProfilerHelper()
{
	if (CCProfiler::sharedProfiler()->isEnable())
	{
		CCProfiler::sharedProfiler()->endTimingBlock(info.c_str());
	}	
}


NS_CC_END

