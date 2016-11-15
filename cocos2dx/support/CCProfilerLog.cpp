#include "CCProfilerLog.h"
#include "CCCallTree.h"

using namespace std;
NS_CC_BEGIN

static list<CCCallTree*>* g_pListCallTree = NULL;

static pthread_t s_profilerLogThread;

static pthread_cond_t		s_SleepCondition;
static pthread_mutex_t      s_SleepMutex;
static pthread_mutex_t      s_asyncCallTreeMutex;

static bool s_quitProfilerLog = false;
static bool s_saveProfilerLog = false;
CCCallTree* s_pFinalCallTree = NULL;

static void* profilerLogThreadFunc(void* data)
{
	while (true)
	{
		CCCallTree* callTree = NULL;

		pthread_mutex_lock(&s_asyncCallTreeMutex);// get async struct from queue
		if (g_pListCallTree->empty())
		{
			pthread_mutex_unlock(&s_asyncCallTreeMutex);

			if (s_saveProfilerLog)
			{
				if (NULL != s_pFinalCallTree)
				{
					s_pFinalCallTree->save();
					s_pFinalCallTree->clear();
					delete s_pFinalCallTree;
					s_pFinalCallTree = NULL;
				}

				s_saveProfilerLog = false;
			}
			

			if (s_quitProfilerLog) 
			{
				break;
			}
			else 
			{
				pthread_cond_wait(&s_SleepCondition, &s_SleepMutex);
				continue;
			}
			continue;
		}
		else
		{
			callTree = g_pListCallTree->front();
			g_pListCallTree->pop_front();
			pthread_mutex_unlock(&s_asyncCallTreeMutex);
		}      

		if (NULL != callTree)
		{
			if (NULL == s_pFinalCallTree)
			{
				s_pFinalCallTree = callTree;
			}
			else
			{
				s_pFinalCallTree->merge(callTree);
				callTree->clear();
				delete callTree;
				callTree = NULL;
			}
		}		
	}

	if( g_pListCallTree != NULL )
	{
		delete g_pListCallTree;
		g_pListCallTree = NULL;

		pthread_mutex_destroy(&s_asyncCallTreeMutex);
		pthread_mutex_destroy(&s_SleepMutex);
		pthread_cond_destroy(&s_SleepCondition);
	}

	return 0;
}

CCProfilerLog::CCProfilerLog()
{

}

CCProfilerLog::~CCProfilerLog()
{
	s_quitProfilerLog = true;

	pthread_cond_signal(&s_SleepCondition);
}

void CCProfilerLog::mergeCallTree( CCCallTree* callTree )
{
	// lazy init
	if (g_pListCallTree == NULL)
	{             
		g_pListCallTree = new list<CCCallTree*>();    

		pthread_mutex_init(&s_asyncCallTreeMutex, NULL);
		pthread_mutex_init(&s_SleepMutex, NULL);
		pthread_cond_init(&s_SleepCondition, NULL);
		pthread_create(&s_profilerLogThread, NULL, profilerLogThreadFunc, NULL);
	}

	// add calltree into list
	pthread_mutex_lock(&s_asyncCallTreeMutex);
	g_pListCallTree->push_back(callTree);
	pthread_mutex_unlock(&s_asyncCallTreeMutex);

	pthread_cond_signal(&s_SleepCondition);
}

void CCProfilerLog::saveProfilerLog()
{
	if (NULL != g_pListCallTree)
	{
		s_saveProfilerLog = true;
		pthread_cond_signal(&s_SleepCondition);
	}
}

NS_CC_END