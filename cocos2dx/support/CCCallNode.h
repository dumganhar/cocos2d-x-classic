#ifndef __CCCALLNODE_H__
#define __CCCALLNODE_H__
#include "cocoa/CCObject.h"
#include "platform/platform.h"
#include "cocoa/CCDictionary.h"
#include "support/tinyxml2/tinyxml2.h"
#include <string>
#include <map>

NS_CC_BEGIN

class CCCallTree;
class CCCallNode : public CCObject
{
public:
	CCCallNode(void);
	virtual ~CCCallNode(void);

	void init(const char* node, CCCallTree* callTree);

	void start();
	void end();
	
	void clear();

	void addChild(CCCallNode* child);
	CCCallNode* getChild(const char* name);

	void merge(CCCallNode* callNode);
	void save(tinyxml2::XMLElement* pParentEleMent, tinyxml2::XMLDocument* pDoc);

	std::string& getName(){return m_name;}

	void setParent(CCCallNode* parent);
	CCCallNode* getParent();	
	
	double getCostTime(){return m_costTime;}
	double getCostTime()const {return m_costTime;} 
	int getCallNum(){return m_callNum;}

	struct cc_timeval getStartTime(){return m_sStartTime;}
	struct cc_timeval getEndTime(){return m_sEndTime;}

	CCCallTree* getCallTree(){return m_callTree;}
	void setCallTree(CCCallTree* callTree, bool recursive = false);
private:
	std::string m_name;
	struct cc_timeval m_sStartTime;
	struct cc_timeval m_sEndTime;
	double m_costTime;
	int m_callNum;

	CCCallNode* m_parent;
	std::map<std::string, CCCallNode*> m_child;

	CCCallTree* m_callTree;
};
NS_CC_END
#endif