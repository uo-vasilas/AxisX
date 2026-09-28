/*

 **********************************************************************
 *
 * Original Axis by:
 * Copyright (C) Philip A. Esterle 1998-2002 + (C) parts Adron 2002
 *
 * 55r,56(x) Mods, and Axis2 re-build by:
 * Copyright (C) Benoit Croussette 2004-2006
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 **********************************************************************

*/

// ScriptObjects.cpp: implementation of the CScriptObjects class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AxisX.h"
#include "ProgressBar.h"
#include "ScriptObjects.h"
#include "RemoteProfileLoginDlg.h"
#include "Common.h"
#include <direct.h>
#include <vector>
#include <algorithm>
#include <shlobj.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScriptObjects::CScriptObjects()
{
	m_pDlg = NULL;
	m_iICatSeq = 0;
	m_iNCatSeq = 0;
	m_iACatSeq = 0;
}

CScriptObjects::~CScriptObjects()
{
	Unload(&m_aItems);
	Unload(&m_aNPCs);
	Unload(&m_aAreas);
	Unload(&m_aDefList);

	RemoveObjectsCategories(&m_olItems, "Items");
	RemoveObjectsCategories(&m_olNPCs, "NPCs");
	RemoveObjectsCategories(&m_olAreas, "Locations");

	m_asaNPCBrains.RemoveAll();
	m_asaNPCSkills.RemoveAll();
	m_asaNPCStats.RemoveAll();
	m_asaNPCResistences.RemoveAll();
	m_asaNPCMisc.RemoveAll();
	m_asaNPCTags.RemoveAll();

	m_asaITEMTypes.RemoveAll();
	m_asaITEMProps.RemoveAll();
	m_asaITEMTags.RemoveAll();

	m_asaFunctions.RemoveAll();
	m_asaEvents.RemoveAll();

	m_asaSPAWNitem.RemoveAll();
	m_asaSPAWNchar.RemoveAll();
}



enum RES_TYPE	// all the script resource blocks we want to deal with !
{
	// NOTE: SPHERE.INI, SPHERETABLE.SCP are read at start.
	// All other files are indexed from the SPEECHFILES and SCPFILES directories.
	// (SI) = Single instance types.
	// (SL) = single line multiple definitions.
	// Alphabetical order.
	RES_UNKNOWN = 0,	// Not to be used.
	RES_ADVANCE,		// Define the advance rates for stats.
	RES_AREA,			// Complex region. (w/extra tags)
	RES_AREADEF,		// Complex region. (w/ extra tags)
	RES_BLOCKIP,		// (SL) A list of IP's to block.
	RES_BOOK,			// A book or a page from a book.
	RES_CHARDEF,		// Define a char type. (overlap with RES_SPAWN)
	RES_COMMENT,		// A commented out block type.
	RES_DEFNAME,		// (SL) Just add a bunch of new defs and equivs str/values.
	RES_DEFNAMES,		// (SL) Just add a bunch of new defs and equivs str/values.  -- Same as DEFNAME, but for 99+
	RES_DEFMESSAGE,		// New 56a message block
	RES_DIALOG,			// A scriptable gump dialog, text or handler block.
	RES_EVENTS,			// An Event handler block with the trigger type in it. ON=@Death etc.
	RES_FAME,
	RES_FUNCTION,		// Define a new command verb script that applies to a char.
	RES_GLOBALS,
	RES_GMPAGE,			// A GM page. (SAVED in World)
	RES_ITEMDEF,		// Define an item type. (overlap with RES_TEMPLATE)
	RES_KARMA,
	RES_LOCATION,		// Hogloc.scp location format.
	RES_MENU,			// General scriptable menus.
	RES_MOONGATES,		// (SL) Define where the moongates are.
	RES_MULTIDEF,
	RES_NAMES,			// A block of possible names for a NPC type. (read as needed)
	RES_NEWBIE,			// Triggers to execute on Player creation (based on skills selected)
	RES_NOTOTITLES,		// (SI) Define the noto titles used.
	RES_OBSCENE,		// (SL) A list of obscene words.
	RES_PLEVEL,			// Define the list of commands that a PLEVEL can access. (or not access)
	RES_REGIONRESOURCE,	// Define an Ore type.
	RES_REGIONTYPE,		// Triggers etc. that can be assinged to a RES_AREA
	RES_RESOURCES,		// (SL) list of all the resource files we should index !
	RES_ROOM,			// Non-complex region. (no extra tags)
	RES_ROOMDEF,		// Non-complex region. (no extra tags)
	RES_RUNES,			// (SI) Define list of the magic runes.
	RES_SECTOR,			// Make changes to a sector. (SAVED in World)
	RES_SERVERS,		// List a number of servers in 3 line format. (Phase this out)
	RES_SKILL,			// Define attributes for a skill (how fast it raises etc)
	RES_SKILLCLASS,		// Define specifics for a char with this skill class. (ex. skill caps)
	RES_SKILLMENU,		// A menu that is attached to a skill. special arguments over other menus.
	RES_SPAWN,			// Define a list of NPC's and how often they may spawn.
	RES_SPEECH,			// A speech block with ON=*blah* in it.
	RES_SPELL,			// Define a magic spell. (0-64 are reserved)
	RES_SPHERE,			// Main Server INI block
	RES_STARTS,			// (SI) List of starting locations for newbies.
	RES_TELEPORTERS,	// (SL) Where are the teleporters in the world ? dungeon transports etc.
	RES_TEMPLATE,		// Define lists of items. (for filling loot etc)
	RES_TIMERF,
	RES_TYPEDEF,			// Define a trigger block for a RES_WORLDITEM m_type.
	RES_TYPEDEFS,
	RES_WEBPAGE,		// Define a web page template.
	RES_WORLDCHAR,		// Define instance of char in the world. (SAVED in World)
	RES_WORLDITEM,		// Define instance of item in the world. (SAVED in World)
	RES_WI,
	RES_WC,
	RES_WS,
	RES_EOF,
	RES_QTY,			// Don't care
};

const char * pResourceBlocks[RES_QTY] =	// static
{
	"AAAUNUSED",		// unused / unknown.
	"ADVANCE",			// Define the advance rates for stats.
	"AREA",				//Same as areadef
	"AREADEF",			// Complex region. (w/ extra tags)
	"BLOCKIP",			// (SL) A list of IP's to block.
	"BOOK",				// A book or a page from a book.
	"CHARDEF",			// Define a char type.
	"COMMENT",			// A commented out block type.
	"DEFNAME",			// (SL) Just add a bunch of new defs and equivs str/values.
	"DEFNAMES",			// (SL) Just add a bunch of new defs and equivs str/values. Same as DEFNAME, but for 99+
	"DEFMESSAGE",		// New 56a message block
	"DIALOG",			// A scriptable gump dialog", text or handler block.
	"EVENTS",			// (SL) Preload these Event files.
	"FAME",
	"FUNCTION",			// Define a new command verb script that applies to a char.
	"GLOBALS",
	"GMPAGE",			// A GM page. (SAVED in World)
	"ITEMDEF",			// Define an item type
	"KARMA",
	"LOCATION",			// Hogloc.scp location format.
	"MENU",				// General scriptable menus.
	"MOONGATES",		// (SL) Define where the moongates are.
	"MULTIDEF",
	"NAMES",			// A block of possible names for a NPC type. (read as needed)
	"NEWBIE",			// Triggers to execute on Player creation (based on skills selected)
	"NOTOTITLES",		// (SI) Define the noto titles used.
	"OBSCENE",			// (SL) A list of obscene words.
	"PLEVEL",			// Define the list of commands that a PLEVEL can access. (or not access)
	"REGIONRESOURCE",	// Define Ore types.
	"REGIONTYPE",		// Triggers etc. that can be assinged to a "AREA
	"RESOURCES",		// (SL) list of all the resource files we should index !
	"ROOM",				// Same as Room
	"ROOMDEF",			// Non-complex region. (no extra tags)
	"RUNES",			// (SI) Define list of the magic runes.
	"SECTOR",			// Make changes to a sector. (SAVED in World)
	"SERVERS",			// List a number of servers in 3 line format.
	"SKILL",			// Define attributes for a skill (how fast it raises etc)
	"SKILLCLASS",		// Define class specifics for a char with this skill class.
	"SKILLMENU",		// A menu that is attached to a skill. special arguments over other menus.
	"SPAWN",			// Define a list of NPC's and how often they may spawn.
	"SPEECH",			// (SL) Preload these speech files.
	"SPELL",			// Define a magic spell. (0-64 are reserved)
	"SPHERE",			// Main Server INI block
	"STARTS",			// (SI) List of starting locations for newbies.
	"TELEPORTERS",		// (SL) Where are the teleporteres in the world ?
	"TEMPLATE",			// Define a list of items. (for filling loot etc)
	"TIMERF",
	"TYPEDEF",			// Define a trigger block for a "WORLDITEM m_type.
	"TYPEDEFS",
	"WEBPAGE",			// Define a web page template.
	"WORLDCHAR",		// Define instance of char in the world. (SAVED in World)
	"WORLDITEM",		// Define instance of item in the world. (SAVED in World)
	"WI",
	"WC",
	"WS",
	"EOF",
};

const char * pRestricted [4] =
{
	"MySqlDatabase",		//0
	"MySqlHost",			//1
	"MySqlPassword",		//2
	"MySqlUser",			//3
};


bool CScriptObjects::LoadFile(CStdioFile * pFile, bool bResource, bool bProgress)
{
	if(!pFile)
		return false;

	CString csFile = pFile->GetFilePath();
	if(m_asaLoadedScripts.Find(csFile) != -1)
		return false;

	m_asaLoadedScripts.Add(csFile);

	// Use a 64KB read buffer for large script files.
	if ( pFile->m_pStream != NULL )
		setvbuf(pFile->m_pStream, NULL, _IOFBF, 64 * 1024);

	try
	{
		Main->m_log.Add(2, AXT("Loading file %s"), csFile);

		CWaitCursor hourglass;
		CString csMessage;
		csMessage.Format(AXT("Lese %s"), (LPCTSTR) csFile.Mid(csFile.ReverseFind('\\') + 1));	// File name only

		if ( m_pDlg )
		{
			m_pDlg->m_csMessage.SetWindowText(csMessage);
			m_pDlg->SetPos(0);
			m_pDlg->SetRange32(0, (DWORD) pFile->GetLength());
		}

		int j = 0;
		BOOL bStatus = TRUE;
		while ( bStatus )
		{
			CString csLine;
			bStatus = pFile->ReadString(csLine);

			if ( !bStatus )
				break;

			if ( csLine.Find("[") == 0 )
			{
				while ( bStatus )
				{
					if ( !bStatus )
						break;
					if ( csLine.Find("[") == 0 )
					{
						CString csKey = csLine.Mid(1);
						csKey = csKey.SpanExcluding("]");
						CString csIndex;
						if ( csKey.Find(" ") != -1 )
							csIndex = csKey.Mid(csKey.Find(" ") + 1);
						CString csType = csKey.SpanExcluding(" ");
						int resource = FindTable(csType, pResourceBlocks, RES_QTY);	

						switch (resource)
						{
						//Ignore these blocks
						case RES_UNKNOWN:
						case RES_ADVANCE:
						case RES_BLOCKIP:
						case RES_BOOK:
						case RES_COMMENT:
						case RES_DIALOG:
						case RES_FAME:
						case RES_GLOBALS:
						case RES_GMPAGE:
						case RES_KARMA:
						case RES_MENU:
						case RES_MOONGATES:
						case RES_NAMES:
						case RES_NEWBIE:
						case RES_NOTOTITLES:
						case RES_OBSCENE:
						case RES_PLEVEL:
						case RES_REGIONRESOURCE:
						case RES_REGIONTYPE:
						case RES_RUNES:
						case RES_SECTOR:
						case RES_SERVERS:
						case RES_SKILLCLASS:
						case RES_SKILLMENU:
						case RES_SPEECH:
						case RES_STARTS:
						case RES_TELEPORTERS:
						case RES_TIMERF:
						case RES_WEBPAGE:
						case RES_WORLDCHAR:
						case RES_WC:
						case RES_WS:
						case RES_EOF:
						case RES_QTY:
							bStatus = pFile->ReadString(csLine);
						break;
						case RES_ITEMDEF:
							{
								CSObject * pItem = new CSObject;
								pItem->m_bType = TYPE_ITEM;
								pItem->m_csID = csIndex;
								pItem->m_csValue = csIndex;
								pItem->m_csDisplay = csIndex;
								pItem->m_csFilename = csFile;	

								csLine = pItem->ReadBlock(*pFile);

								int iOld = m_aItems.Find(pItem->m_csValue);
								if ( iOld != -1 )
								{
									CSObject * pOld = (CSObject *) m_aItems.GetAt(iOld);
									m_aItems.RemoveAt(iOld);
									delete pOld;
								}
								m_aItems.Insert(pItem);
							}
							break;
						case RES_MULTIDEF:
							{
								CSObject * pItem = new CSObject;
								pItem->m_bType = TYPE_MULTI;
								pItem->m_csID = csIndex;
								pItem->m_csValue = csIndex;
								pItem->m_csDisplay = csIndex;
								pItem->m_csFilename = csFile;	

								csLine = pItem->ReadBlock(*pFile);

								int iOld = m_aItems.Find(pItem->m_csValue);
								if ( iOld != -1 )
								{
									CSObject * pOld = (CSObject *) m_aItems.GetAt(iOld);
									m_aItems.RemoveAt(iOld);
									delete pOld;
								}
								m_aItems.Insert(pItem);
							}
							break;
						case RES_TEMPLATE:
							{
								CSObject * pTempl = new CSObject;
								pTempl->m_bType = TYPE_TEMPLATE;
								pTempl->m_csID = csIndex;
								pTempl->m_csValue = csIndex;
								pTempl->m_csDisplay = "01";
								pTempl->m_csFilename = csFile;	

								csLine = pTempl->ReadBlock(*pFile);

								int iOld = m_aItems.Find(pTempl->m_csValue);
								if ( iOld != -1 )
								{
									CSObject * pOld = (CSObject *) m_aItems.GetAt(iOld);
									m_aItems.RemoveAt(iOld);
									delete pOld;
								}
								m_aItems.Insert(pTempl);
							}
							break;
						case RES_CHARDEF:
							{
								CSObject * pNPC = new CSObject;
								pNPC->m_bType = TYPE_CHAR;
								pNPC->m_csID = csIndex;
								pNPC->m_csValue = csIndex;
								pNPC->m_csDisplay = csIndex;
								pNPC->m_csFilename = csFile;

								csLine = pNPC->ReadBlock(*pFile);

								int iOld = m_aNPCs.Find(pNPC->m_csValue);
								if ( iOld != -1 )
								{
									CSObject * pOld = (CSObject *) m_aNPCs.GetAt(iOld);
									m_aNPCs.RemoveAt(iOld);
									delete pOld;
								}
								m_aNPCs.Insert(pNPC);
							}
							break;
						case RES_SPAWN:
							{
								CSObject * pSpawn = new CSObject;
								pSpawn->m_bType = TYPE_SPAWN;
								pSpawn->m_csID = csIndex;
								pSpawn->m_csValue = csIndex;
								pSpawn->m_csDisplay = "01";
								pSpawn->m_csFilename = csFile;

								csLine = pSpawn->ReadBlock(*pFile);

								int iOld = m_aNPCs.Find(pSpawn->m_csValue);
								if ( iOld != -1 )
								{
									CSObject * pOld = (CSObject *) m_aNPCs.GetAt(iOld);
									m_aNPCs.RemoveAt(iOld);
									delete pOld;
								}
								m_aNPCs.Insert(pSpawn);
							}
							break;
						case RES_AREA:
						case RES_AREADEF:
						case RES_ROOM:
						case RES_ROOMDEF:
						case RES_LOCATION:
							{
								CSObject * pArea = new CSObject;
								pArea->m_bType = TYPE_AREA;
								pArea->m_csValue = csIndex;
								pArea->m_csFilename = csFile;

								csLine = pArea->ReadBlock(*pFile);

								int iOld = m_aAreas.Find(pArea->m_csValue);
								if ( iOld != -1 )
								{
									CSObject * pOld = (CSObject *) m_aAreas.GetAt(iOld);
									m_aAreas.RemoveAt(iOld);
									delete pOld;
								}
								m_aAreas.Insert(pArea);
							}
							break;
						case RES_DEFNAME:
						case RES_SPHERE:
						case RES_DEFNAMES:
						case RES_DEFMESSAGE:
							{
								while ( bStatus )
								{
									bStatus = pFile->ReadString(csLine);
									if ( !bStatus )
										break;
									if ( csLine.Find("[") == 0 )
									{
										break;
									}
									csLine = csLine.SpanExcluding("//");
									csLine.Trim(); 
									if ( csLine != "" )
									{
										//load brain section separately
										if (csIndex.CompareNoCase("brains") == 0)
										{
											CString csBrain, csValue, csTemp;
											csTemp = csLine.SpanExcluding(" \t=");
											csValue = csLine.Mid(csLine.FindOneOf(" \t="));
											csValue.Trim();
											if ((csTemp != "") && (csValue != ""))
											{
												csBrain.Format("%s (%s)",csTemp,csValue);
												m_asaNPCBrains.Insert(csBrain);
											}
											continue;
										}
										CSObject * pDef = new CSObject;
										pDef->m_bType = TYPE_DEF;
										CString csTemp;
										csTemp = csLine.SpanExcluding(" \t=");
										pDef->m_csValue = csTemp;

										if ( pDef->m_csValue.GetLength() == csLine.GetLength() )
										{
											delete pDef;
											continue;
										}

										int restricted = FindTable(pDef->m_csValue, pRestricted, 4);
										if (restricted != -1)
										{
											delete pDef;
											continue;
										}
								
										csTemp = csLine.Mid(pDef->m_csValue.GetLength() + 1);
										csTemp.Trim();
										if( csTemp.FindOneOf(" \t="))
											csTemp = csTemp.Mid(csTemp.FindOneOf(" \t=") + 1);
										csTemp.Trim();

										if(csTemp.Find('{') != -1)
										{
											csTemp = csTemp.Mid(csTemp.Find("{")+1);
											csTemp = csTemp.Left(csTemp.ReverseFind('}'));
											if(csTemp.Find('{') != -1)
											{
												csTemp = csTemp.Mid(csTemp.ReverseFind('{')+1);
												csTemp = csTemp.SpanExcluding("}");	
											}
										}

										pDef->m_csID = csTemp;

										if ( pDef->m_csValue == "" )
										{
											delete pDef;
											continue;
										}

										if ( pDef->m_csValue == pDef->m_csID )
										{
											delete pDef;
											continue;
										}

										int iOld = m_aDefList.Find(pDef->m_csValue);
										if ( iOld != -1 )
										{
											CSObject * pOld = (CSObject *) m_aDefList.GetAt(iOld);
											m_aDefList.RemoveAt(iOld);
											delete pOld;
										}
										m_aDefList.Insert(pDef);
									}
								}
							}
							break;
						case RES_TYPEDEFS:
							{
								while ( bStatus )
								{
									bStatus = pFile->ReadString(csLine);
									if ( !bStatus )
										break;
									if ( csLine.Find("[") == 0 )
									{
										break;
									}
									csLine = csLine.SpanExcluding("//");
									csLine.Trim(); 
									if ( csLine != "" )
									{
											CString csType, csValue;
											csType = csLine.SpanExcluding(" \t=");
											int iOld = m_asaITEMTypes.Find(csType);
											if ( iOld != -1 )
												m_asaITEMTypes.RemoveAt(iOld);
											m_asaITEMTypes.Insert(csType);
									}
								}
							}
							break;
						case RES_TYPEDEF:
							{
								int iOld = m_asaITEMTypes.Find(csIndex);
								if ( iOld != -1 )
									m_asaITEMTypes.RemoveAt(iOld);
								m_asaITEMTypes.Insert(csIndex);
								bStatus = pFile->ReadString(csLine);
							}
							break;
						case RES_EVENTS:
							{
								int iOld = m_asaEvents.Find(csIndex);
								if ( iOld != -1 )
									m_asaEvents.RemoveAt(iOld);
								m_asaEvents.Insert(csIndex);
								bStatus = pFile->ReadString(csLine);
							}
							break;
						case RES_FUNCTION:
							{
								int iOld = m_asaFunctions.Find(csIndex);
								if ( iOld != -1 )
									m_asaFunctions.RemoveAt(iOld);
								m_asaFunctions.Insert(csIndex);
								bStatus = pFile->ReadString(csLine);
							}
							break;
						case RES_SPELL:
							{
								CSObject * pSpell = new CSObject;
								pSpell->m_bType = TYPE_SPELL;
								pSpell->m_csValue = csIndex;
								pSpell->m_csFilename = csFile;

								csLine = pSpell->ReadBlock(*pFile);

								int iOld = m_aSpellList.Find(pSpell->m_csValue);
								if ( iOld != -1 )
								{
									CSObject * pOld = (CSObject *) m_aSpellList.GetAt(iOld);
									m_aSpellList.RemoveAt(iOld);
									delete pOld;
								}
								m_aSpellList.Insert(pSpell);
							}
							break;
						case RES_SKILL:
							{
								while ( bStatus )
								{
									bStatus = pFile->ReadString(csLine);
									if ( !bStatus )
										break;
									if ( csLine.Find("[") == 0 )
									{
										break;
									}
									csLine = csLine.SpanExcluding("//");
									csLine.Trim();
									if ( csLine != "" )
									{
										CString csKey, csTemp;
										csTemp = csLine.SpanExcluding(" \t=");

										if ( csTemp.CompareNoCase("Key") != 0 )
											continue;
								
										csTemp = csLine.Mid(csTemp.GetLength() + 1);
										csTemp.Trim();
										if( csTemp.FindOneOf(" \t="))
											csTemp = csTemp.Mid(csTemp.FindOneOf(" \t=") + 1);
										csTemp.Trim();
										csKey.Format("%s (%s)", csTemp, csIndex);
										if (csKey != "")
											m_asaNPCSkills.Insert(csKey);
									}
								}
							}
							break;
						case RES_WORLDITEM:
						case RES_WI:
							{
								if (csIndex.CompareNoCase("i_worldgem_bit") == 0)
								{
									int iType = ITEM_SPAWN_CHAR;
									CString csPos;
									while ( bStatus )
									{
										bStatus = pFile->ReadString(csLine);
										if ( !bStatus )
											break;
										if ( csLine.Find("[") == 0 )
										{
											break;
										}
										csLine = csLine.SpanExcluding("//");
										csLine.Trim();
										if ( csLine != "" )
										{
											CString csTemp;
											csTemp = csLine.SpanExcluding(" \t=");

											if ( csTemp.CompareNoCase("TYPE") == 0 )
											{
												csTemp = csLine.Mid(csTemp.GetLength() + 1);
												csTemp.Trim();
												if( csTemp.FindOneOf(" \t="))
													csTemp = csTemp.Mid(csTemp.FindOneOf(" \t=") + 1);
												csTemp.Trim();
												if ( csTemp.CompareNoCase("t_spawn_item") == 0 )
													iType = ITEM_SPAWN_ITEM;
												else
													iType = 0;
											}
									
											if ( csTemp.CompareNoCase("P") == 0 )
											{
												csPos = csLine.Mid(csTemp.GetLength() + 1);
												csPos.Trim();
												if( csPos.FindOneOf(" \t="))
												{
													csPos = csPos.Mid(csPos.FindOneOf(" \t=") + 1);
													csPos.Trim();
												}
											}
										}
									}
									if (iType == ITEM_SPAWN_ITEM)
										m_asaSPAWNitem.Insert(csPos);
									else if (iType == ITEM_SPAWN_CHAR)
										m_asaSPAWNchar.Insert(csPos);
								}
								else
									bStatus = pFile->ReadString(csLine);
							}
							break;
						case RES_RESOURCES:
							{
								if(bResource)
								{
									while ( bStatus )
									{
										bStatus = pFile->ReadString(csLine);
										if ( !bStatus )
											break;
										if ( csLine.Find("[") == 0 )
										{
											break;
										}
										csLine = csLine.SpanExcluding("//");
										csLine.Trim(); 
										if ( csLine != "" )
										{
											CString csLoadFile;
											csLoadFile.Format("%s\\%s",csFile.Left(csFile.ReverseFind('\\')),csLine.Mid(csLine.FindOneOf("/\\")+1));
											csLoadFile.Replace('/', '\\');
											if( csLine.Right(4).CompareNoCase(".scp") == 0)
											{
												CStdioFile cfLoadFile;
												if ( !cfLoadFile.Open(csLoadFile, CFile::modeRead | CFile::shareDenyNone) )
												{
													Main->m_log.Add(1, AXT("ERROR: Unable to open file %s"), csLoadFile);
													continue;
												}
												LoadFile(&cfLoadFile);
												cfLoadFile.Close();
											}
											else
											{
												CString csLoadDir = csLoadFile.Left(csLoadFile.ReverseFind('\\'));
												LoadSingleDirectory(csLoadDir);
											}
										}
									}
								}
								bStatus = pFile->ReadString(csLine);
							}
							break;
						default:
							Main->m_log.Add(1, AXT("Unknown section %s in file %s"), csLine, csFile);
							bStatus = pFile->ReadString(csLine);
							break;
						}
					}
					else
					{
						bStatus = pFile->ReadString(csLine);
					}

					if ((bProgress)&&( m_pDlg ))
					{
						j++;
						if ( j % 10 == 0 && m_pDlg)
						{
							m_pDlg->SetPos((DWORD) pFile->GetPosition());
						}
					}
				}
			}
		}
	}
	catch (CFileException *e)
	{
		Main->m_log.Add(1, AXT("ERROR: Caught an exception while reading the file %s.  Cause code = %ld"), e->m_strFileName, e->m_cause);
		e->Delete();
	}
	if (m_pDlg)
		m_pDlg->SetPos((DWORD) pFile->GetLength());
	return true;
}

void CScriptObjects::LoadQuicklist(CString csList, CScriptArray * pObjList, CScriptArray * pDestList)
{
	CString csKey;
	csKey.Format("%s\\%s\\Quicklist",REGKEY_PROFILE, Main->m_csCurentProfile);
	CStringArray csaList;
	Main->GetRegistryMultiSz(csList, &csaList, hRegLocation, csKey);

	CreateProgressDialog();

	Main->m_log.Add(0, AXT("Loading %s Quicklist"), csList);
	CWaitCursor hourglass;
	m_pDlg->SetRange(0,(unsigned short) csaList.GetSize());
	m_pDlg->SetPos(0);
	m_pDlg->SetWindowText(AXT("Axis X l\xE4" "dt ..."));
	CString csMessage;
	csMessage.Format(AXT("Lade Schnellliste %s"), csList);
	m_pDlg->m_csMessage.SetWindowText(csMessage);

	if ( csaList.GetSize() > 0 )
	{
		for ( int i = 0; i <= csaList.GetUpperBound(); i++ )
		{
			if ( ( i % 0x100 ) == 0 ) // update the progress bar every 256 objects
				m_pDlg->SetPos(i);
			CString csObj = csaList.GetAt(i);
			int iLocated = pObjList->Find(csObj);
			if (iLocated != -1)
			{
				CSObject * pObject = (CSObject *) pObjList->GetAt(iLocated);
				pDestList->Insert(pObject);
			}
		}
	}
	DestroyProgressDialog();
}

void CScriptObjects::LoadProfile(CString csProfile)
{
		Main->m_csCurentProfile = csProfile;

		//No Profile
		if (csProfile == "<None>")
			return;

		Main->m_log.Add(0, AXT("Loading %s Profile"), csProfile);
		CreateProgressDialog();

		LoadCustomLocations();

		//Default Profile
		if (csProfile == "<Axis Profile>")
		{
			LoadProfileDirectory(csProfilePath);
			CategorizeObjects(&m_aItems, &m_olItems, "Items", &m_iICatSeq);
			CategorizeObjects(&m_aNPCs, &m_olNPCs, "NPCs", &m_iNCatSeq);
			CategorizeObjects(&m_aAreas, &m_olAreas, "Locations", &m_iACatSeq);
			LoadQuicklist("Items", &m_aItems, &m_ItemQuickList);
			LoadQuicklist("Spawns", &m_aNPCs, &m_SpawnQuickList);
			LoadQuicklist("Area", &m_aAreas, &m_AreaQuickList);
			DestroyProgressDialog();
			return;
		}

		//Local Profile
		CString csKey;
		csKey.Format("%s\\%s",REGKEY_PROFILE, csProfile);
		if(IsLocalProfile(csProfile))
		{
			CStringArray csaScripts;
			Main->GetRegistryMultiSz("Selected Scripts", &csaScripts, hRegLocation, csKey);
			DWORD dwLoadResource = Main->GetRegistryDword("Load Resource", 0, hRegLocation, csKey);
			if ( csaScripts.GetSize() > 0 )
			{
				for ( int i = 0; i <= csaScripts.GetUpperBound(); i++ )
				{
					CString csFile = csaScripts.GetAt(i);
					csFile.Replace('/', '\\');
					CStdioFile cfLocalFile;
					if ( !cfLocalFile.Open(csFile, CFile::modeRead | CFile::shareDenyNone) )
					{
						Main->m_log.Add(1, AXT("ERROR: Unable to open file %s"), csFile);
						continue;
					}
					if(dwLoadResource)
						LoadFile(&cfLocalFile, 1);
					else
						LoadFile(&cfLocalFile);
					cfLocalFile.Close();
				}
			}
			CategorizeObjects(&m_aItems, &m_olItems, "Items", &m_iICatSeq);
			CategorizeObjects(&m_aNPCs, &m_olNPCs, "NPCs", &m_iNCatSeq);
			CategorizeObjects(&m_aAreas, &m_olAreas, "Locations", &m_iACatSeq);
			LoadQuicklist("Items", &m_aItems, &m_ItemQuickList);
			LoadQuicklist("Spawns", &m_aNPCs, &m_SpawnQuickList);
			LoadQuicklist("Area", &m_aAreas, &m_AreaQuickList);
			DestroyProgressDialog();
			return;
		}

		//Web Profile
		else
		{
			CString csURL = Main->GetRegistryString("URL", "", hRegLocation, csKey);
			CInternetSession WebSession("Axis X - Web Profile");
			//CStdioFile * pFile = NULL;
				try
				{
					CString csInventory;
					CString strServerName;
					CString strObject;
					CString csUsername;
					CString csPassword;
					INTERNET_PORT nPort;
					DWORD dwServiceType;
					DWORD dwFlags = ICU_BROWSER_MODE;

					// check to see if this is a reasonable URL
					if (!AfxParseURLEx(csURL, dwServiceType, strServerName, strObject, nPort, csUsername, csPassword, dwFlags)
						|| (dwServiceType != INTERNET_SERVICE_HTTP && dwServiceType != INTERNET_SERVICE_FTP))
					{
						AfxMessageBox(AXT("Error: (Unknown service type) Only http:// and ftp:// allowed"));
						if(WebSession)
							WebSession.Close();
						DestroyProgressDialog();
						return;
					}
					//Http Connection
					if(dwServiceType == INTERNET_SERVICE_HTTP)
					{
						//Find Inventory File
						bool bDirect = false;
						csInventory = strObject.Mid(strObject.ReverseFind('/'));
						if(csInventory.Find('.') != -1)
						{
							csInventory = csURL;
							bDirect = true;
						}
						else
							csInventory.Format("%s/AxisSvr.ini",csURL);

						// All downloads go through the local web cache.
						CString csManifestName = bDirect ? csInventory.Mid(csInventory.ReverseFind('/') + 1) : CString(_T("AxisSvr.ini"));
						CString csManifestLocal = WebCacheLocalPath(csManifestName);

						int iRet = DownloadWebFile(WebSession, csInventory, csManifestLocal);
						while (iRet == 2) // HTTP 401 - ask for credentials and retry
						{
							CRemoteProfileLoginDlg dlg;
							if ( dlg.DoModal() == IDOK )
							{
								csUsername = dlg.m_csAccountName;
								csPassword = dlg.m_csPassword;
								CString scTemp;
								scTemp.Format("http://%s:%s@%s",csUsername,csPassword,csInventory.Mid(7));
								iRet = DownloadWebFile(WebSession, scTemp, csManifestLocal);
								if (iRet == 2)
									AfxMessageBox(AXT("Error: Login rejected by the server"));
							}
							else
							{
								if(WebSession)
									WebSession.Close();
								DestroyProgressDialog();
								return;
							}
						}

						if (iRet == 0)
						{
							AfxMessageBox(AXT("Error: Web profile is not reachable and no cached copy exists yet.\nConnect once successfully to fill the local cache."));
						}
						else if(bDirect)
						{
							LoadLocalCachedFile(csManifestLocal);
						}
						else
						{
							CStdioFile fManifest;
							if ( fManifest.Open(csManifestLocal, CFile::modeRead | CFile::shareDenyNone) )
							{
								CString csLine;
								while (fManifest.ReadString(csLine))
								{
									csLine.TrimLeft();
									csLine.TrimRight();
									if(csLine != "")
									{
										CString csSubFile;
										csSubFile.Format("%s/%s",csURL,csLine );
										CString csSubLocal = WebCacheLocalPath(csLine);
										int iSub = DownloadWebFile(WebSession, csSubFile, csSubLocal);
										if (iSub == 1)
											LoadLocalCachedFile(csSubLocal);
										else
											Main->m_log.Add(1, AXT("ERROR: Unable to load %s (download failed, no cached copy)"), csLine);
									}
								}
								fManifest.Close();
							}
							else
								Main->m_log.Add(1, AXT("ERROR: Unable to open cached manifest %s"), csManifestLocal);
						}
					}

					//FTP Connection
					else
					{
						if ((csUsername.GetLength() == 0)||(csPassword.GetLength() == 0))
						{
							CRemoteProfileLoginDlg dlg;
							if ( dlg.DoModal() == IDOK )
							{
								csUsername = dlg.m_csAccountName;
								csPassword = dlg.m_csPassword;
							}
							else
							{
								if(WebSession)
									WebSession.Close();
								DestroyProgressDialog();
								return;
							}
						}

						CFtpConnection* pConnect = NULL;
						pConnect = WebSession.GetFtpConnection(strServerName, csUsername, csPassword, nPort, 1);

						//Find Inventory File
						bool bDirect = false;
						csInventory = strObject.Mid(strObject.ReverseFind('/'));
						if(csInventory.Find('.') != -1)
						{
							csInventory = strObject;
							bDirect = true;
						}
						else
							csInventory.Format("%sAxisSvr.ini",strObject);

						// FTP files are always fetched, into the same local cache.
						CFtpFileFind bOpen(pConnect);
						if(bOpen.FindFile(csInventory))
						{
							if(bDirect)
							{
								CString csLocal = WebCacheLocalPath(csInventory.Mid(csInventory.ReverseFind('/') + 1));
								CFileStatus fsCached;
								if(pConnect->GetFile(csInventory, csLocal, FALSE, FILE_ATTRIBUTE_NORMAL, FTP_TRANSFER_TYPE_ASCII))
									LoadLocalCachedFile(csLocal);
								else if(CFile::GetStatus(csLocal, fsCached))
								{
									Main->m_log.Add(1, AXT("WARNING: %s not downloadable - using cached copy"), csInventory);
									LoadLocalCachedFile(csLocal);
								}
								else
									Main->m_log.Add(1, AXT("ERROR: Unable to open %s"), csInventory);
							}
							else
							{
								CString csManifestLocal = WebCacheLocalPath(_T("AxisSvr.ini"));
								if(!pConnect->GetFile(csInventory, csManifestLocal, FALSE, FILE_ATTRIBUTE_NORMAL, FTP_TRANSFER_TYPE_ASCII))
									Main->m_log.Add(1, AXT("WARNING: Unable to refresh %s - using cached manifest"), csInventory);
								CStdioFile pTemp;
								if ( pTemp.Open(csManifestLocal, CFile::modeRead | CFile::shareDenyNone) )
								{
									CString csLine;
									while (pTemp.ReadString(csLine))
									{
										csLine.TrimLeft();
										csLine.TrimRight();
										if(csLine != "")
										{
											CString csLocal = WebCacheLocalPath(csLine);
											CFileStatus fsCachedSub;
											if(pConnect->GetFile(csLine, csLocal, FALSE, FILE_ATTRIBUTE_NORMAL, FTP_TRANSFER_TYPE_ASCII))
												LoadLocalCachedFile(csLocal);
											else if(CFile::GetStatus(csLocal, fsCachedSub))
											{
												Main->m_log.Add(1, AXT("WARNING: %s not downloadable - using cached copy"), csLine);
												LoadLocalCachedFile(csLocal);
											}
											else
												Main->m_log.Add(1, AXT("ERROR: Unable to open %s"), csLine);
										}
									}
									pTemp.Close();
								}
								else
								{
									AfxMessageBox(AXT("Error: Unable to open inventory file AxisSvr.ini"));
								}
							}
						}
						else
						{
							CString csMessage;
							csMessage.Format(AXT("Error: Unable to find inventory file %s"), csInventory);
							AfxMessageBox(csMessage);
						}
						if(pConnect)
							pConnect->Close();
					}
				}
				catch (CInternetException* pEx)
				{
					TCHAR szErr[1024];
					pEx->GetErrorMessage(szErr, 1024);
					CString csMessage;
					csMessage.Format(AXT("Error: (%d) %s"),pEx->m_dwError, szErr);
					pEx->Delete();

					// Fall back to the local cache when the server is unreachable.
					CString csManifestLocal = WebCacheLocalPath(_T("AxisSvr.ini"));
					CStdioFile fManifest;
					if ( fManifest.Open(csManifestLocal, CFile::modeRead | CFile::shareDenyNone) )
					{
						Main->m_log.Add(1, AXT("WARNING: %s - loading web profile from local cache"), csMessage);
						CString csLine;
						while (fManifest.ReadString(csLine))
						{
							csLine.TrimLeft();
							csLine.TrimRight();
							if (csLine != "")
								LoadLocalCachedFile(WebCacheLocalPath(csLine));
						}
						fManifest.Close();
					}
					else
						AfxMessageBox(csMessage);
				}
			if(WebSession)
				WebSession.Close();
			CategorizeObjects(&m_aItems, &m_olItems, "Items", &m_iICatSeq);
			CategorizeObjects(&m_aNPCs, &m_olNPCs, "NPCs", &m_iNCatSeq);
			CategorizeObjects(&m_aAreas, &m_olAreas, "Locations", &m_iACatSeq);
			LoadQuicklist("Items", &m_aItems, &m_ItemQuickList);
			LoadQuicklist("Spawns", &m_aNPCs, &m_SpawnQuickList);
			LoadQuicklist("Area", &m_aAreas, &m_AreaQuickList);
			DestroyProgressDialog();
		}
}

//*************************************
// Web profile cache

// %LOCALAPPDATA%\AxisX\WebCache\<profile> - created on demand.
CString CScriptObjects::GetWebCacheDir()
{
	TCHAR szPath[MAX_PATH];
	if ( FAILED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, szPath)) )
		return _T("");
	CString csProfile = Main->m_csCurentProfile;
	if ( csProfile.IsEmpty() )
		csProfile = _T("default");
	// keep the folder name filesystem-safe
	for ( int i = 0; i < csProfile.GetLength(); i++ )
	{
		TCHAR c = csProfile[i];
		if ( c == '\\' || c == '/' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|' )
			csProfile.SetAt(i, '_');
	}
	CString csDir;
	csDir.Format(_T("%s\\AxisX\\WebCache\\%s"), szPath, csProfile);
	SHCreateDirectoryEx(NULL, csDir, NULL);
	return csDir;
}

// Maps a (relative) file name from the web manifest to its cache location and
// makes sure the sub folder exists.
CString CScriptObjects::WebCacheLocalPath(CString csRelativeName)
{
	csRelativeName.Replace('/', '\\');
	// strip anything that could escape the cache folder
	while ( csRelativeName.Find(_T("..")) != -1 )
		csRelativeName.Replace(_T(".."), _T(""));
	while ( csRelativeName.GetLength() > 0 && csRelativeName[0] == '\\' )
		csRelativeName = csRelativeName.Mid(1);
	if ( csRelativeName.IsEmpty() )
		return _T("");
	CString csDir = GetWebCacheDir();
	if ( csDir.IsEmpty() )
		return _T("");
	CString csFull;
	csFull.Format(_T("%s\\%s"), csDir, csRelativeName);
	int iSlash = csFull.ReverseFind('\\');
	if ( iSlash > 0 )
		SHCreateDirectoryEx(NULL, csFull.Left(iSlash), NULL);
	return csFull;
}

// Downloads csRemoteURL into csLocalPath unless the cached copy is current
// (If-Modified-Since). Returns 0 = failure, 1 = usable copy, 2 = HTTP 401.
int CScriptObjects::DownloadWebFile(CInternetSession & session, CString csRemoteURL, CString csLocalPath)
{
	if ( csLocalPath.IsEmpty() )
		return 0;
	CFileStatus fsCache;
	bool bHaveCache = CFile::GetStatus(csLocalPath, fsCache) ? true : false;

	CString csHeaders;
	if ( bHaveCache )
		csHeaders = _T("If-Modified-Since: ") + fsCache.m_mtime.FormatGmt(_T("%a, %d %b %Y %H:%M:%S GMT")) + _T("\r\n");

	CHttpFile * pHttpFile = NULL;
	try
	{
		pHttpFile = (CHttpFile *) session.OpenURL(csRemoteURL, 1,
			INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_RELOAD,
			csHeaders.IsEmpty() ? NULL : (LPCTSTR) csHeaders,
			csHeaders.IsEmpty() ? 0 : (DWORD) csHeaders.GetLength());
	}
	catch (CInternetException * pEx)
	{
		pEx->Delete();
		pHttpFile = NULL;
	}
	if ( pHttpFile == NULL )
	{
		// offline / server down - fall back to whatever is cached
		if ( bHaveCache )
			Main->m_log.Add(1, AXT("WARNING: %s not reachable - using cached copy"), (LPCTSTR) csRemoteURL);
		return bHaveCache ? 1 : 0;
	}

	DWORD dwStatus = 0;
	pHttpFile->QueryInfoStatusCode(dwStatus);

	if ( dwStatus == HTTP_STATUS_DENIED )
	{
		pHttpFile->Close();
		delete pHttpFile;
		return 2;
	}
	if ( dwStatus == HTTP_STATUS_NOT_MODIFIED && bHaveCache )
	{
		pHttpFile->Close();
		delete pHttpFile;
		return 1;
	}
	if ( dwStatus != HTTP_STATUS_OK )
	{
		pHttpFile->Close();
		delete pHttpFile;
		if ( bHaveCache )
		{
			Main->m_log.Add(1, AXT("WARNING: HTTP %d for %s - using cached copy"), dwStatus, (LPCTSTR) csRemoteURL);
			return 1;
		}
		Main->m_log.Add(1, AXT("ERROR: HTTP %d for %s"), dwStatus, (LPCTSTR) csRemoteURL);
		return 0;
	}

	// 200 OK - stream the file to disk
	CFile fOut;
	if ( !fOut.Open(csLocalPath, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareDenyWrite) )
	{
		pHttpFile->Close();
		delete pHttpFile;
		Main->m_log.Add(1, AXT("ERROR: Unable to write cache file %s"), (LPCTSTR) csLocalPath);
		return 0;
	}
	BYTE bBuffer[16384];
	UINT uRead;
	while ( (uRead = pHttpFile->Read(bBuffer, sizeof(bBuffer))) > 0 )
		fOut.Write(bBuffer, uRead);
	fOut.Close();

	// Stamp the copy with the server's Last-Modified time.
	SYSTEMTIME stMod;
	DWORD dwLen = sizeof(stMod);
	if ( pHttpFile->QueryInfo(HTTP_QUERY_LAST_MODIFIED | HTTP_QUERY_FLAG_SYSTEMTIME, &stMod, &dwLen, NULL) )
	{
		FILETIME ftMod;
		if ( SystemTimeToFileTime(&stMod, &ftMod) )
		{
			HANDLE hFile = CreateFile(csLocalPath, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
			if ( hFile != INVALID_HANDLE_VALUE )
			{
				SetFileTime(hFile, NULL, NULL, &ftMod);
				CloseHandle(hFile);
			}
		}
	}
	pHttpFile->Close();
	delete pHttpFile;
	Main->m_log.Add(2, AXT("Downloaded %s"), (LPCTSTR) csRemoteURL);
	return 1;
}

bool CScriptObjects::LoadLocalCachedFile(const CString & csLocalPath)
{
	CStdioFile fCached;
	if ( !fCached.Open(csLocalPath, CFile::modeRead | CFile::shareDenyNone) )
	{
		Main->m_log.Add(1, AXT("ERROR: Unable to open cached file %s"), (LPCTSTR) csLocalPath);
		return false;
	}
	LoadFile(&fCached, 0, 0);
	fCached.Close();
	return true;
}

void CScriptObjects::LoadProfileDirectory(CString csPath)
{
	WIN32_FIND_DATA findData;
	memset(&findData, 0x00, sizeof(findData));
	CString csTestFile;
	csTestFile.Format("%s\\*", csPath);
	HANDLE hSearch = FindFirstFile(csTestFile, &findData);
	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		BOOL bStatus = TRUE;
		while (bStatus)
		{
			if ( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				if ( strcmp(findData.cFileName, "..") != 0 && strcmp(findData.cFileName, ".") != 0 )
				{
					CString csNewPath;
					csNewPath.Format("%s\\%s", csPath, findData.cFileName);
					LoadProfileDirectory(csNewPath);
				}
			}
			else
			{
				CString csFileName = _T(findData.cFileName);
				if (( csFileName.Right(4).CompareNoCase(".scp") == 0)||(csFileName.CompareNoCase("sphere.ini")==0))
				{
					if (( csFileName.Mid(0,7).CompareNoCase("sphereb") == 0) && (IsNumber(csFileName.Mid(7,2))))
					{
						bStatus = FindNextFile(hSearch, &findData);
						continue;
					}

					CString csFullPath;
					csFullPath.Format("%s\\%s", csPath, findData.cFileName);
					CStdioFile cfDefaultFile;
					if ( cfDefaultFile.Open(csFullPath, CFile::modeRead | CFile::shareDenyNone) )
					{
						LoadFile(&cfDefaultFile, 0);
						cfDefaultFile.Close();
					}
					else
					{
						Main->m_log.Add(1, AXT("ERROR: Unable to open file %s"), csFullPath);
					}
				}
			}
			bStatus = FindNextFile(hSearch, &findData);
		}
		FindClose(hSearch);
	}
}

void CScriptObjects::LoadSingleDirectory(CString csPath)
{
	WIN32_FIND_DATA findData;
	memset(&findData, 0x00, sizeof(findData));
	CString csTestFile;
	csTestFile.Format("%s\\*", csPath);
	HANDLE hSearch = FindFirstFile(csTestFile, &findData);
	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		BOOL bStatus = TRUE;
		while (bStatus)
		{
			if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
			{
				CString csFileName = _T(findData.cFileName);
				if ( csFileName.Right(4).CompareNoCase(".scp") == 0)
				{
					CString csFullPath;
					csFullPath.Format("%s\\%s", csPath, findData.cFileName);
					CStdioFile cfFile;
					if ( cfFile.Open(csFullPath, CFile::modeRead | CFile::shareDenyNone) )
					{
						LoadFile(&cfFile, 0);
						cfFile.Close();
					}
					else
					{
						Main->m_log.Add(1, AXT("ERROR: Unable to open file %s"), csFullPath);
					}
				}
			}
			bStatus = FindNextFile(hSearch, &findData);
		}
		FindClose(hSearch);
	}
}

void CScriptObjects::UnloadProfile()
{
	delete Main->m_pScripts;
	Main->m_pScripts = new CScriptObjects;
}

void CScriptObjects::Unload(CScriptArray * pObjList)
{
	for ( int i = 0; i < pObjList->GetSize(); i++ )
	{
		CSObject * pDef = (CSObject *) pObjList->GetAt(i);
		if ( pDef )
			delete pDef;
	}
	pObjList->RemoveAll();
}


void CScriptObjects::CreateProgressDialog()
{
	if ( m_pDlg )
		return;
	m_pDlg = new CProgressBar;
	m_pDlg->Create(IDD_PROGRESS);
	m_bDeleteDialog = true;
}


void CScriptObjects::DestroyProgressDialog()
{
	if ( !m_bDeleteDialog )
		return;
	if ( !m_pDlg )
		return;
	m_pDlg->DestroyWindow();
	delete m_pDlg;
	m_pDlg = NULL;
}

void CScriptObjects::RemoveObjectsCategories(CPtrList * pCatList, CString csName)
{
	Main->m_log.Add(0, AXT("Clearing %s categories"), csName);
	if (!pCatList->IsEmpty())
	{
		POSITION catPos = pCatList->GetHeadPosition();
		while (catPos != NULL)
		{
			CCategory * pCat = (CCategory *) pCatList->GetNext(catPos);
			if (! pCat->m_SubsectionList.IsEmpty())
			{
				POSITION subPos = pCat->m_SubsectionList.GetHeadPosition();
				while (subPos != NULL)
				{
					CSubsection *pSub = (CSubsection *) pCat->m_SubsectionList.GetNext(subPos);
					delete (pSub);
				}
			}
			delete (pCat);
		}
	}
	pCatList->RemoveAll();
}

void CScriptObjects::CategorizeObjects(CScriptArray * pObjList, CPtrList * pCatList, CString csName, int * m_iCatSeq)
{
	CreateProgressDialog();

	Main->m_log.Add(0, AXT("Categorizing %s"), csName);
	CWaitCursor hourglass;
	m_pDlg->SetRange(0,(unsigned short) pObjList->GetSize());
	m_pDlg->SetPos(0);
	m_pDlg->SetWindowText(AXT("Axis X l\xE4" "dt ..."));
	CString csMessage;
	csMessage.Format(AXT("Sortiere %s"), csName);
	m_pDlg->m_csMessage.SetWindowText(csMessage);

	// Hash lookup for categories/subsections instead of linear searches.
	CMapStringToPtr mapCats;   // category name -> CCategory*
	CMapStringToPtr mapSubs;   // "category\x01subsection" -> CSubsection*
	for ( POSITION posPre = pCatList->GetHeadPosition(); posPre != NULL; )
	{
		CCategory * pCat = (CCategory *) pCatList->GetNext(posPre);
		mapCats.SetAt(pCat->m_csName, pCat);
		for ( POSITION posSub = pCat->m_SubsectionList.GetHeadPosition(); posSub != NULL; )
		{
			CSubsection * pSub = (CSubsection *) pCat->m_SubsectionList.GetNext(posSub);
			mapSubs.SetAt(pCat->m_csName + _T("\x01") + pSub->m_csName, pSub);
		}
	}

	// Go through the item array and organize it into categories.
	for ( int i = 0; i < pObjList->GetSize(); i++ )
	{
		if ( ( i % 0x100 ) == 0 ) // update the progress bar every 256 objects
			m_pDlg->SetPos(i);
		CSObject * pItem = (CSObject *) pObjList->GetAt(i);
		if ( pItem )
		{
			if ( pItem->m_csDupeItem != "" )
			{
				int iIndex = pObjList->Find(pItem->m_csDupeItem);
				if ( iIndex == -1 )
					iIndex = pObjList->Find(pObjList->GetDef(pItem->m_csDupeItem));
				if ( iIndex != -1 )
				{
					CSObject * pDupe = (CSObject *) pObjList->GetAt(iIndex);
					if ( pItem->m_csCategory == "<none>" )
						pItem->m_csCategory = pDupe->m_csCategory;
					if ( pItem->m_csSubsection == "<none>" )
						pItem->m_csSubsection = pDupe->m_csSubsection;
					if ( pItem->m_csDescription.Find("<unnamed>") != -1 )
					{
						CString csDesc;
						csDesc.Format("%s - (Dupe)", pDupe->m_csDescription);
						pItem->m_csDescription = csDesc;
					}
					if ( ahextoi(pItem->m_csColor) == 0 )
						pItem->m_csColor = pDupe->m_csColor;
				}
			}
			CString cs_TempCat, cs_Cat;
			if ( pItem->m_csDescription.Find("@") != -1 )
				pItem->m_csDescription.Replace("@", pItem->m_csSubsection);
			if ( pItem->m_csCategory == "<none>" )
			{
				cs_TempCat.Format("<uncategorized %s>",csName);
				cs_TempCat.MakeLower();
				pItem->m_csCategory = cs_TempCat;
			}
			else
			{
				cs_TempCat = pItem->m_csCategory;
				cs_TempCat.MakeLower();
				cs_Cat = cs_TempCat.Left(1);
				cs_Cat.MakeUpper();
				cs_Cat = cs_Cat + cs_TempCat.Mid(1);
				pItem->m_csCategory = cs_Cat;
			}
			if ( pItem->m_csSubsection == "<none>" )
			{
				cs_TempCat = pItem->m_csFilename;
				cs_TempCat = cs_TempCat.Mid(cs_TempCat.ReverseFind('\\')+1);
				cs_TempCat.MakeLower();
				cs_Cat = cs_TempCat.Left(1);
				cs_Cat.MakeUpper();
				cs_Cat = cs_Cat + cs_TempCat.Mid(1);
				pItem->m_csSubsection = cs_Cat;
			}
			else
			{
				cs_TempCat = pItem->m_csSubsection;
				cs_TempCat.MakeLower();
				cs_Cat = cs_TempCat.Left(1);
				cs_Cat.MakeUpper();
				cs_Cat = cs_Cat + cs_TempCat.Mid(1);
				pItem->m_csSubsection = cs_Cat;
			}
			if ( pItem->m_csDescription == "<unnamed>" )
				pItem->m_csDescription = pItem->m_csID;
			else
			{
				cs_TempCat = pItem->m_csDescription;
				cs_TempCat.MakeLower();
				cs_Cat = cs_TempCat.Left(1);
				cs_Cat.MakeUpper();
				cs_Cat = cs_Cat + cs_TempCat.Mid(1);
				pItem->m_csDescription = cs_Cat;
			}

			void * pvLookup = NULL;
			CCategory * pCategory;
			if ( mapCats.Lookup(pItem->m_csCategory, pvLookup) )
				pCategory = (CCategory *) pvLookup;
			else
			{
				pCategory = FindCategory(pCatList, pItem->m_csCategory);
				mapCats.SetAt(pItem->m_csCategory, pCategory);
			}
			CString csSubKey = pItem->m_csCategory + _T("\x01") + pItem->m_csSubsection;
			CSubsection * pSubsection;
			if ( mapSubs.Lookup(csSubKey, pvLookup) )
				pSubsection = (CSubsection *) pvLookup;
			else
			{
				pSubsection = FindSubsection(pCategory, pItem->m_csSubsection);
				mapSubs.SetAt(csSubKey, pSubsection);
			}

			// Append unsorted; each list is sorted once below.
			pSubsection->m_ItemList.AddTail(pItem);
		}
	}

	// Sort every subsection list once (by description, stable).
	for ( POSITION posCat = pCatList->GetHeadPosition(); posCat != NULL; )
	{
		CCategory * pCat = (CCategory *) pCatList->GetNext(posCat);
		for ( POSITION posSub = pCat->m_SubsectionList.GetHeadPosition(); posSub != NULL; )
		{
			CSubsection * pSub = (CSubsection *) pCat->m_SubsectionList.GetNext(posSub);
			if ( pSub->m_ItemList.GetCount() > 1 )
			{
				std::vector<CSObject *> vItems;
				vItems.reserve((size_t) pSub->m_ItemList.GetCount());
				for ( POSITION posItem = pSub->m_ItemList.GetHeadPosition(); posItem != NULL; )
					vItems.push_back((CSObject *) pSub->m_ItemList.GetNext(posItem));
				std::stable_sort(vItems.begin(), vItems.end(),
					[](const CSObject * a, const CSObject * b) { return a->m_csDescription < b->m_csDescription; });
				pSub->m_ItemList.RemoveAll();
				for ( size_t k = 0; k < vItems.size(); k++ )
					pSub->m_ItemList.AddTail(vItems[k]);
			}
		}
	}
	DestroyProgressDialog();
	srand( (unsigned)time( NULL ) );
	*m_iCatSeq = rand();
}

void CScriptObjects::LoadCustomLocations()
{
	HKEY hCatKey;
	int iCatIndex = 0;
	CString csCategory, csSubsection, csDescription;
	LONG lCatStatus = RegOpenKeyEx(hRegLocation, REGKEY_LOCATION, 0, KEY_ALL_ACCESS, &hCatKey);
	if ( lCatStatus == ERROR_SUCCESS )
	{
		while (lCatStatus == ERROR_SUCCESS)
		{
			char szBuffer[MAX_PATH];
			DWORD dwSize = sizeof(szBuffer);
			lCatStatus = RegEnumKeyEx(hCatKey, iCatIndex, &szBuffer[0], &dwSize, 0, NULL, NULL, NULL);
			if (lCatStatus == ERROR_SUCCESS)
			{
				csCategory = szBuffer;

				HKEY hSubKey;
				CString csSubKey;
				csSubKey.Format("%s\\%s", REGKEY_LOCATION, csCategory);
				LONG lSubStatus = RegOpenKeyEx(hRegLocation, csSubKey, 0, KEY_ALL_ACCESS, &hSubKey);
				if ( lSubStatus == ERROR_SUCCESS )
				{
					// The enum index restarts at 0 for every category.
					int iSubIndex = 0;
					while (lSubStatus == ERROR_SUCCESS)
					{
						char szSubBuffer[MAX_PATH];
						DWORD dwSize = sizeof(szSubBuffer);
						lSubStatus = RegEnumKeyEx(hSubKey, iSubIndex, &szSubBuffer[0], &dwSize, 0, NULL, NULL, NULL);
						if (lSubStatus == ERROR_SUCCESS)
						{
							csSubsection = szSubBuffer;

							HKEY hDescKey;
							CString csDescKey;
							csDescKey.Format("%s\\%s", csSubKey, csSubsection);
							LONG lDescStatus = RegOpenKeyEx(hRegLocation, csDescKey, 0, KEY_ALL_ACCESS, &hDescKey);
							if ( lDescStatus == ERROR_SUCCESS )
							{
								// The enum index restarts at 0 for every subsection.
								int iDescIndex = 0;
								while (lDescStatus == ERROR_SUCCESS)
								{
									char szDescBuffer[MAX_PATH];
									DWORD dwSize = sizeof(szDescBuffer);
									lDescStatus = RegEnumKeyEx(hDescKey, iDescIndex, &szDescBuffer[0], &dwSize, 0, NULL, NULL, NULL);
									if (lDescStatus == ERROR_SUCCESS)
									{
										csDescription = szDescBuffer;

										CSObject * pArea = new CSObject;
										pArea->m_bType = TYPE_AREA;
										pArea->m_bCustom = true;
										pArea->m_csCategory = csCategory;
										pArea->m_csSubsection = csSubsection;
										pArea->m_csDescription = csDescription;
										CString csValueKey;
										csValueKey.Format("%s\\%s", csDescKey, csDescription);
										pArea->m_csID = Main->GetRegistryString("Point", "", hRegLocation, csValueKey);
										pArea->m_csDisplay = Main->GetRegistryString("Map", "", hRegLocation, csValueKey);
										m_aAreas.Insert(pArea);
									}
									iDescIndex++;
								}
								RegCloseKey(hDescKey);
							}
						}
						iSubIndex++;
					}
					RegCloseKey(hSubKey);
				}
			}
			iCatIndex++;
		}
		RegCloseKey(hCatKey);
	}
}