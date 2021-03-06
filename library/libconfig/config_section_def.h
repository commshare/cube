#pragma once

#define DEFINECONFIGITEM(SECTIONID, ITEMNAME) if (SectionIDs & SECTIONID)tl.AddItem(ITEMNAME, #ITEMNAME, ITEMNAME);

#define CONFIG_SECTION_ALL					0xFFFFFFFF
#define CONFIG_SECTION_SKIN					0x00000001
#define CONFIG_SECTION_SYSTEM				0x00000002
#define CONFIG_SECTION_LAYOUT				0x00000004
#define CONFIG_SECTION_CUSTOMINSTRUMENT		0x00000008
#define CONFIG_SECTION_INVESTOR				0x00000010
#define CONFIG_SECTION_OTHER				0x00000020
#define CONFIG_SECTION_BROKERS				0x00000040
#define CONFIG_SECTION_LICENSE				0x00000080
#define CONFIG_SECTION_CONSTANT				0x00000100
#define CONFIG_SECTION_USER					0x00000200

