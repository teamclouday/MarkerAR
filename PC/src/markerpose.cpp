#include "marker.hpp"
#include <Eigen/SVD>
#include <Eigen/Dense>
#include <cmath>
#include <iostream>

// estimate pose from homography
// refer to book: "Augmented Reality: Principles and Practice"
void Marker::estimatePose(const glm::mat3& cameraK, const glm::mat3& cameraInvK)
{
    if(_marker_borderp1p2.x == 0.0f)
    {
        _poseH = glm::mat3(0.0f);
        return;
    }
    // prepare p
    glm::vec3 p1 = glm::vec3(_marker_borderp1p2.x, _marker_borderp1p2.y, 1.0f);
    glm::vec3 p2 = glm::vec3(_marker_borderp1p2.z, _marker_borderp1p2.w, 1.0f);
    glm::vec3 p3 = glm::vec3(_marker_borderp3p4.x, _marker_borderp3p4.y, 1.0f);
    glm::vec3 p4 = glm::vec3(_marker_borderp3p4.z, _marker_borderp3p4.w, 1.0f);
    // prepare q
    static glm::vec3 q1 = glm::vec3(0.0f, 0.0f, 1.0f);
    static glm::vec3 q2 = glm::vec3(0.0f, 1.0f, 1.0f);
    static glm::vec3 q3 = glm::vec3(1.0f, 0.0f, 1.0f);
    static glm::vec3 q4 = glm::vec3(1.0f, 1.0f, 1.0f);
    // set up matrix A
    Eigen::Matrix<float, 8, 9> A;
    // A << 0.0f,      0.0f,      0.0f,      -p1.z*q1.x, -p1.z*q1.y, -p1.z*q1.z,  p1.y*q1.x,  p1.y*q1.y,  p1.y*q1.z,
    //      p1.z*q1.x, p1.z*q1.y, p1.z*q1.z,  0.0f,       0.0f,       0.0f,      -p1.x*q1.x, -p1.x*q1.y, -p1.x*q1.z,

    //      0.0f,      0.0f,      0.0f,      -p2.z*q2.x, -p2.z*q2.y, -p2.z*q2.z,  p2.y*q2.x,  p2.y*q2.y,  p2.y*q2.z,
    //      p2.z*q2.x, p2.z*q2.y, p2.z*q2.z,  0.0f,       0.0f,       0.0f,      -p2.x*q2.x, -p2.x*q2.y, -p2.x*q2.z,

    //      0.0f,      0.0f,      0.0f,      -p3.z*q3.x, -p3.z*q3.y, -p3.z*q3.z,  p3.y*q3.x,  p3.y*q3.y,  p3.y*q3.z,
    //      p3.z*q3.x, p3.z*q3.y, p3.z*q3.z,  0.0f,       0.0f,       0.0f,      -p3.x*q3.x, -p3.x*q3.y, -p3.x*q3.z,

    //      0.0f,      0.0f,      0.0f,      -p4.z*q4.x, -p4.z*q4.y, -p4.z*q4.z,  p4.y*q4.x,  p4.y*q4.y,  p4.y*q4.z,
    //      p4.z*q4.x, p4.z*q4.y, p4.z*q4.z,  0.0f,       0.0f,       0.0f,      -p4.x*q4.x, -p4.x*q4.y, -p4.x*q4.z;
    A << q1.x, q1.y, 1.0f, 0.0f, 0.0f, 0.0f, -p1.x*q1.x, -p1.x*q1.y, -p1.x,
         0.0f, 0.0f, 0.0f, q1.x, q1.y, 1.0f, -p1.y*q1.x, -p1.y*q1.y, -p1.y,

         q2.x, q2.y, 1.0f, 0.0f, 0.0f, 0.0f, -p2.x*q2.x, -p2.x*q2.y, -p2.x,
         0.0f, 0.0f, 0.0f, q2.x, q2.y, 1.0f, -p2.y*q2.x, -p2.y*q2.y, -p2.y,

         q3.x, q3.y, 1.0f, 0.0f, 0.0f, 0.0f, -p3.x*q3.x, -p3.x*q3.y, -p3.x,
         0.0f, 0.0f, 0.0f, q3.x, q3.y, 1.0f, -p3.y*q3.x, -p3.y*q3.y, -p3.y,

         q4.x, q4.y, 1.0f, 0.0f, 0.0f, 0.0f, -p4.x*q4.x, -p4.x*q4.y, -p4.x,
         0.0f, 0.0f, 0.0f, q4.x, q4.y, 1.0f, -p4.y*q4.x, -p4.y*q4.y, -p4.y;
    // solve SVD for V
    Eigen::JacobiSVD<Eigen::MatrixXf> svd(A, Eigen::ComputeFullV);
    auto& h = svd.matrixV().col(8);
    _poseH[0][0] = h[0];
    _poseH[1][0] = h[1];
    _poseH[2][0] = h[2];

    _poseH[0][1] = h[3];
    _poseH[1][1] = h[4];
    _poseH[2][1] = h[5];

    _poseH[0][2] = h[6];
    _poseH[1][2] = h[7];
    _poseH[2][2] = h[8];
    _poseH = cameraInvK * _poseH;
    // recover rotation matrix and translation vector
    float d = 1.0f / std::sqrt(glm::length(_poseH[0]) * glm::length(_poseH[1]));
    _poseM[3] = d * _poseH[2]; // set translation vector
    glm::vec3 h1 = d * _poseH[0];
    glm::vec3 h2 = d * _poseH[1];
    glm::vec3 h12 = glm::normalize(h1 + h2);
    glm::vec3 h21 = glm::normalize(glm::cross(h12, glm::cross(h1, h2)));
    d = 1.0f / std::sqrt(2.0f);
    _poseM[0] = d * (h12 + h21); // set R1
    _poseM[1] = d * (h12 - h21); // set R2
    _poseM[2] = glm::cross(_poseM[0], _poseM[1]); // set R3
    
    auto tmp = cameraK * _poseM * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    // auto tmp = cameraK * _poseH * glm::vec3(0.0f, 0.0f, 1.0f);
    std::cout << tmp.x << "," << tmp.y << "," << tmp.z << std::endl;
}

// reference: https://franklinta.com/2014/09/08/computing-css-matrix3d-transforms/
// void Marker::estimatePose(const glm::mat3& cameraK, const glm::mat3& cameraInvK)
// {
//     if(_marker_borderp1p2.x == 0.0f)
//     {
//         _poseH = glm::mat4(0.0f);
//         return;
//     }
//     // prepare p
//     glm::vec2 p1 = glm::vec2(_marker_borderp1p2.x, _marker_borderp1p2.y);
//     glm::vec2 p2 = glm::vec2(_marker_borderp1p2.z, _marker_borderp1p2.w);
//     glm::vec2 p3 = glm::vec2(_marker_borderp3p4.x, _marker_borderp3p4.y);
//     glm::vec2 p4 = glm::vec2(_marker_borderp3p4.z, _marker_borderp3p4.w);
//     // prepare q
//     static glm::vec2 q1 = glm::vec2(0.0f, 0.0f);
//     static glm::vec2 q2 = glm::vec2(0.0f, 1.0f);
//     static glm::vec2 q3 = glm::vec2(1.0f, 0.0f);
//     static glm::vec2 q4 = glm::vec2(1.0f, 1.0f);
//     // set up matrix A
//     Eigen::Matrix<float, 8, 8> A;
//     A << q1.x, q1.y, 1.0f, 0.0f, 0.0f, 0.0f, -p1.x*q1.x, -p1.x*q1.y,
//          0.0f, 0.0f, 0.0f, q1.x, q1.y, 1.0f, -p1.y*q1.x, -p1.y*q1.y,

//          q2.x, q2.y, 1.0f, 0.0f, 0.0f, 0.0f, -p2.x*q2.x, -p2.x*q2.y,
//          0.0f, 0.0f, 0.0f, q2.x, q2.y, 1.0f, -p2.y*q2.x, -p2.y*q2.y,

//          q3.x, q3.y, 1.0f, 0.0f, 0.0f, 0.0f, -p3.x*q3.x, -p3.x*q3.y,
//          0.0f, 0.0f, 0.0f, q3.x, q3.y, 1.0f, -p3.y*q3.x, -p3.y*q3.y,

//          q4.x, q4.y, 1.0f, 0.0f, 0.0f, 0.0f, -p4.x*q4.x, -p4.x*q4.y,
//          0.0f, 0.0f, 0.0f, q4.x, q4.y, 1.0f, -p4.y*q4.x, -p4.y*q4.y;

//     Eigen::Vector<float, 8> b;
//     b << p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y;

//     auto h = A.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b);
//     _poseH = glm::mat4(
//         glm::vec4(h[0], h[3], 0.0f, h[6]),
//         glm::vec4(h[1], h[4], 0.0f, h[7]),
//         glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
//         glm::vec4(h[2], h[5], 0.0f, h[8])
//     );
//     auto tmp = _poseH * glm::vec4(0.0f, 0.0f, 2.0f, 1.0f);
//     std::cout << tmp.x << "," << tmp.y << "," << tmp.z << std::endl;
// }