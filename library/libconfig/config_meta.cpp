#include "config_section_def.h"
#include "config_meta.h"

void CConfigMeta::BindStruct(utility::TiXmlTieList &tl, bool)
{
	GetItems(CONFIG_SECTION_ALL, tl);
}

bool CConfigMeta::LoadFile(const std::string &FileName, long ConfigSectionIDs /*= CONFIG_SECTION_ALL*/)
{
	utility::TiXmlTieList tl;
	GetItems(ConfigSectionIDs, tl);
	return tl.LoadFile(FileName);
}

bool CConfigMeta::SaveFile(const std::string &FileName, long ConfigSectionIDs /*= CONFIG_SECTION_ALL*/)
{
	utility::TiXmlTieList tl;
	GetItems(ConfigSectionIDs, tl);
	return tl.SaveFile(FileName);
}
