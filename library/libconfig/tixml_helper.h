#pragma once
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <map>
#include <memory>
#include <sstream>
#include "tinyxml.h"
#include "esunny_datatype_flag.h"

namespace utility
{
	template <int v> struct Int2Type {enum { value = v };};

	template<typename Source>
	std::string to_str(const Source &v)
	{
		std::stringstream s;
		s << v;
		return s.str();
	}

	template<>
	inline std::string to_str(const std::string &v)
	{
		return v;
	}

	template<typename Target>
	Target my_cast(const std::string &str)
	{
		std::stringstream s(str);
		Target val;
		return (s >> val).fail()?Target():val;
	}

	template<typename Target>
	inline Target my_cast(const char * v)
	{
		return v?my_cast<Target>(std::string(v)):Target();
	}

	template<>
	inline std::string my_cast(const std::string &str)
	{
		return str;
	}

	template<class T>
	void assignment(T& target, T& source)
	{
		target = source;
	}


	template<class T, size_t N>
	void assignment(T(& target)[N], T(& source)[N])
	{
		for(size_t i = 0; i < N; i++)
		{
			assignment(target[i], source[i]);
		}
	}

	template<class T>
	void assignDefault(T& target)
	{
		target = T();
	}


	template<class T, size_t N>
	void assignDefault(T(& target)[N])
	{
		for(size_t i = 0; i < N; i++)
		{
			assignDefault(target[i]);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	struct TiXmlTieBase
	{
		std::string m_NodeName;

		TiXmlTieBase(const std::string &Name)
			: m_NodeName(Name) {}
		virtual ~TiXmlTieBase() {}

		virtual void SetValue(const TiXmlNode * Parent) = 0;
		virtual TiXmlNode * LinkChild(TiXmlNode * Parent) = 0;
	};

	template<class T, bool IsCustom = Conversion<T, TiXmlHelperSimple>::exists>
	struct TiXmlBind
	{
		enum { IsSimple = true };
		typedef T value_type;

		static TiXmlElement *GetTiXml(const T &val, const std::string & Name = "item")
		{
			TiXmlElement * Node = new TiXmlElement(Name.c_str());
			Node->LinkEndChild(new TiXmlText(val.ValueToString().c_str()));
			return Node;
		}

		static T& TiXmlGetValue(const TiXmlNode * Node, T &Value)
		{
			if (Node) Node = Node->FirstChild();
			return Value.ValueFromString(Node ? Node->Value() : "");
		}

		inline static std::string ValueToString(const T &Value)
		{
			return Value.ValueToString();
		}

		inline static T& ValueFromString(const std::string &str, T &Value)
		{
			return Value.ValueFromString(str);
		}
	};

	template<class T> struct BindSelect
	{
		typedef TiXmlBind<T> bind_type;
	};

	template<class T> struct TiXmlBindSimple
	{
		typedef T value_type;
		enum { IsSimple = true };

		static TiXmlElement *GetTiXml(const T &Value, const std::string & Name = "item")
		{
			TiXmlElement * Node = new TiXmlElement(Name.c_str());
			Node->LinkEndChild(new TiXmlText(to_str(Value).c_str()));
			return Node;
		}

		static T& TiXmlGetValue(const TiXmlNode * Node, T &Value)
		{
			if (Node) Node = Node->FirstChild();
			if (Node)
				Value = my_cast<T>(Node->Value());
			else
				Value = T();
			return Value;
		}

		inline static std::string ValueToString(const T &Value)
		{
			return to_str(Value);
		}

		inline static T& ValueFromString(const std::string &str, T &Value)
		{
			Value = my_cast<T>(str);
			return Value;
		}
	};

	template<> struct BindSelect<std::string>
	{
		typedef TiXmlBindSimple<std::string> bind_type;
	};

	template<> struct BindSelect<short>
	{
		typedef TiXmlBindSimple<short> bind_type;
	};

	template<> struct BindSelect<unsigned short>
	{
		typedef TiXmlBindSimple<unsigned short> bind_type;
	};

	template<> struct BindSelect<int>
	{
		typedef TiXmlBindSimple<int> bind_type;
	};

	template<> struct BindSelect<char>
	{
		typedef TiXmlBindSimple<char> bind_type;
	};

	template<> struct BindSelect<unsigned int>
	{
		typedef TiXmlBindSimple<unsigned int> bind_type;
	};

	template<> struct BindSelect<long>
	{
		typedef TiXmlBindSimple<long> bind_type;
	};

	template<> struct BindSelect<unsigned long>
	{
		typedef TiXmlBindSimple<unsigned long> bind_type;
	};

	template<> struct BindSelect<bool>
	{
		typedef TiXmlBindSimple<bool> bind_type;
	};

	template<> struct BindSelect<float>
	{
		typedef TiXmlBindSimple<float> bind_type;
	};

	template<> struct BindSelect<double>
	{
		typedef TiXmlBindSimple<double> bind_type;
	};

	template<class T> struct TiXmlBindSequenceSet
	{
		typedef T value_type;
		typedef typename T::value_type element_type;
		enum { IsSimple = false };

		static TiXmlElement *GetTiXml(const T &val, const std::string & Name = "sequence")
		{
			TiXmlElement * Node = new TiXmlElement(Name.c_str());
			for (typename T::const_iterator it = val.begin(); it != val.end(); ++it)
			{
				Node->LinkEndChild(BindSelect<element_type>::bind_type::GetTiXml(*it));
			}
			return Node;
		}

		static T& TiXmlGetValue(const TiXmlNode * Node, T &Value)
		{
			Value.clear();
			if (Node) Node = Node->FirstChild();
			for (; Node; Node = Node->NextSibling())
			{
				element_type ev;
				Value.insert(Value.end(), BindSelect<element_type>::bind_type::TiXmlGetValue(Node, ev));
			}
			return Value;
		}
	};

	template<class T> struct BindSelect<std::vector<T> >
	{
		typedef TiXmlBindSequenceSet<std::vector<T> > bind_type;
	};

	template<class T> struct BindSelect<std::list<T> >
	{
		typedef TiXmlBindSequenceSet<std::list<T> > bind_type;
	};

	template<class T> struct BindSelect<std::deque<T> >
	{
		typedef TiXmlBindSequenceSet<std::deque<T> > bind_type;
	};

	template<class T> struct BindSelect<std::set<T> >
	{
		typedef TiXmlBindSequenceSet<std::set<T> > bind_type;
	};

	template<class T> struct TiXmlBindMap
	{
		typedef T value_type;
		typedef typename T::key_type key_type;
		typedef typename T::mapped_type mapped_type;
		enum { IsSimple = false };

		static TiXmlElement *GetTiXml(const T &val, const std::string & Name = "map")
		{
			TiXmlElement * Node = new TiXmlElement(Name.c_str());
			for (typename T::const_iterator it = val.begin(); it != val.end(); ++it)
			{
				TiXmlElement * Child = BindSelect<mapped_type>::bind_type::GetTiXml(it->second);
				Child->SetAttribute("_key", BindSelect<key_type>::bind_type::ValueToString(it->first).c_str());
				Node->LinkEndChild(Child);
			}
			return Node;
		}

		static T& TiXmlGetValue(const TiXmlNode * Node, T &Value)
		{
			Value.clear();
			if (Node) Node = Node->FirstChild();
			for (; Node; Node = Node->NextSibling())
			{
				const TiXmlElement *elem = Node->ToElement();
				if (elem)
				{
					const char * keystr = elem->Attribute("_key");
					if (NULL == keystr) keystr = Node->Value();
					std::pair<key_type, mapped_type> val;
					BindSelect<key_type>::bind_type::ValueFromString(keystr, val.first);
					BindSelect<mapped_type>::bind_type::TiXmlGetValue(Node, val.second);
					Value.insert(val);
				}
			}
			return Value;
		}
	};

	template<class K, class V>
	struct BindSelect<std::map<K, V> >
	{
		typedef TiXmlBindMap<std::map<K, V> > bind_type;
	};

	template<class T, size_t N> struct TiXmlBindArray
	{
		typedef T value_type[N];
		typedef T element_type;
		enum { IsSimple = false };

		static TiXmlElement *GetTiXml(const T(&val)[N], const std::string & Name = "sequence")
		{
			TiXmlElement * Node = new TiXmlElement(Name.c_str());
			for (size_t i = 0; i < N; i++)
			{
				Node->LinkEndChild(BindSelect<element_type>::bind_type::GetTiXml(val[i]));
			}
			return Node;
		}

		static void TiXmlGetValue(const TiXmlNode * Node, value_type Value)
		{
			if (Node) Node = Node->FirstChild();
			for (size_t i = 0; i < N; i++)
			{
				element_type ev;
				if (Node)
				{
					BindSelect<element_type>::bind_type::TiXmlGetValue(Node, ev);
					assignment(Value[i], ev);
					Node = Node->NextSibling();
				}
				else
				{
					assignDefault(Value[i]);
				}
			}
		}
	};

	template<size_t N> struct TiXmlBindArray<char, N>
	{
		typedef char value_type[N];
		typedef char element_type;
		enum { IsSimple = false };

		static TiXmlElement *GetTiXml(const char(&val)[N], const std::string & Name = "sequence")
		{
			TiXmlElement * Node = new TiXmlElement(Name.c_str());
			Node->LinkEndChild(new TiXmlText(to_str(val).c_str()));
			return Node;
		}

		static void TiXmlGetValue(const TiXmlNode * Node, value_type Value)
		{
			if (Node) Node = Node->FirstChild();
			memset(Value, 0x0, N);
			if (Node)
			{
				strncpy(Value, Node->Value(), N - 1);
			}
		}
	};

	template<class T, size_t N> struct BindSelect<T[N]>
	{
		typedef TiXmlBindArray<T, N> bind_type;
	};

	template<class T, bool bAttribute>
	struct TiXmlTie : public TiXmlTieBase
	{
		typedef typename T::value_type element_type;

		TiXmlTie(element_type &Ref, const std::string &Name, const element_type &defaultValue)
			: TiXmlTieBase(Name)
			, m_RefValue(Ref)
			, m_defaultValue(defaultValue)
		{}

		TiXmlTie(element_type &Ref, const std::string &Name)
			: TiXmlTieBase(Name)
			, m_RefValue(Ref)
		{
		}
		element_type &m_RefValue;
		element_type m_defaultValue;

		bool SetValue(const TiXmlNode * Parent, Int2Type<true>)
		{
			const TiXmlElement *p = Parent->ToElement();
			if (p)
			{
				const char *attstr = p->Attribute(m_NodeName.c_str());
				if (NULL != attstr)
				{
					BindSelect<element_type>::bind_type::ValueFromString(attstr, m_RefValue);
				}
				else
				{
					assignment(m_RefValue, m_defaultValue);
				}
				return true;
			}
			return false;
		}

		bool SetValue(const TiXmlNode * Parent, Int2Type<false>)
		{
			const TiXmlElement * Node;
			if ((Node = Parent->FirstChildElement(m_NodeName.c_str())))
			{
				T::TiXmlGetValue(Node, m_RefValue);
				return true;
			}
			return false;
		}

		virtual void SetValue(const TiXmlNode * Parent)
		{
			if (!(Parent && SetValue(Parent, Int2Type<bAttribute>())))
			{
				assignment(m_RefValue, m_defaultValue);
			}
		}

		TiXmlNode * LinkChild(TiXmlNode * Parent, Int2Type<true>)
		{
			TiXmlElement *p = Parent->ToElement();
			if (p) p->SetAttribute(m_NodeName.c_str(), BindSelect<element_type>::bind_type::ValueToString(m_RefValue).c_str());
			return NULL;
		}

		TiXmlNode * LinkChild(TiXmlNode * Parent, Int2Type<false>)
		{
			return Parent->LinkEndChild(T::GetTiXml(m_RefValue, m_NodeName.c_str()));
		}

		virtual TiXmlNode * LinkChild(TiXmlNode * Parent)
		{
			return (Parent) ? LinkChild(Parent, Int2Type<bAttribute>()) : NULL;
		}
	};

	struct TiXmlTieList
	{
		std::list<std::shared_ptr<TiXmlTieBase> > m_TiXmlVars;

		typedef std::list<std::shared_ptr<TiXmlTieBase> >::iterator tie_iterator;

		TiXmlTieList& AddItem(TiXmlTieBase * Var);

		TiXmlTieList& AddItem(TiXmlTieList & var, const std::string & Name);

		TiXmlTieList& AddItem(TiXmlElement & var, const std::string & Name);

		template<class T>
		TiXmlTieList& AddItem(T & var, const std::string & Name, const T &defaultVal = T())
		{
			return AddItem(new TiXmlTie<typename BindSelect<T>::bind_type, false>(var, Name, defaultVal));
		}

		template<class T, size_t N>
		TiXmlTieList& AddItem(T(&var)[N], const std::string & Name)
		{
			return AddItem(new TiXmlTie<typename BindSelect<T[N]>::bind_type, false>(var, Name));
		}

		template<class T>
		TiXmlTieList& AddAttr(T & var, const std::string & Name, const T &defaultVal = T())
		{
			return AddItem(new TiXmlTie<typename BindSelect<T>::bind_type, BindSelect<T>::IsSimple>(var, Name, defaultVal));
		}

		bool RemoveItem(const std::string &Name);

		bool FromTiXmlNode(const TiXmlElement * Node);

		bool LinkTiXmlNode(TiXmlElement * Node);

		bool LoadXmlText(const std::string &xml);

		bool SaveXmlText(std::string &xml, const std::string Name = "root", const std::string &Encoding = "GB2312");

		bool LoadFile(const std::string &FileName);

		bool SaveFile(const std::string &FileName, const std::string &Encoding = "GB2312");

		void clear();

		bool empty() const;
	};

	template<class T> struct TiXmlBind<T, false>
	{
		typedef T value_type;
		enum { IsSimple = false };

		static TiXmlElement *GetTiXml(const T &s, const std::string & Name = "item")
		{
			TiXmlElement * Node = new TiXmlElement(Name.c_str());
			TiXmlTieList tl;
			const_cast<T*>(&s)->BindStruct(tl, true);
			tl.LinkTiXmlNode(Node);
			return Node;
		}

		static T& TiXmlGetValue(const TiXmlNode * Node, T &Value)
		{
			TiXmlTieList tl;
			Value.BindStruct(tl, false);
			tl.FromTiXmlNode(Node->ToElement());
			return Value;
		}

	};


	//////////////////////////////////////////////////////////////////////////
	template<class T>
	TiXmlElement * VarToTiXml(const T &val, const std::string & Name)
	{
		return BindSelect<T>::bind_type::GetTiXml(val, Name);
	}

	template<class T>
	TiXmlNode * VarToTiXmlChild(const T &val, TiXmlNode * Node, const std::string & Name)
	{
		if (Node) return Node->LinkEndChild(VarToTiXml(val, Name)); else return NULL;
	}

	template<class T>
	bool VarToXmlFile(const T &val, const std::string & FileName, const std::string &Encoding = "GB2312")
	{
		TiXmlDocument doc;
		doc.LinkEndChild( new TiXmlDeclaration( "1.0", Encoding.c_str(), "" ) );
		doc.LinkEndChild(VarToTiXml(val, "root"));
		return doc.SaveFile(FileName.c_str());
	}
	template<class T>
	bool VarToXmlString(const T &val, std::string& XmlString, const std::string& RootName = "root", const std::string &Encoding = "GB2312")
	{
		TiXmlDocument doc;
		doc.LinkEndChild( new TiXmlDeclaration( "1.0", Encoding.c_str(), "" ) );
		doc.LinkEndChild(VarToTiXml(val, RootName));
		TiXmlPrinter printer;
		doc.Accept( &printer );
		XmlString = printer.CStr();
		return true;
	}
	template<class T>
	T & TiXmlToVar(T &val, const TiXmlNode * Node)
	{
		return BindSelect<T>::bind_type::TiXmlGetValue(Node, val);
	}

	template<class T>
	void TiXmlChildToVar(T &val, const TiXmlNode * Node, const std::string & Name)
	{
		if (Node) TiXmlToVar(val, Node->FirstChild(Name.c_str())); else val = T();
	}

	template<class T>
	bool XmlFileToVar(T &val, const std::string & FileName)
	{
		TiXmlDocument doc;
		TiXmlNode * Node;
		if (doc.LoadFile(FileName.c_str()) && (Node = doc.FirstChildElement()))
		{
			TiXmlToVar(val, Node);
			return true;
		}
		return false;
	}
	template<class T>
	bool XmlStringToVar(T &val, const std::string & XmlString)
	{
		TiXmlDocument doc;
		TiXmlNode * Node;
		if (doc.Parse(XmlString.c_str()) && (Node = doc.FirstChildElement()))
		{
			TiXmlToVar(val, Node);
			return true;
		}
		return false;
	}

	template<class T>
	void CopyVarViaTiXml(T &From, T &To)
	{
		typedef typename BindSelect<T>::bind_type bind_type;
		TiXmlElement * Node = bind_type::GetTiXml(From);
		bind_type::TiXmlGetValue(Node, To);
		delete Node;
	}

	//////////////////////////////////////////////////////////////////////////
	struct TiXmlTieTiXmlElement : public TiXmlTieBase
	{
		TiXmlElement &m_TiXmlElement;

		TiXmlTieTiXmlElement(TiXmlElement &element, const std::string &Name);

		virtual void SetValue(const TiXmlNode * Parent);

		virtual TiXmlNode * LinkChild(TiXmlNode * Parent);
	};

	struct TiXmlTieTieList
		: public TiXmlTieBase
	{
		TiXmlTieList m_TieList;
		TiXmlTieTieList(const TiXmlTieList &TieList, const std::string &Name);

		virtual void SetValue(const TiXmlNode * Parent);

		virtual TiXmlNode * LinkChild(TiXmlNode * Parent);
	};
}
