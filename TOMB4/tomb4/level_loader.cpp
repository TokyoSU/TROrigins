#include "../tomb4/pch.h"
#include "level_loader.h"
#include "../specific/file.h"
#include "../specific/function_stubs.h"

bool CLevelReader::Load(const char* path)
{
    s_dataFile = fopen(path, "rb");
    return s_dataFile != NULL;
}

void CLevelReader::Release()
{
    if (s_dataFile)
    {
        fclose(s_dataFile);
        s_dataFile = NULL;
    }
}

void CLevelReader::Seek(int to)
{
    fseek(s_dataFile, to, SEEK_CUR);
}

bool CLevelReader::IsValid()
{
    return s_dataFile != NULL && GetSize() > 0;
}

long CLevelReader::GetSize()
{
    fseek(s_dataFile, 0, SEEK_END);
    long size = ftell(s_dataFile);
    fseek(s_dataFile, 0, SEEK_SET);
    return size;
}

void CLevelReader::SetAnotherFile(FILE* otherFile)
{
    s_dataFile = otherFile;
}

char CLevelReader::GetChar()
{
    char data = 0;
    fread(&data, sizeof(char), 1, s_dataFile);
    return data;
}

uchar CLevelReader::GetUChar()
{
    uchar data = 0;
    fread(&data, sizeof(uchar), 1, s_dataFile);
    return data;
}

short CLevelReader::GetShort()
{
    short data = 0;
    fread(&data, sizeof(short), 1, s_dataFile);
    return data;
}

ushort CLevelReader::GetUShort()
{
    ushort data = 0;
    fread(&data, sizeof(ushort), 1, s_dataFile);
    return data;
}

int CLevelReader::GetInt()
{
    int data = 0;
    fread(&data, sizeof(int), 1, s_dataFile);
    return data;
}

uint CLevelReader::GetUInt()
{
    uint data = 0;
    fread(&data, sizeof(uint), 1, s_dataFile);
    return data;
}

long CLevelReader::GetLong()
{
    long data = 0;
    fread(&data, sizeof(long), 1, s_dataFile);
    return data;
}

ulong CLevelReader::GetULong()
{
    ulong data = 0;
    fread(&data, sizeof(ulong), 1, s_dataFile);
    return data;
}

double CLevelReader::GetDouble()
{
    double data = 0;
    fread(&data, sizeof(double), 1, s_dataFile);
    return data;
}

float CLevelReader::GetFloat()
{
    float data = 0;
    fread(&data, sizeof(float), 1, s_dataFile);
    return data;
}

char* CLevelReader::GetCompressedData(int uncompressedSize, int compressedSize)
{
    if (compressedSize <= 0 || uncompressedSize <= 0)
        return NULL;
    char* resultData = (char*)malloc(uncompressedSize);
    char* compressedData = (char*)malloc(compressedSize);
    if (compressedData == NULL)
        return NULL;
    fread(compressedData, compressedSize, 1, s_dataFile);
    Decompress(resultData, compressedData, compressedSize, uncompressedSize);
    free(compressedData);
    return resultData;
}

void CLevelReader::GetCopyToPtr(LPVOID dest, int size)
{
    fread(dest, size, 1, s_dataFile);
}
