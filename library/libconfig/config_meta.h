#pragma once

#include "tixml_helper.h"

class CConfigMeta
{
public:
	void BindStruct( utility::TiXmlTieList &tl, bool );
	bool LoadFile( const std::string &FileName, long ConfigSectionIDs);
	bool SaveFile( const std::string &FileName, long ConfigSectionIDs);
	virtual void GetItems(long SectionIDs, utility::TiXmlTieList &tl) = 0;
};