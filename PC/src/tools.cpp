#include "camera.hpp"
#include "model.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

void Camera::loadCameraData()
{
    const std::string filename = "camera.txt";
    std::ifstream inFile(filename.c_str());
    if(inFile.good())
    {
        float cameraCalibWidth = 1.0f, cameraCalibHeight = 1.0f;
        std::string line;
        std::stringstream sstr;
        float x = 0.0f;
        int y = 0;
        char dot = ',';

        // first line get measurement picture size
        if(!std::getline(inFile, line)) goto invaliddata;
        sstr.clear();
        sstr.str(line);
        if(!(sstr >> y)) goto invaliddata;
        cameraCalibHeight = cameraCalibHeight / y;
        if(!(sstr >> dot)) goto invaliddata;
        if(!(sstr >> y)) goto invaliddata;
        cameraCalibWidth = cameraCalibWidth / y;

        // second line get camera matrix
        if(!std::getline(inFile, line)) goto invaliddata;
        sstr.clear();
        sstr.str(line);
        if(!(sstr >> x)) goto invaliddata;
        _camK[0][0] = x;
        if(!(sstr >> dot)) goto invaliddata;
        if(!(sstr >> x)) goto invaliddata;
        _camK[1][0] = x;
        if(!(sstr >> dot)) goto invaliddata;
        if(!(sstr >> x)) goto invaliddata;
        _camK[2][0] = x;

        if(!std::getline(inFile, line)) goto invaliddata;
        sstr.clear();
        sstr.str(line);
        if(!(sstr >> x)) goto invaliddata;
        _camK[0][1] = x;
        if(!(sstr >> dot)) goto invaliddata;
        if(!(sstr >> x)) goto invaliddata;
        _camK[1][1] = x;
        if(!(sstr >> dot)) goto invaliddata;
        if(!(sstr >> x)) goto invaliddata;
        _camK[2][1] = x;

        if(!std::getline(inFile, line)) goto invaliddata;
        sstr.clear();
        sstr.str(line);
        if(!(sstr >> x)) goto invaliddata;
        _camK[0][2] = x;
        if(!(sstr >> dot)) goto invaliddata;
        if(!(sstr >> x)) goto invaliddata;
        _camK[1][2] = x;
        if(!(sstr >> dot)) goto invaliddata;
        if(!(sstr >> x)) goto invaliddata;
        _camK[2][2] = x;

        // last line get distortion coefficients
        if(!std::getline(inFile, line)) goto validdata;
        sstr.clear();
        sstr.str(line);
        if(!(sstr >> x)) goto validdata;
        _camDistCoeffK[0] = x;
        if(!(sstr >> dot)) goto validdata;
        if(!(sstr >> x)) goto validdata;
        _camDistCoeffK[1] = x;
        if(!(sstr >> dot)) goto validdata;
        if(!(sstr >> x)) goto validdata;
        _camDistCoeffP[0] = x;
        if(!(sstr >> dot)) goto validdata;
        if(!(sstr >> x)) goto validdata;
        _camDistCoeffP[1] = x;
        if(!(sstr >> dot)) goto validdata;
        if(!(sstr >> x)) goto validdata;
        _camDistCoeffK[2] = x;

        validdata:
        _camK[0][0] *= _width * cameraCalibWidth;
        _camK[2][0] *= _width * cameraCalibWidth;
        _camK[1][1] *= _height * cameraCalibHeight;
        _camK[2][1] *= _height * cameraCalibHeight;
        _camInvK = glm::inverse(_camK);
        inFile.close();
        return;
    }

    invaliddata:
    inFile.close();
    std::cout << "Warn: invalid camera.txt! Will use default parameters!" << std::endl;
    // camera intrinsic and distortion data measured on my webcam
    _camK = glm::mat3(
        glm::vec3(776.39107688f, 0.0f,          0.0f),
        glm::vec3(0.0f,          770.19292802f, 0.0f),
        glm::vec3(346.33206594f, 315.46822203f, 1.0f)
    );
    const float cameraCalibWidth = 1.0f / 800.0f, cameraCalibHeight = 1.0f / 600.0f;
    _camK[0][0] *= _width * cameraCalibWidth;
    _camK[2][0] *= _width * cameraCalibWidth;
    _camK[1][1] *= _height * cameraCalibHeight;
    _camK[2][1] *= _height * cameraCalibHeight;
    _camInvK = glm::inverse(_camK);
    _camDistCoeffK = glm::vec3(-3.58177671e-1,  5.58704262e-1, -8.93211088e-1);
    _camDistCoeffP = glm::vec2(8.46171348e-4, -4.12405134e-3);
}

bool Model::loadObj(
    const std::string& filename,
    std::vector<glm::vec3>& vertices,
    std::vector<unsigned>& indices
)
{
    // read file
    std::ifstream inFile(filename.c_str());
    if(!inFile.good())
    {
        std::cout << "Failed to read obj file: " << filename << std::endl;
        return false;
    }
    std::string type;
    float vertVal = 0.0f;
    unsigned int indVal = 0;
    std::string line;
    std::stringstream sstr;
    // prepare array
    vertices.resize(0);
    vertices.clear();
    indices.resize(0);
    indices.clear();
    glm::vec3 vertex(0.0f);
    glm::uvec3 face(0);
    // start reading
    while(std::getline(inFile, line))
    {
        sstr.clear();
        sstr.str(line);
        if(!(sstr >> type)) continue;
        if(type.length() > 1) continue;
        if(type[0] == 'v')
        {
            if(sstr >> vertVal)
                vertex.x = vertVal;
            else continue;
            if(sstr >> vertVal)
                vertex.y = vertVal;
            else continue;
            if(sstr >> vertVal)
                vertex.z = vertVal;
            else continue;
            vertices.push_back(vertex);
        }
        else if(type[0] == 'f')
        {
            if(sstr >> indVal)
                face.x = indVal;
            else continue;
            if(sstr >> indVal)
                face.y = indVal;
            else continue;
            if(sstr >> indVal)
                face.z = indVal;
            else continue;
            indices.push_back(face.x - 1);
            indices.push_back(face.y - 1);
            indices.push_back(face.z - 1);
        }
    }
    if(vertices.size() == 0 || indices.size() == 0) return false;
    return true;
}

// TODO: further optimize this to include index buffer
bool Model::loadObj(
    const std::string& filename,
    // vertex position, normal, and uv
    std::vector<float>& buffer
    // std::vector<std::tuple<glm::vec3,glm::vec3,glm::vec2>>& buffer
)
{
    // read file
    std::ifstream inFile(filename.c_str());
    if(!inFile.good())
    {
        std::cout << "Failed to read obj file: " << filename << std::endl;
        return false;
    }
    char skip = '/';
    std::string type;
    float floatVal = 0.0f;
    int uintVal = 0;
    std::string line;
    std::stringstream sstr;
    // prepare array
    buffer.resize(0);
    buffer.clear();
    // temporary buffer (offset by 1)
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    glm::vec3 vertex(0.0f);
    glm::vec3 normal(0.0f);
    glm::vec2 uv(0.0f);
    glm::uvec3 face(0);

    unsigned int faceCount = 0;
    // start reading
    while(std::getline(inFile, line))
    {
        sstr.clear();
        sstr.str(line);
        if(!(sstr >> type)) continue;
        if(type == "v")
        {
            if(sstr >> floatVal)
                vertex.x = floatVal;
            else continue;
            if(sstr >> floatVal)
                vertex.y = floatVal;
            else continue;
            if(sstr >> floatVal)
                vertex.z = floatVal;
            else continue;
            vertices.push_back(vertex);
        }
        else if(type == "vn")
        {
            if(sstr >> floatVal)
                normal.x = floatVal;
            else continue;
            if(sstr >> floatVal)
                normal.y = floatVal;
            else continue;
            if(sstr >> floatVal)
                normal.z = floatVal;
            else continue;
            normals.push_back(normal);
        }
        else if(type == "vt")
        {
            if(sstr >> floatVal)
                uv.x = floatVal;
            else continue;
            if(sstr >> floatVal)
                uv.y = floatVal;
            else continue;
            uvs.push_back(uv);
        }
        else if(type == "f")
        {
            faceCount++;
            size_t verticesSize = vertices.size();
            size_t uvsSize = uvs.size();
            size_t normalsSize = normals.size();
            int vertexIdx[3], uvIdx[3], normalIdx[3];
            if((sstr >> uintVal) && uintVal <= verticesSize)
                vertexIdx[0] = uintVal;
            else continue;
            if(!(sstr >> skip)) continue;
            if((sstr >> uintVal) && uintVal <= uvsSize)
                uvIdx[0] = uintVal;
            else continue;
            if(!(sstr >> skip)) continue;
            if((sstr >> uintVal) && uintVal <= normalsSize)
                normalIdx[0] = uintVal;
            else continue;

            if((sstr >> uintVal) && uintVal <= verticesSize)
                vertexIdx[1] = uintVal;
            else continue;
            if(!(sstr >> skip)) continue;
            if((sstr >> uintVal) && uintVal <= uvsSize)
                uvIdx[1] = uintVal;
            else continue;
            if(!(sstr >> skip)) continue;
            if((sstr >> uintVal) && uintVal <= normalsSize)
                normalIdx[1] = uintVal;
            else continue;

            if((sstr >> uintVal) && uintVal <= verticesSize)
                vertexIdx[2] = uintVal;
            else continue;
            if(!(sstr >> skip)) continue;
            if((sstr >> uintVal) && uintVal <= uvsSize)
                uvIdx[2] = uintVal;
            else continue;
            if(!(sstr >> skip)) continue;
            if((sstr >> uintVal) && uintVal <= normalsSize)
                normalIdx[2] = uintVal;
            else continue;

            for(int i = 0; i < 3; i++)
            {
                buffer.push_back(vertices[vertexIdx[i]-1].x);
                buffer.push_back(vertices[vertexIdx[i]-1].y);
                buffer.push_back(vertices[vertexIdx[i]-1].z);

                buffer.push_back(normals[normalIdx[i]-1].x);
                buffer.push_back(normals[normalIdx[i]-1].y);
                buffer.push_back(normals[normalIdx[i]-1].z);

                buffer.push_back(uvs[uvIdx[i]-1].x);
                buffer.push_back(uvs[uvIdx[i]-1].y);
            }
        }
    }
    if(buffer.size() == 0) return false;
    return true;
}

bool Model::loadTexture(const std::string& filename, GLuint& texIdx)
{
    stbi_set_flip_vertically_on_load(true);
    int width, height, channels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb);
    if(!data) return false;
    glGenTextures(1, &texIdx);
    glBindTexture(GL_TEXTURE_2D, texIdx);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}