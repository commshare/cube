#include "tixml_helper.h"

namespace utility
{
	TiXmlTieList& TiXmlTieList::AddItem(TiXmlTieBase * Var)
	{
		m_TiXmlVars.push_back(std::shared_ptr<TiXmlTieBase>(Var));
		return *this;
	}

	TiXmlTieList& TiXmlTieList::AddItem(TiXmlElement & var, const std::string & Name)
	{
		return AddItem(new TiXmlTieTiXmlElement(var, Name));
	}

	TiXmlTieList& TiXmlTieList::AddItem(TiXmlTieList & var, const std::string & Name)
	{
		return AddItem(new TiXmlTieTieList(var, Name));
	}

	bool utility::TiXmlTieList::RemoveItem(const std::string &Name)
	{
		for (tie_iterator it = m_TiXmlVars.begin(); it != m_TiXmlVars.end(); ++it)
		{
			if ((*it)->m_NodeName == Name)
			{
				m_TiXmlVars.erase(it);
				return true;
			}
		}
		return false;
	}

	bool TiXmlTieList::FromTiXmlNode(const TiXmlElement * Node)
	{
		for (tie_iterator it = m_TiXmlVars.begin(); it != m_TiXmlVars.end(); ++it)
		{
			(*it)->SetValue(Node);
		}
		return true;
	}

	bool TiXmlTieList::LinkTiXmlNode(TiXmlElement * Node)
	{
		if (Node)
		{
			for (tie_iterator it = m_TiXmlVars.begin(); it != m_TiXmlVars.end(); ++it)
			{
				(*it)->LinkChild(Node);
			}
			return true;
		}
		return false;
	}

	bool TiXmlTieList::LoadXmlText(const std::string &xml)
	{
		TiXmlDocument doc;
		if (doc.Parse(xml.c_str()))
		{
			return FromTiXmlNode(doc.FirstChildElement());
		}
		return false;
	}

	bool TiXmlTieList::SaveXmlText(std::string &xml, const std::string Name /*= "root"*/, const std::string &Encoding /*= "GB2312"*/)
	{
		TiXmlDocument doc;
		TiXmlElement * Node = new TiXmlElement(Name.c_str());
		doc.LinkEndChild(Node);

		if (LinkTiXmlNode(Node))
		{
			TiXmlPrinter printer;
			doc.Accept(&printer);

			xml = printer.CStr();
			return true;
		}
		return false;
	}

	bool TiXmlTieList::LoadFile(const std::string &FileName)
	{
		TiXmlDocument doc;
		if (doc.LoadFile(FileName.c_str()))
		{
			return FromTiXmlNode(doc.FirstChildElement());
		}
		return false;
	}

	bool TiXmlTieList::SaveFile(const std::string &FileName, const std::string &Encoding /*= "GB2312"*/)
	{
		TiXmlDocument doc;
		doc.LinkEndChild(new TiXmlDeclaration("1.0", Encoding.c_str(), ""));
		TiXmlElement * Node = new TiXmlElement("root");
		doc.LinkEndChild(Node);

		if (LinkTiXmlNode(Node) && doc.SaveFile(FileName.c_str()))
		{
			return true;
		}
		return false;
	}

	void utility::TiXmlTieList::clear()
	{
		m_TiXmlVars.clear();
	}

	bool utility::TiXmlTieList::empty() const
	{
		return m_TiXmlVars.empty();
	}
	//////////////////////////////////////////////////////////////////////////

	TiXmlTieTieList::TiXmlTieTieList(const TiXmlTieList &TieList, const std::string &Name)
		: TiXmlTieBase(Name)
		, m_TieList(TieList)
	{
	}

	void TiXmlTieTieList::SetValue(const TiXmlNode * Parent)
	{
		m_TieList.FromTiXmlNode(Parent ? Parent->FirstChildElement(m_NodeName.c_str()) : NULL);
	}

	TiXmlNode * TiXmlTieTieList::LinkChild(TiXmlNode * Parent)
	{
		if (Parent)
		{
			TiXmlElement * Node = new TiXmlElement(m_NodeName.c_str());
			m_TieList.LinkTiXmlNode(Node);
			return Parent->LinkEndChild(Node);
		}
		return NULL;
	}

	TiXmlTieTiXmlElement::TiXmlTieTiXmlElement(TiXmlElement &element, const std::string &Name)
		: TiXmlTieBase(Name)
		, m_TiXmlElement(element)
	{
	}

	void TiXmlTieTiXmlElement::SetValue(const TiXmlNode * Parent)
	{
		const TiXmlElement * Node;
		if (Parent && (Node = Parent->FirstChildElement(m_NodeName.c_str())))
		{
			m_TiXmlElement = *Node;
		}
		else
		{
			m_TiXmlElement = *std::shared_ptr<TiXmlElement>(new TiXmlElement(m_NodeName.c_str()));
		}
	}

	TiXmlNode * TiXmlTieTiXmlElement::LinkChild(TiXmlNode * Parent)
	{
		if (Parent)
		{
			TiXmlElement * elem = new TiXmlElement(m_NodeName.c_str());
			*elem = m_TiXmlElement;
			elem->SetValue(m_NodeName.c_str());
			return Parent->LinkEndChild(elem);
		}
		return NULL;
	}

};
