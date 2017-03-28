
#ifndef __OBJ_LOADER__H__
#define __OBJ_LOADER__H__

#include <glm/glm.hpp>
#include <vector>
#include <string>

struct ObjIndex
{
    unsigned int vertexIndex;
    unsigned int uvIndex;
    unsigned int normalIndex;

    inline bool operator< (const ObjIndex& other) const;
};

struct IndexedModel
{
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;

    void CalcNormal(void);
};

class ObjModel
{
public:
    ObjModel(const std::string& filename);

    IndexedModel ToIndexedModel(void);

private:
    unsigned int FindLastVertexIndex(const std::vector<ObjIndex*>& indexLookup, const ObjIndex* currentIndex, const IndexedModel& result);
    void CreateObjFace(const std::string& line);

    glm::vec2 ParseObjVec2(const std::string& line);
    glm::vec3 ParseObjVec3(const std::string& line);
    ObjIndex ParseObjIndex(const std::string& token, bool* hasUVs, bool* hasNormals);

private:
    std::vector<ObjIndex> objIndices_;
    std::vector<glm::vec3> vertices_;
    std::vector<glm::vec2> uvs_;
    std::vector<glm::vec3> normals_;

    bool hasUVs_;
    bool hasNormals_;
};

//---------------------------------------------------------------------------
// inline Section
//---------------------------------------------------------------------------
inline bool ObjIndex::operator< (const ObjIndex& other) const
{
    return vertexIndex < other.vertexIndex;
}

#endif /*__OBJ_LOADER__H__*/
