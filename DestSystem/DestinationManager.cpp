#include <StdAfx.h>

#include "DestinationManager.h"

#include <iostream>
#include <fstream>
#include <iterator>

constexpr char* DEST_FOLDER = "Dest system";

namespace STARTUP
{
	static CDestinationManager DestMGR;
}

CDestinationManager::CDestinationManager()
{
}

CDestinationManager::~CDestinationManager()
{
}

void CDestinationManager::Init()
{
#if ISYSTEM_PROJECT
	LogAlways(__FUNCTION__);

	time_t now = time(0);
	const char* dt = ctime(&now);
	CryLogAlways(dt);

	int filecount = 0;
	std::vector<SDestinationV2> container;
	ICryPak* pCryPak = gEnv->pCryPak;
	_finddata_t fd;
	char filePath[_MAX_PATH];

	string folder = string(iSystemConfigFolder.toStdString().c_str()) + DEST_FOLDER;
	string search = folder + "/*.DESTDAT";

	intptr_t handle = pCryPak->FindFirst(search.c_str(), &fd);
	if (handle != -1)
	{
		int res = 0;
		do
		{
			cry_strcpy(filePath, folder);
			cry_strcat(filePath, "/");
			cry_strcat(filePath, fd.name);

			// 把文件内容读取到m_Data
			if (OpenFile(filePath) == false)
			{
				res = pCryPak->FindNext(handle, &fd);
				LogError("File %s open failed. skip...", fd.name);
				continue;
			}

			filecount++;
			container.insert(container.end(), m_Data.begin(), m_Data.end());
			res = pCryPak->FindNext(handle, &fd);

			LogAlways("File: %s, dest number: %d", fd.name, m_Data.size());
		} while (res >= 0);
		pCryPak->FindClose(handle);
	}

	m_Data = container;
	LogAlways("DEST manager init done.		total file: %d, total dest: %d", m_Data.size());

#endif
}

bool CDestinationManager::AddDestination(SDestinationV2 & _Dest)
{
	if (!m_bFileLoaded)
		return false;

	for each (SDestinationV2 obj in m_Data)
	{
		if (obj == _Dest)
			return false;
	}

	m_Data.push_back(_Dest);

	return true;
}

bool CDestinationManager::RemoveDestination(SDestinationV2 & _Dest)
{
	if (!m_bFileLoaded)
		return false;

	for (auto iter = m_Data.begin(); iter!=m_Data.end(); ++iter)
	{
		if (*iter == _Dest)
		{
			m_Data.erase(iter);
			return true;
		}
	}

	return false;
}

bool CDestinationManager::ReplaceDestination(SDestinationV2 & _Old, SDestinationV2 & _New)
{
	if (!m_bFileLoaded)
		return false;

	for (auto iter = m_Data.begin(); iter != m_Data.end(); ++iter)
	{
		if (*iter == _Old)
		{
			(*iter) = _New;
			return true;
		}
	}

	return false;
}

bool CDestinationManager::OpenFile(const char * szFilename)
{
	using namespace std;
	int fileSize, size;
	DEST_DATA_HEADER header;
	memset(&header, 0, sizeof(DEST_DATA_HEADER));

	ifstream is(szFilename, ios::binary);

	if (!is)
		return false;

	is.seekg(0, ios::end);
	fileSize = is.tellg();
	is.seekg(0);

	is.read((char*)&header, sizeof(DEST_DATA_HEADER));
	
	if (header.Length != fileSize)
		return false;

	is.read((char *)&size, sizeof(int));

	if (size <= 0)
		return false;

	m_Data.clear();

	if (header.Version == 1.0)
	{
		for (int i = 0; i < size; ++i)
		{
			SDestinationV1 data;
			is.read((char*)&data, sizeof(SDestinationV1));

			SDestinationV2 data2;
			data2 = data;

			m_Data.push_back(data2);
		}
	}
	else if (header.Version == 2.0)
	{
		m_Data.resize(size);
		is.read((char*)&m_Data[0], size * sizeof(SDestinationV2));
	}
	else
		return false;

	m_bFileLoaded = true;
	return true;
}

bool CDestinationManager::SaveFile(const char * szFilename)
{
	using namespace std;

	ofstream os(szFilename, ios::binary);

	if (!os)
		return false;

	int size = (int)m_Data.size();
	int fileLength = sizeof(DEST_DATA_HEADER) + sizeof(int) + size * sizeof(SDestinationV2);

	DEST_DATA_HEADER header = { 2.0, fileLength };

	os.write((char*)&header, sizeof(DEST_DATA_HEADER));
	os.write((char*)&size, sizeof(int));

	if (size > 0)
		os.write((char*)&m_Data[0], size * sizeof(SDestinationV2));
	
	os.close();

	return true;
}

void CDestinationManager::NewFile()
{
	m_Data.clear();
	m_bFileLoaded = true;
}

void CDestinationManager::CloseFile()
{
	m_Data.clear();
	m_bFileLoaded = false;
}

bool CDestinationManager::searchByPinYin(const char* szKey, std::vector<SDestinationV2>& output)
{
	char keyUpper[100];
	memset(keyUpper, 0, sizeof(keyUpper));
	output.clear();

	if (szKey && *szKey)
	{
		size_t len = strlen(szKey);

		for (int i = 0; i < len; ++i)
		{
			if (szKey[i] > 'a' && szKey[i] < 'z')
				keyUpper[i] = (char)(szKey[i] - 32);
			else
				keyUpper[i] = (char)szKey[i];
		}

		std::string key = keyUpper;
		for (auto obj : m_Data)
		{
			std::string source = obj.initialsPy;

			if (source.find(key) != std::string::npos)
				output.push_back(obj);
		}
	}
	return output.size() > 0;
}

bool CDestinationManager::searchByUID(const DEST_UID & uid, SDestinationV2 * output)
{
	for (auto obj : m_Data)
	{
		if (obj.uid == uid)
		{
			memcpy(output, &obj, sizeof(SDestinationV2));
			return true;
		}
	}

	return false;
}

CDestinationManager * CDestinationManager::Instance()
{
	return &STARTUP::DestMGR;
}
