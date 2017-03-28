
#include "obj_loader.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <map>

//---------------------------------------------------------------------------
// define - Local
//---------------------------------------------------------------------------
namespace
{
    bool CompareObjIndexPtr(const ObjIndex* a, const ObjIndex* b)
    {
        return a->vertexIndex < b->vertexIndex;
    }

    unsigned int FindNextChar(unsigned int start, const char* str, unsigned int length, char token)
    {
        unsigned int result = start;
        while (result < length)
        {
            ++result;
            if (str[result] == token)
            {
                break;
            }
        }

        return result;
    }

    unsigned int ParseObjIndexValue(const std::string& token, unsigned int start, unsigned int end)
    {
        return atoi(token.substr(start, end - start).c_str()) - 1;
    }

    float ParseObjFloatValue(const std::string& token, unsigned int start, unsigned int end)
    {
        return atof(token.substr(start, end - start).c_str());
    }
    
    std::vector<std::string> SplitString(const std::string& s, char delim)
    {
        std::vector<std::string> elems;

        const char* cstr = s.c_str();
        unsigned int strlength = s.length();
        unsigned int start = 0;
        unsigned int end = 0;

        while (end <= strlength)
        {
            while (end <= strlength)
            {
                if (cstr[end] == delim)
                {
                    break;
                }
                ++end;
            }

            elems.push_back(s.substr(start, end - start));
            start = end + 1;
            end = start;
        }

        return elems;
    }
}

//---------------------------------------------------------------------------
// define - IndexedModel
//---------------------------------------------------------------------------
void IndexedModel::CalcNormal(void)
{
    for (unsigned int i = 0; i < indices.size(); i += 3)
    {
        int i0 = indices[i];
        int i1 = indices[i + 1];
        int i2 = indices[i + 2];

        glm::vec3 v1 = positions[i1] - positions[i0];
        glm::vec3 v2 = positions[i2] - positions[i0];

        glm::vec3 normal = glm::normalize(glm::cross(v1, v2));

        normals[i0] += normal;
        normals[i1] += normal;
        normals[i2] += normal;
    }

    for (unsigned int i = 0; i < positions.size(); ++i)
    {
        normals[i] = glm::normalize(normals[i]);
    }
}

//---------------------------------------------------------------------------
// define - ObjModel
//---------------------------------------------------------------------------    
ObjModel::ObjModel(const std::string& filename)
{
    hasUVs_ = false;
    hasNormals_ = false;
    std::ifstream file;
    file.open(filename.c_str());

    std::string line;
    if (file.is_open() == true)
    {
        while (file.good() == true)
        {
            getline(file, line);
            unsigned int lineLength = line.length();
            if (lineLength < 2)
            {
                continue;
            }

            const char* lineCStr = line.c_str();
            switch (lineCStr[0])
            {
                case 'v' :
                {
                    if (lineCStr[1] == 't')
                    {
                        this->uvs_.push_back(ParseObjVec2(line));
                    }
                    else if (lineCStr[1] == 'n')
                    {
                        this->normals_.push_back(ParseObjVec3(line));
                    }
                    else if (lineCStr[1] == ' ' || lineCStr[1] == '\t')
                    {
                       this->vertices_.push_back(ParseObjVec3(line));
                    }
                }
                break;
                case 'f' :
                {
                    CreateObjFace(line);                    
                }
                break;
                default:
                break;
            }
        }
    }
    else
    {
        std::cerr << "Unable to laod mesh!" << std::endl;
    }
}

IndexedModel ObjModel::ToIndexedModel(void)
{
    IndexedModel result;
    IndexedModel normalModel;

    unsigned int numIndedices = objIndices_.size();

    std::vector<ObjIndex*> indexLookup;
    for (unsigned int i = 0; i < numIndedices; ++i)
    {
        indexLookup.push_back(&objIndices_[i]);
    }

    std::sort(indexLookup.begin(), indexLookup.end(), CompareObjIndexPtr);

    std::map<ObjIndex, unsigned int> normalModeIndexMap;
    std::map<unsigned int, unsigned int> indexMap;

    for (unsigned int i = 0; i < numIndedices; ++i)
    {
        ObjIndex* currentIndex = &objIndices_[i];

        glm::vec3 currentPosition = vertices_[currentIndex->vertexIndex];
        glm::vec2 currentTexCoord;
        glm::vec3 currentNormal;

        if (hasUVs_ == true)
        {
            currentTexCoord = uvs_[currentIndex->uvIndex];
        }
        else 
        {
            currentTexCoord = glm::vec2(0.0f, 0.0f);
        }

        if (hasNormals_ == true)
        {
            currentNormal = normals_[currentIndex->normalIndex];
        }
        else
        {
            currentNormal = glm::vec3(0.0f, 0.0f, 0.0f);
        }

        unsigned int normalModelIndex;
        unsigned int resultModelIndex;

        //std::map<ObjIndex, unsigned int
        auto it = normalModeIndexMap.find(*currentIndex);
        if (it == normalModeIndexMap.end())
        {
            normalModelIndex = normalModel.positions.size();

            normalModeIndexMap.insert(std::make_pair(*currentIndex, normalModelIndex));
            normalModel.positions.push_back(currentPosition);
            normalModel.texCoords.push_back(currentTexCoord);
            normalModel.normals.push_back(currentNormal);
        }
        else
        {
            normalModelIndex = it->second;
        }

        unsigned int previousVertexLocation = FindLastVertexIndex(indexLookup, currentIndex, result);
        if (previousVertexLocation == unsigned int(-1))
        {
            resultModelIndex = result.positions.size();
            result.positions.push_back(currentPosition);
            result.texCoords.push_back(currentTexCoord);
            result.normals.push_back(currentNormal);
        }
        else
        {
            resultModelIndex = previousVertexLocation;
        }

        normalModel.indices.push_back(normalModelIndex);
        result.indices.push_back(resultModelIndex);
        indexMap.insert(std::make_pair(resultModelIndex, normalModelIndex));
    }

    if (hasNormals_ == false)
    {
        normalModel.CalcNormal();
        for (unsigned int i = 0; i < result.positions.size(); ++i)
        {
            result.normals[i] = normalModel.normals[indexMap[i]];
        }
    }

    return result;
}

unsigned int ObjModel::FindLastVertexIndex(const std::vector<ObjIndex*>& indexLookup, const ObjIndex* currentIndex, const IndexedModel& result)
{
    unsigned int start = 0;
    unsigned int end = indexLookup.size();
    unsigned int current = (end - start) / 2 + start;
    unsigned int previous = start;

    while (current != previous)
    {
        ObjIndex* testIndex = indexLookup[current];

        if (testIndex->vertexIndex == currentIndex->vertexIndex)
        {
            unsigned int constStart = current;

            for (unsigned int i = 0; i < current; ++i)
            {
                ObjIndex* possibleIndex = indexLookup[current - i];
                if (possibleIndex == currentIndex)
                {
                    continue;
                }

                if (possibleIndex->vertexIndex != currentIndex->vertexIndex)
                {
                    break;
                }

                --constStart;
            }

            for (unsigned int i = constStart; i < indexLookup.size() - constStart; ++i)
            {
                ObjIndex* possibleIndex = indexLookup[current + i];

                if (possibleIndex == currentIndex)
                {
                    continue;
                }

                if (possibleIndex->vertexIndex != currentIndex->vertexIndex)
                {
                    break;
                }
                else if ((hasUVs_ == false || possibleIndex->uvIndex == currentIndex->uvIndex)
                    && (hasNormals_ ==false || possibleIndex->normalIndex == currentIndex->normalIndex))
                {
                    glm::vec3 currentPosition = vertices_[currentIndex->vertexIndex];
                    glm::vec2 currentTexcoord;
                    glm::vec3 currentNormal;

                    if (hasUVs_ == true)
                    {
                        currentTexcoord = uvs_[currentIndex->uvIndex];
                    }
                    else
                    {
                        currentTexcoord = glm::vec2(0.0f, 0.0f);
                    }
                    if (hasNormals_ == true)
                    {
                        currentNormal = normals_[currentIndex->normalIndex];
                    }
                    else
                    {
                        currentNormal = glm::vec3(0.0f, 0.0f, 0.0f);
                    }
                    
                    for (unsigned int j = 0; j < result.positions.size(); ++j)
                    {
                        if (currentPosition == result.positions[j]
                            && (hasUVs_ == false || currentTexcoord == result.texCoords[i])
                            && (hasNormals_ == false || currentNormal == result.normals[i]))
                        {
                            return j;
                        }
                    }
                }
            }
        }
        else
        {
            if (testIndex->vertexIndex < currentIndex->vertexIndex)
            {
                start = current;
            }
            else
            {
                end = current;
            }
        }

        previous = current;
        current = (end - start) / 2 + start;
    }

    return -1;
}

void ObjModel::CreateObjFace(const std::string& line)
{   
    std::vector<std::string> tokens = SplitString(line, ' ');

    objIndices_.push_back(ParseObjIndex(tokens[1], &hasUVs_, &hasNormals_));
    objIndices_.push_back(ParseObjIndex(tokens[2], &hasUVs_, &hasNormals_));
    objIndices_.push_back(ParseObjIndex(tokens[3], &hasUVs_, &hasNormals_));

    if (int(tokens.size()) > 4)
    {
        objIndices_.push_back(ParseObjIndex(tokens[1], &hasUVs_, &hasNormals_));
        objIndices_.push_back(ParseObjIndex(tokens[3], &hasUVs_, &hasNormals_));
        objIndices_.push_back(ParseObjIndex(tokens[4], &hasUVs_, &hasNormals_));
    }
}


glm::vec2 ObjModel::ParseObjVec2(const std::string& line)
{
    unsigned int tokenLength = line.length();
    const char* tokenString = line.c_str();

    unsigned int vertexIndexStart = 3;

    while (vertexIndexStart < tokenLength)
    {
        if (tokenString[vertexIndexStart] != ' ')
        {
            break;
        }
        ++ vertexIndexStart;
    }

    unsigned int vertexIndexEnd = FindNextChar(vertexIndexStart, tokenString, tokenLength, ' ');
    float x = ParseObjFloatValue(line, vertexIndexStart, vertexIndexEnd);

    vertexIndexStart = vertexIndexEnd + 1;
    vertexIndexEnd = FindNextChar(vertexIndexStart, tokenString, tokenLength, ' ');
    float y = ParseObjFloatValue(line, vertexIndexStart, vertexIndexEnd);

    return glm::vec2(x, y);
}

glm::vec3 ObjModel::ParseObjVec3(const std::string& line)
{
    unsigned int tokenLength = line.length();
    const char* tokenString = line.c_str();

    unsigned int vertexIndexStart = 2;

    while (vertexIndexStart < tokenLength)
    {
        if (tokenString[vertexIndexStart] != ' ')
        {
            break;
        }
        ++ vertexIndexStart;
    }

    unsigned int vertexIndexEnd = FindNextChar(vertexIndexStart, tokenString, tokenLength, ' ');
    float x = ParseObjFloatValue(line, vertexIndexStart, vertexIndexEnd);

    vertexIndexStart = vertexIndexEnd + 1;
    vertexIndexEnd = FindNextChar(vertexIndexStart, tokenString, tokenLength, ' ');
    float y = ParseObjFloatValue(line, vertexIndexStart, vertexIndexEnd);

    vertexIndexStart = vertexIndexEnd + 1;
    vertexIndexEnd = FindNextChar(vertexIndexStart, tokenString, tokenLength, ' ');
    float z = ParseObjFloatValue(line, vertexIndexStart, vertexIndexEnd);

    return glm::vec3(x, y, z);
}

ObjIndex ObjModel::ParseObjIndex(const std::string& token, bool* hasUVs, bool* hasNormals)
{
    unsigned int tokenLength = token.length();
    const char* tokenString = token.c_str();

    unsigned int vertexIndexStart = 0;
    unsigned int vertexIndexEnd = FindNextChar(vertexIndexStart, tokenString, tokenLength, '/');

    ObjIndex result;
    result.vertexIndex = ParseObjIndexValue(token, vertexIndexStart, vertexIndexEnd);
    result.uvIndex = 0;
    result.normalIndex = 0;

    if (vertexIndexEnd >= tokenLength)
    {
        return result;
    }

    vertexIndexStart = vertexIndexEnd + 1;
    vertexIndexEnd = FindNextChar(vertexIndexStart, tokenString, tokenLength, '/');
    result.uvIndex = ParseObjIndexValue(token, vertexIndexStart, vertexIndexEnd);
    *hasUVs = true;

    if (vertexIndexEnd >= tokenLength)
    {
        return result;
    }

    vertexIndexStart = vertexIndexEnd + 1;
    vertexIndexEnd = FindNextChar(vertexIndexStart, tokenString, tokenLength, '/');
    result.normalIndex = ParseObjIndexValue(token, vertexIndexStart, vertexIndexEnd);
    *hasNormals = true;

    return result;
}