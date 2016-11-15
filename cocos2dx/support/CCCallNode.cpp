#include "CCCallNode.h"
#include "CCCallTree.h"
#include <set>

using namespace std;
NS_CC_BEGIN
extern CCCallTree*  s_pFinalCallTree;

struct SCallNodeComp  
{  
	bool operator()(const CCCallNode* left, const CCCallNode* right)  
	{  
		if (left == right)
		{
			return false;
		}
		return left->getCostTime() > right->getCostTime();
	}  
};  

CCCallNode::CCCallNode() : m_parent(NULL), m_costTime(0), m_callNum(0), m_callTree(NULL)
{

}

CCCallNode::~CCCallNode()
{

}

void CCCallNode::init( const char* node, CCCallTree* callTree )
{
	m_name = node;
	m_callTree = callTree;
}

void CCCallNode::addChild( CCCallNode* child )
{
	child->setParent(this);	
	m_child.insert( make_pair(child->getName(), child) );
}

void CCCallNode::setParent( CCCallNode* parent )
{
	m_parent = parent;
}

CCCallNode* CCCallNode::getParent()
{
	return m_parent;
}

void CCCallNode::clear()
{
	for (std::map<std::string, CCCallNode*>::iterator iter = m_child.begin(); iter != m_child.end(); ++iter)
	{
		CCCallNode* child = iter->second;
		child->clear();

		delete child;
	}
	m_child.clear();

	m_parent = NULL;
}

void CCCallNode::start()
{
	CCTime::gettimeofdayCocos2d(&m_sStartTime, NULL);
	++m_callNum;
}

void CCCallNode::end()
{
	CCTime::gettimeofdayCocos2d(&m_sEndTime, NULL);

	m_costTime += CCTime::timersubCocos2d((struct cc_timeval *)&m_sStartTime, (struct cc_timeval *)&m_sEndTime);
}

void CCCallNode::save( tinyxml2::XMLElement* pParentEleMent, tinyxml2::XMLDocument* pDoc )
{
	m_callTree->increSaveNodeNum();

	tinyxml2::XMLElement* pElement = pDoc->NewElement("CallNode");
	pElement->SetAttribute("name", m_name.c_str());
	
	double percent = m_costTime;
	percent *= 100;
	percent /= m_callTree->getCostTime();
	if (percent > 100)
	{
		percent = 100;
	}
	if (percent < 0)
	{
		percent = 0;
	}
	char szPercent[64]= {0};
	sprintf(szPercent, "%.3f", (float)percent);
	pElement->SetAttribute("percent", szPercent);

	pElement->SetAttribute("time", m_costTime);	
	pElement->SetAttribute("callNum", m_callNum);
	pElement->SetAttribute("childNum", (int)(m_child.size()));


	std::set<CCCallNode*, SCallNodeComp> setChild;
	for (std::map<std::string, CCCallNode*>::iterator iter = m_child.begin(); iter != m_child.end(); ++iter)
	{
		CCCallNode* child = iter->second;	
		setChild.insert(child);
	}

	for (std::set<CCCallNode*, SCallNodeComp>::iterator iter = setChild.begin(); iter != setChild.end(); ++iter)
	{
		CCCallNode* child = *iter;	
		child->save(pElement, pDoc);
	}

	pParentEleMent->LinkEndChild(pElement);
}

CCCallNode* CCCallNode::getChild( const char* name )
{
	std::map<std::string, CCCallNode*>::iterator iter = m_child.find(name);
	if (iter != m_child.end())
	{
		return iter->second;
	}
	return NULL;
}

void CCCallNode::merge( CCCallNode* callNode )
{
	string& name = callNode->getName();
	CC_ASSERT(m_name.compare(name) == 0);

	m_costTime += callNode->getCostTime();
	m_callNum += callNode->getCallNum();

	callNode->setParent(m_parent);
	callNode->setCallTree(m_callTree);

	//Recursive merge child
	for (map<std::string, CCCallNode*>::iterator iter = callNode->m_child.begin(); iter != callNode->m_child.end(); ++iter)
	{
		string name = iter->first;
		CCCallNode* child = getChild(name.c_str());
		if (NULL == child)
		{
			(iter->second)->setCallTree(m_callTree, true);
			addChild(iter->second);
		}
		else
		{
			child->merge(iter->second);
			delete iter->second;
		}
	}
	callNode->m_child.clear();

	for (map<std::string, CCCallNode*>::iterator iter = m_child.begin(); iter != m_child.end(); ++iter)
	{
		CCCallNode* child = iter->second;
		CC_ASSERT(child->getCallTree() == s_pFinalCallTree);
	}
}

void CCCallNode::setCallTree( CCCallTree* callTree, bool recursive /*= false*/ )
{
	m_callTree = callTree;

	if (recursive)
	{
		for (map<std::string, CCCallNode*>::iterator iter = m_child.begin(); iter != m_child.end(); ++iter)
		{
			CCCallNode* child = iter->second;
			child->setCallTree(callTree, recursive);
		}
	}
}

NS_CC_END