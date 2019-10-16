#pragma once

#include <vector>

#include "DEST_Typer.h"

class CDestinationManager
{
public:
	CDestinationManager();
	~CDestinationManager();

	void Init();

	bool AddDestination(SDestinationV2 & _Dest);
	bool RemoveDestination(SDestinationV2 & _Dest);
	bool ReplaceDestination(SDestinationV2& _Old, SDestinationV2& _New);

	bool IsFileOpen() { return m_bFileLoaded; }
	bool OpenFile(const char* szFilename);
	bool SaveFile(const char* szFilename);
	void NewFile();
	void CloseFile();

	const std::vector<SDestinationV2>* getData() { return &m_Data; }
	bool searchByPinYin(const char* szKey, std::vector<SDestinationV2>& output);
	bool searchByUID(const DEST_UID & uid, SDestinationV2* output);

	static CDestinationManager* Instance();

private:
	std::vector<SDestinationV2> m_Data;
	bool m_bFileLoaded = false;
};