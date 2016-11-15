#include "CCCallTree.h"
#include "CCCallNode.h"
#include "ccUtils.h"
#include "CCProfiling.h"

NS_CC_BEGIN

CCCallTree::CCCallTree() : m_depth(0), m_maxDepth(0), m_pRoot(NULL), m_pCurCallNode(NULL), m_saveNodeNum(0)
{
	m_thread = pthread_self();
}

CCCallTree::~CCCallTree()
{

}

void CCCallTree::beginCallNode( const char* node )
{
	if (NULL == m_pRoot)
	{
		std::string rootName = CCProfiler::sharedProfiler()->getRootName();
		if (rootName.compare(node) != 0)
		{
			return;
		}

		m_pRoot = new CCCallNode();
		m_pRoot->init(node, this);
		m_pRoot->start();

		m_pCurCallNode = m_pRoot;

		m_depth = 1;
		m_maxDepth = 1;
		m_sStartTime = m_pRoot->getStartTime();
	}
	else
	{
		pthread_t thread = pthread_self();
		if (pthread_equal(thread, m_thread) == 0)
		{
			return;
		}		

		CCCallNode* callNode = m_pCurCallNode->getChild(node);
		if (NULL == callNode)
		{
			callNode = new CCCallNode();
			callNode->init(node, this);
			m_pCurCallNode->addChild(callNode);
		}		
		CC_ASSERT(callNode->getCallTree() == this);
		callNode->start();		

		m_pCurCallNode = callNode;

		++m_depth;
		if (m_depth > m_maxDepth)
		{
			m_maxDepth = m_depth;
		}
	}
}

void CCCallTree::endCallNode( const char* node )
{
    if (NULL != m_pRoot)
    {
        CC_ASSERT(NULL != m_pCurCallNode);
        if (NULL != m_pCurCallNode)
        {
            std::string& name = m_pCurCallNode->getName();
            CC_ASSERT(name.compare(node) == 0);
            
            m_pCurCallNode->end();
            CCCallNode* parent = m_pCurCallNode->getParent();
            m_pCurCallNode = parent;
            
            --m_depth;
            CC_ASSERT(m_depth >= 0);

			if (isComplete())
			{
				m_sEndTime = m_pRoot->getEndTime();
			}
        }
    }
}

pthread_t CCCallTree::getThread()
{
	return m_thread;
}

int CCCallTree::getMaxDepth()
{
	return m_maxDepth;
}

CCCallNode* CCCallTree::getRoot()
{
	return m_pRoot;
}

bool CCCallTree::isComplete()
{
	return m_pRoot != NULL && m_pCurCallNode == NULL && m_depth == 0;
}

void CCCallTree::save()
{
	tinyxml2::XMLDocument *pDoc = new tinyxml2::XMLDocument();
	if (NULL == pDoc)
		return;

	tinyxml2::XMLDeclaration *pDeclaration = pDoc->NewDeclaration("xml version=\"1.0\" encoding=\"UTF-8\"");
	if (NULL == pDeclaration)
	{
		delete pDoc;
		return;
	}
	pDoc->LinkEndChild(pDeclaration);	
	
	//calltree
	tinyxml2::XMLElement* pRootEle = pDoc->NewElement("CallTree");
	if (NULL == pRootEle)
	{
		delete pDoc;
		return;
	}
	
	if (NULL != m_pRoot)
	{
		int callNum = m_pRoot->getCallNum();
		double costTime = m_pRoot->getCostTime();
		double fps = 1000 / costTime * callNum;
		pRootEle->SetAttribute("fps", (int)fps);
		
		char szCostTime[64]= {0};
		sprintf(szCostTime, "%.3f", (float)costTime);
		pRootEle->SetAttribute("totalTime", szCostTime);
		pRootEle->SetAttribute("depth", m_maxDepth);	
	}
	else
	{
		pRootEle->SetAttribute("error", "no profiler data");	
	}
	
	m_saveNodeNum = 0;
	if (NULL != m_pRoot)
	{
		//Recursive save callnode
		m_pRoot->save(pRootEle, pDoc);
	}	

	pRootEle->SetAttribute("nodeNum", m_saveNodeNum);

	std::string info = CCProfiler::sharedProfiler()->getInfo();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	pRootEle->SetAttribute("platform", "windows");
	info = "win32 profiler";
	pRootEle->SetAttribute("info", info.c_str());
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	pRootEle->SetAttribute("platform", "ios");	
	pRootEle->SetAttribute("info", info.c_str());
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	pRootEle->SetAttribute("platform", "android");
	pRootEle->SetAttribute("info", info.c_str());
#endif

	pDoc->LinkEndChild(pRootEle);

	//write file
	std::string path = CCProfiler::sharedProfiler()->getPath();
	path += "feiji_profiler.xml";
	bool bRet = tinyxml2::XML_SUCCESS == pDoc->SaveFile(path.c_str());
	if (!bRet)
	{
		pDoc->PrintError();
	}	

	profiler_save_callback callBack = CCProfiler::sharedProfiler()->getSaveLogCallBack();
	callBack(bRet);
	delete pDoc;
}

void CCCallTree::clear()
{
	if (NULL != m_pRoot)
	{
		m_pRoot->clear();
		delete m_pRoot;
	}
	m_pRoot = NULL;
	m_pCurCallNode = NULL;
	m_depth = 0;
	m_maxDepth = 0;
}

double CCCallTree::getCostTime()
{
	if (NULL != m_pRoot)
	{
		return m_pRoot->getCostTime();
		//return CCTime::timersubCocos2d(&m_sStartTime, &m_sEndTime);
	}
	return -1;
}

void CCCallTree::merge(CCCallTree* callTree)
{
	CCCallNode* root = callTree->getRoot();
	if (NULL == root)
	{
		return;
	}
	if (NULL == m_pRoot)
	{
		m_pRoot = root;
		m_maxDepth = callTree->getMaxDepth();
		return;
	}

	std::string& name = root->getName();
	std::string& myRootName = m_pRoot->getName();
	if (myRootName.compare(name) != 0)
	{
		return;
	}

	int nMaxDepth = callTree->getMaxDepth();
	if (nMaxDepth > m_maxDepth)
	{
		m_maxDepth = nMaxDepth;
	}

	m_pRoot->merge(callTree->getRoot());

	m_sEndTime = callTree->getEndTime();

	callTree->clear();
}

void CCCallTree::increSaveNodeNum()
{
	++m_saveNodeNum;
}

NS_CC_END