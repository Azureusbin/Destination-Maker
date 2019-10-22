#pragma once

#ifdef ISYSTEM_PROJECT
#include <CryCore\Platform\CryWindows.h>
#else
#include <Windows.h>
#endif

#include <wincrypt.h>

#pragma pack(1)
struct SDestinationV1
{
	char name[20];
	char initialsPy[20];
	int floorNum;

	bool operator==(SDestinationV1 &rhs)
	{
		return stricmp(name, rhs.name) == 0;
	}
};

enum type_invaild { INVAILD };

struct DEST_UID
{
	unsigned char data[4];

	explicit DEST_UID() {};

	constexpr DEST_UID(type_invaild) : data{0,0,0,0} {}

	constexpr DEST_UID(unsigned char x, unsigned char y, unsigned char z, unsigned char w) : data{ x,y,z,w } {}

	inline bool IsInvaild() { 
		return data[0] == INVAILD
		&&data[1] == INVAILD
		&&data[2] == INVAILD
		&&data[3] == INVAILD; }

	inline void GenerateUID()
	{
		HCRYPTPROV Rnd;
		LPCSTR UserName = "SDestinationV2_Container";

		if (!CryptAcquireContextA(&Rnd, UserName, NULL, PROV_RSA_FULL, 0))
		{
			if (GetLastError() == NTE_BAD_KEYSET)
			{
				CryptAcquireContextA(&Rnd, UserName, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET);
			}
		}

		CryptGenRandom(Rnd, 4, (BYTE*)(&data));
		CryptReleaseContext(Rnd, 0);
	}

	unsigned char& operator[](int i)
	{
		return data[i];
	}

	inline DEST_UID& operator=(type_invaild)
	{
		memset(this, 0, 4);
		return *this;
	}

	inline bool operator==(const DEST_UID &rhs)
	{
		return data[0] == rhs.data[0] && data[1] == rhs.data[1] && data[2] == rhs.data[2] && data[3] == rhs.data[3];
	}

	static constexpr DEST_UID FromStringInternal(const char* sz)
	{
		return DEST_UID{ HexStringToUInt8(sz),HexStringToUInt8(sz + 3),HexStringToUInt8(sz + 6),HexStringToUInt8(sz + 9) };
	}

	constexpr static unsigned char HexCharToUInt8(char x)
	{
		return x >= '0' && x <= '9' ? x - '0' :
			x >= 'a' && x <= 'f' ? x - 'a' + 10 :
			x >= 'A' && x <= 'F' ? x - 'A' + 10 : 0;
	}

	constexpr static unsigned char HexStringToUInt8(const char* szInput)
	{
		return (HexCharToUInt8(szInput[0]) << 4) + HexCharToUInt8(szInput[1]);
	}

	inline void ToString(char *output, size_t const output_size_in_bytes) const
	{
		sprintf(output, "%02X-%02X-%02X-%02X", data[0], data[1], data[2], data[3]);
	}
};

constexpr DEST_UID operator"" _dest_uid(const char* szInput, size_t)
{
	return (szInput[0] == '{') ? DEST_UID::FromStringInternal(szInput + 1) : DEST_UID::FromStringInternal(szInput);
}

struct SDestinationV2
{
	char name[36];
	DEST_UID uid;
	char initialsPy[20];
	int floorNum;

	void GenerateUid()
	{
		uid.GenerateUID();
	}

	// format {xxx-xxx-xxx-xxx}
	void GetUID(void * _Dst, size_t _Size)
	{
		if (_Size >= 16)
		{
			uid.ToString((char*)_Dst, _Size);
		}
	}

	SDestinationV2& operator=(const SDestinationV1& rhs)
	{
		memset(this, 0, sizeof(SDestinationV2));

		GenerateUid();

		memcpy(name, rhs.name, sizeof(rhs.name));
		memcpy(initialsPy, rhs.initialsPy, sizeof(rhs.initialsPy));
		floorNum = rhs.floorNum;

		return *this;
	}

	bool operator==(SDestinationV2 &rhs)
	{
		return uid == rhs.uid;
	}
};

struct DEST_DATA_HEADER
{
	float Version;
	int Length;
};

#pragma pack ()