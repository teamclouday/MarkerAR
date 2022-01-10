#include "model.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>
#include <iostream>

ModelCube::ModelCube(int camWidth, int camHeight)
{
    _width = camWidth;
    _height = camHeight;
    // data from http://www.opengl-tutorial.org/beginners-tutorials/tutorial-4-a-colored-cube/
    const float vertices[] = {
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f,

        -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f,

         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f,

        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f,

        // -1.0f,-1.0f,-1.0f, 0.583f,  0.771f,  0.014f,
        // -1.0f,-1.0f, 1.0f, 0.609f,  0.115f,  0.436f,
        // -1.0f, 1.0f, 1.0f, 0.327f,  0.483f,  0.844f,
        //  1.0f, 1.0f,-1.0f, 0.822f,  0.569f,  0.201f,
        // -1.0f,-1.0f,-1.0f, 0.435f,  0.602f,  0.223f,
        // -1.0f, 1.0f,-1.0f, 0.310f,  0.747f,  0.185f,
        //  1.0f,-1.0f, 1.0f, 0.597f,  0.770f,  0.761f,
        // -1.0f,-1.0f,-1.0f, 0.559f,  0.436f,  0.730f,
        //  1.0f,-1.0f,-1.0f, 0.359f,  0.583f,  0.152f,
        //  1.0f, 1.0f,-1.0f, 0.483f,  0.596f,  0.789f,
        //  1.0f,-1.0f,-1.0f, 0.559f,  0.861f,  0.639f,
        // -1.0f,-1.0f,-1.0f, 0.195f,  0.548f,  0.859f,
        // -1.0f,-1.0f,-1.0f, 0.014f,  0.184f,  0.576f,
        // -1.0f, 1.0f, 1.0f, 0.771f,  0.328f,  0.970f,
        // -1.0f, 1.0f,-1.0f, 0.406f,  0.615f,  0.116f,
        //  1.0f,-1.0f, 1.0f, 0.676f,  0.977f,  0.133f,
        // -1.0f,-1.0f, 1.0f, 0.971f,  0.572f,  0.833f,
        // -1.0f,-1.0f,-1.0f, 0.140f,  0.616f,  0.489f,
        // -1.0f, 1.0f, 1.0f, 0.997f,  0.513f,  0.064f,
        // -1.0f,-1.0f, 1.0f, 0.945f,  0.719f,  0.592f,
        //  1.0f,-1.0f, 1.0f, 0.543f,  0.021f,  0.978f,
        //  1.0f, 1.0f, 1.0f, 0.279f,  0.317f,  0.505f,
        //  1.0f,-1.0f,-1.0f, 0.167f,  0.620f,  0.077f,
        //  1.0f, 1.0f,-1.0f, 0.347f,  0.857f,  0.137f,
        //  1.0f,-1.0f,-1.0f, 0.055f,  0.953f,  0.042f,
        //  1.0f, 1.0f, 1.0f, 0.714f,  0.505f,  0.345f,
        //  1.0f,-1.0f, 1.0f, 0.783f,  0.290f,  0.734f,
        //  1.0f, 1.0f, 1.0f, 0.722f,  0.645f,  0.174f,
        //  1.0f, 1.0f,-1.0f, 0.302f,  0.455f,  0.848f,
        // -1.0f, 1.0f,-1.0f, 0.225f,  0.587f,  0.040f,
        //  1.0f, 1.0f, 1.0f, 0.517f,  0.713f,  0.338f,
        // -1.0f, 1.0f,-1.0f, 0.053f,  0.959f,  0.120f,
        // -1.0f, 1.0f, 1.0f, 0.393f,  0.621f,  0.362f,
        //  1.0f, 1.0f, 1.0f, 0.673f,  0.211f,  0.457f,
        // -1.0f, 1.0f, 1.0f, 0.820f,  0.883f,  0.371f,
        //  1.0f,-1.0f, 1.0f, 0.982f,  0.099f,  0.879f
    };
    _count = static_cast<int>(sizeof(vertices) / sizeof(float) / 6);
    GLuint vbo;
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    // load shaders
    _shader = std::make_shared<Shader>();
    _shader->add("shaders/model.unlit.vert.glsl", GL_VERTEX_SHADER);
    _shader->add("shaders/model.unlit.frag.glsl", GL_FRAGMENT_SHADER);
    _shader->compile();
    _lineMode = false;
}

ModelCube::~ModelCube()
{
    glDeleteVertexArrays(1, &_vao);
}

void ModelCube::render(
    const glm::mat3& cameraK, const glm::mat4x3& poseM,
    float cameraRatio, float windowRatio
)
{
    glEnable(GL_DEPTH_TEST);
    if(_lineMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glUseProgram(_shader->program());
    glm::mat4 modelMatrix = glm::translate(
        glm::scale(glm::mat4(1.0f), glm::vec3(_scale)),
        _translation
    );
    _shader->uniformMat4x3("poseM", poseM);
    _shader->uniformMat4x4("modelMat", modelMatrix);
    _shader->uniformMat3x3("cameraK", cameraK);
    _shader->uniformFloat("ratio_img", cameraRatio);
    _shader->uniformFloat("ratio_win", windowRatio);
    _shader->uniformFloat("cam_width", _width);
    _shader->uniformFloat("cam_height", _height);
    glBindVertexArray(_vao);
    glDrawArrays(GL_TRIANGLES, 0, _count);
    glBindVertexArray(0);
    glUseProgram(0);
    if(_lineMode) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_DEPTH_TEST);
}



ModelTeapot::ModelTeapot(int camWidth, int camHeight)
{
    _width = camWidth;
    _height = camHeight;
    // load obj data
    std::vector<glm::vec3> vertices;
    std::vector<unsigned> indices;
    if(!loadObj("models/teapot.obj", vertices, indices))
        throw std::runtime_error("Failed to load teapot!");
    _count = static_cast<int>(indices.size());
    // create array buffers
    GLuint vbo;
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &_ebo);
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float)*3, vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned), indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    // load shaders
    _shader = std::make_shared<Shader>();
    _shader->add("shaders/model.lighted.vert.glsl", GL_VERTEX_SHADER);
    _shader->add("shaders/model.lighted.frag.glsl", GL_FRAGMENT_SHADER);
    _shader->compile();
    _lineMode = false;
}

ModelTeapot::~ModelTeapot()
{
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_ebo);
}

void ModelTeapot::render(
    const glm::mat3& cameraK, const glm::mat4x3& poseM,
    float cameraRatio, float windowRatio
)
{
    glEnable(GL_DEPTH_TEST);
    if(_lineMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glUseProgram(_shader->program());
    glm::mat4 modelMatrix = glm::translate(
        glm::scale(glm::mat4(1.0f), glm::vec3(_scale)),
        _translation
    );
    _shader->uniformMat4x3("poseM", poseM);
    _shader->uniformMat4x4("modelMat", modelMatrix);
    _shader->uniformMat3x3("cameraK", cameraK);
    _shader->uniformFloat("ratio_img", cameraRatio);
    _shader->uniformFloat("ratio_win", windowRatio);
    _shader->uniformFloat("cam_width", _width);
    _shader->uniformFloat("cam_height", _height);
    _shader->uniformVec3("lightPos", _lightPos);
    _shader->uniformVec3("diffuse", _model_diffuse);
    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, _count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    if(_lineMode) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_DEPTH_TEST);
}


ModelBunny::ModelBunny(int camWidth, int camHeight)
{
    _width = camWidth;
    _height = camHeight;
    // load obj data
    std::vector<glm::vec3> vertices;
    std::vector<unsigned> indices;
    if(!loadObj("models/bunny.obj", vertices, indices))
        throw std::runtime_error("Failed to load bunny!");
    _count = static_cast<int>(indices.size());
    // create array buffers
    GLuint vbo;
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &_ebo);
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float)*3, vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned), indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    // load shaders
    _shader = std::make_shared<Shader>();
    _shader->add("shaders/model.lighted.vert.glsl", GL_VERTEX_SHADER);
    _shader->add("shaders/model.lighted.frag.glsl", GL_FRAGMENT_SHADER);
    _shader->compile();
    _lineMode = false;
}

ModelBunny::~ModelBunny()
{
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_ebo);
}

void ModelBunny::render(
    const glm::mat3& cameraK, const glm::mat4x3& poseM,
    float cameraRatio, float windowRatio
)
{
    glEnable(GL_DEPTH_TEST);
    if(_lineMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glUseProgram(_shader->program());
    glm::mat4 modelMatrix = glm::translate(
        glm::scale(glm::mat4(1.0f), glm::vec3(_scale)),
        _translation
    );
    _shader->uniformMat4x3("poseM", poseM);
    _shader->uniformMat4x4("modelMat", modelMatrix);
    _shader->uniformMat3x3("cameraK", cameraK);
    _shader->uniformFloat("ratio_img", cameraRatio);
    _shader->uniformFloat("ratio_win", windowRatio);
    _shader->uniformFloat("cam_width", _width);
    _shader->uniformFloat("cam_height", _height);
    _shader->uniformVec3("lightPos", _lightPos);
    _shader->uniformVec3("diffuse", _model_diffuse);
    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, _count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    if(_lineMode) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_DEPTH_TEST);
}