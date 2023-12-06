#pragma once
#include "../global/types.h"

class CLevelReader
{
public:
    CLevelReader() {}
    ~CLevelReader() {
        Release();
    }

    bool Load(const char* path);
    void Release();
    void Seek(int to);
    bool IsValid();
    long GetSize();
    void SetAnotherFile(FILE* otherFile);

    char GetChar();
    uchar GetUChar();
    short GetShort();
    ushort GetUShort();
    int GetInt();
    uint GetUInt();
    long GetLong();
    ulong GetULong();
    double GetDouble();
    float GetFloat();
    char* GetCompressedData(int uncompressedSize, int compressedSize);
    void GetCopyToPtr(LPVOID dest, int size);

    template<class T>
    T* GetStructMalloc(int readSize)
    {
        uint size = sizeof(T) * readSize;
        T* data = (T*)malloc(size);
        fread(data, size, 1, s_dataFile);
        return data;
    }

    template<class T>
    T* GetStructGameMalloc(int readSize)
    {
        uint size = sizeof(T) * readSize;
        T* data = (T*)game_malloc(size);
        fread(data, size, 1, s_dataFile);
        return data;
    }

    template<class T>
    T* GetPtr()
    {
        T* data = (T*)(*(T**)s_dataFile);
        s_dataFile += sizeof(T*);
        return data;
    }

    template<class T>
    T** GetArrayStructMalloc(int readSize)
    {
        T** data = (T**)malloc(2 * sizeof(T*) * readSize);
        fread(data, sizeof(T*) * readSize, 1, s_dataFile);
        return data;
    }

    template<class T>
    T** GetArrayStructGameMalloc(int readSize)
    {
        T** data = (T**)game_malloc(2 * sizeof(T*) * readSize);
        fread(data, sizeof(T*) * readSize, 1, s_dataFile);
        return data;
    }

private:
    FILE* s_dataFile = NULL;
};

