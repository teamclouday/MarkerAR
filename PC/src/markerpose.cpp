#include "marker.hpp"
#include <Eigen/SVD>
#include <Eigen/Dense>
#include <glm/gtc/matrix_transform.hpp>
#include <opencv2/opencv.hpp>
#include <cmath>
#include <limits>
#include <iostream>

// compute reprojection error
float reprojectionError(
    const glm::mat3& cameraK, const glm::mat4x3& M,
    const std::vector<glm::vec2>& objPoints,
    const std::vector<glm::vec2>& imgPoints
)
{
    float error = 0.0f;
    for(size_t i = 0; i < 4; i++)
    {
        auto projected = cameraK * M * glm::vec4(objPoints[i], 0.0f, 1.0f);
        float dx = projected.x - imgPoints[i].x;
        float dy = projected.y - imgPoints[i].y;
        error += dx * dx + dy * dy;
    }
    return std::sqrt(error * 0.125f);
}

// validate solution based on Zhang's method
bool validateSolutionZhang(
    const Eigen::Vector3f& tstar, const Eigen::Vector3f& nstar, const Eigen::MatrixXf& H,
    const Eigen::MatrixXf& mstarT, Eigen::Matrix3f& outR, Eigen::Vector3f& outT
)
{
    // prepare rotation matrix and translation vector
    // R = H (I + t* n^T)^-1
    outR = H * (Eigen::Matrix3f::Identity() + tstar * nstar.transpose()).inverse();
    if(outR.determinant() < 0.0f)
    {
        // flip surface
        outR *= -1.0f;
    }
    outT = outR * tstar;
    // 1 + n^T R^T t
    float positive = 1.0f + (nstar.transpose() * outR.transpose() * outT)[0];
    if(positive <= 0.0f)
        return false;
    // m*^T n*
    Eigen::Vector4f positives = mstarT * nstar;
    if(positives[0] <= 0.0f || positives[1] <= 0.0f ||
       positives[2] <= 0.0f || positives[3] <= 0.0f)
       return false;
    return true;
}

// refer to https://hal.inria.fr/inria-00174036v3/document
// refer to https://github.com/opencv/opencv/blob/4.x/modules/calib3d/src/homography_decomp.cpp#L217
// decompose homography matrix
void decomposeHomoMatrixZhang(
    const glm::mat3& cameraK, const glm::mat3& cameraInvK, const glm::mat3x4& mstarTransposed,
    const glm::mat3& G, const std::vector<glm::vec2>& objPoints,
    const std::vector<glm::vec2>& imgPoints, glm::mat4x3& outputM)
{
    // normalize H
    // glm::mat3 hHat = cameraInvK * G * cameraK;
    glm::mat3 hHat = cameraInvK * G;
    // retrieve homography matrix H
    Eigen::Matrix<float, 3, 3> H;
    H << hHat[0][0], hHat[1][0], hHat[2][0],
         hHat[0][1], hHat[1][1], hHat[2][1],
         hHat[0][2], hHat[1][2], hHat[2][2];
    Eigen::EigenSolver<Eigen::MatrixXf> eigenSolver;
    eigenSolver.compute(H, false);
    float gamma = eigenSolver.eigenvalues().real()[1];
    H = H / gamma; // scale by lambda2
    // H^TH = V A V^T and solve for eigenvalues and eigenvectors
    // H = H.transpose() * H;
    eigenSolver.compute(H, true);
    Eigen::VectorXf lambdas = eigenSolver.eigenvalues().real();
    Eigen::MatrixXf vs = eigenSolver.eigenvectors().real();
    // verify the eigenvectors
    if (vs.rows() < 3 || vs.cols() < 3) return;
    float lambda1 = lambdas[0];
    float lambda3 = lambdas[2];
    // float lambda1 = lambdas[0] >= lambdas[2] ? lambdas[0] : lambdas[2];
    // float lambda3 = lambdas[0] >= lambdas[2] ? lambdas[2] : lambdas[0];
    float lambda1x3 = lambda1 * lambda3;
    float lambda1m3 = lambda1 - lambda3;
    float lambda1m3_2 = lambda1m3 * lambda1m3;
    // prepare zeta
    float tmp1 = 1.0f / (2.0f * lambda1x3);
    float tmp2 = std::sqrt(std::abs(1.0f + 4.0f * lambda1x3 / lambda1m3_2));
    float tmp1x2 = tmp1 * tmp2;
    float zeta1 = -tmp1 + tmp1x2;
    float zeta3 = -tmp1 - tmp1x2;
    float zeta1_2 = zeta1 * zeta1;
    float zeta3_2 = zeta3 * zeta3;
    float zeta1m3 = zeta1 - zeta3;
    float zeta1m3_inv = 1.0f / zeta1m3;
    // compute norms of v'1,3
    float nv1p = std::sqrt(std::abs(zeta1_2 * lambda1m3_2 + 2.0f * zeta1 * (lambda1x3 - 1.0f) + 1.0f));
    float nv3p = std::sqrt(std::abs(zeta3_2 * lambda1m3_2 + 2.0f * zeta3 * (lambda1x3 - 1.0f) + 1.0f));
    // compute v1' and v3'
    Eigen::Vector3f v1p, v3p;
    v1p << vs.col(0)[0]*nv1p, vs.col(0)[1]*nv1p, vs.col(0)[2]*nv1p;
    v3p << vs.col(2)[0]*nv3p, vs.col(2)[1]*nv3p, vs.col(2)[2]*nv3p;
    // 8 solutions in total:
    Eigen::Vector3f tstar[4], nstar[4];
    // solution 1
    tstar[0] =  (v1p - v3p) * zeta1m3_inv;
    tstar[1] = -(v1p - v3p) * zeta1m3_inv;
    nstar[0] =  (zeta1 * v3p - zeta3 * v1p) * zeta1m3_inv;
    nstar[1] = -(zeta1 * v3p - zeta3 * v1p) * zeta1m3_inv;
    // solution 2
    tstar[2] =  (v1p + v3p) * zeta1m3_inv;
    tstar[3] = -(v1p + v3p) * zeta1m3_inv;
    nstar[2] =  (zeta1 * v3p + zeta3 * v1p) * zeta1m3_inv;
    nstar[3] = -(zeta1 * v3p + zeta3 * v1p) * zeta1m3_inv;
    // prepare mstar transposed (for validation)
    Eigen::Matrix<float, 4, 3> mstarT;
    mstarT << mstarTransposed[0][0], mstarTransposed[1][0], mstarTransposed[2][0],
              mstarTransposed[0][1], mstarTransposed[1][1], mstarTransposed[2][1],
              mstarTransposed[0][2], mstarTransposed[1][2], mstarTransposed[2][2],
              mstarTransposed[0][3], mstarTransposed[1][3], mstarTransposed[2][3];
    // validate solutions, collect the first one
    Eigen::Matrix3f R; // rotation
    Eigen::Vector3f T; // translation
    std::vector<glm::mat4x3> candidates;
    for(int i = 0; i < 2; i++)
    {
        for(int j = 0; j < 2; j++)
        {
            if(validateSolutionZhang(tstar[i], nstar[j], H, mstarT, R, T))
            {
                candidates.push_back(glm::mat4x3(
                    glm::vec3(R.col(0)[0], R.col(0)[1], R.col(0)[2]),
                    glm::vec3(R.col(1)[0], R.col(1)[1], R.col(1)[2]),
                    glm::vec3(R.col(2)[0], R.col(2)[1], R.col(2)[2]),
                    glm::vec3(T[0], T[1], T[2])
                ));
            }
            if(validateSolutionZhang(tstar[i+2], nstar[j+2], H, mstarT, R, T))
            {
                candidates.push_back(glm::mat4x3(
                    glm::vec3(R.col(0)[0], R.col(0)[1], R.col(0)[2]),
                    glm::vec3(R.col(1)[0], R.col(1)[1], R.col(1)[2]),
                    glm::vec3(R.col(2)[0], R.col(2)[1], R.col(2)[2]),
                    glm::vec3(T[0], T[1], T[2])
                ));
            }
        }
    }

    if(candidates.size() == 0)
    {
        outputM = glm::mat4x3(0.0f);
        return;
    }
    // compute reprojection error for each
    float minError = std::numeric_limits<float>::max();
    size_t selectedIdx = 0;
    for(size_t i = 0; i < candidates.size(); i++)
    {
        float error = reprojectionError(cameraK, candidates[i], objPoints, imgPoints);
        if(error < minError)
        {
            minError = error;
            selectedIdx = i;
        }
    }
    // std::cout << "Reprojection Error: " << minError << std::endl;
    // set pose matrix
    outputM = candidates[selectedIdx];
}

// refer to: https://github.com/opencv/opencv/blob/master/modules/calib3d/src/undistort.dispatch.cpp#L384
// undistort image points
void undistortPoints(
    const glm::mat3& cameraK, const glm::vec3& cameraDistK, const glm::vec2& cameraDistP,
    glm::vec2& p1, glm::vec2& p2, glm::vec2& p3, glm::vec2& p4)
{
    glm::vec2 src[4] = {p1, p2, p3, p4};
    glm::vec2 dst[4];
    float fx = cameraK[0][0], fy = cameraK[1][1];
    float invfx = 1.0f / fx, invfy = 1.0f / fy;
    float cx = cameraK[2][0], cy = cameraK[2][1];
    // prepare coefficient K
    // k1,k2,p1,p2,k3
    float k[5] = {cameraDistK.x, cameraDistK.y, cameraDistP.x, cameraDistP.y, cameraDistK.z};
    for(int i = 0; i < 4; i++)
    {
        float x = src[i].x, y = src[i].y;
        float x0, y0, u, v;
        u = x;
        v = y;
        x = x0 = (x - cx) * invfx;
        y = y0 = (y - cy) * invfy;
        // init error
        float error = std::numeric_limits<float>::max();
        // iteratively reduce distortion error
        const int MAX_ITER = 10;
        const float EPS = 0.001f;
        int j = 0;
        for(; j < MAX_ITER; j++)
        {
            if(error < EPS) break;
            float r2 = x*x + y*y;
            float icdist = 1.0f / (
                1.0f + ((k[4] * r2 + k[1]) * r2 + k[0]) * r2
            );
            // if distortion is negative, reset
            if(icdist < 0.0f)
            {
                x = (u - cx) * invfx;
                y = (v - cy) * invfy;
                break;
            }
            float deltaX = 2.0f * k[2] * x * y + k[3] * (r2 + 2.0f * x * x);
            float deltaY = k[2] * (r2 + 2 * y * y) + 2.0f * k[3] * x * y;
            x = (x0 - deltaX) * icdist;
            y = (y0 - deltaY) * icdist;
            // compute error
            {
                float r4, r6;
                float a1, a2, a3;
                float cdist;
                float xd, yd;
                r2 = x * x + y * y;
                r4 = r2 * r2;
                r6 = r4 * r2;
                a1 = 2.0f * x * y;
                a2 = r2 + 2.0f * x * x;
                a3 = r2 + 2.0f * y * y;
                cdist = 1.0f + k[0] * r2 + k[1] * r4 + k[4] * r6;
                xd = x * cdist + k[2] * a1 + k[3] * a2;
                yd = y * cdist + k[2] * a3 + k[3] * a1;
                float xproj, yproj;
                xproj = xd * fx + cx;
                yproj = yd * fy + cy;
                error = std::sqrt((xproj - u) * (xproj - u) + (yproj - v) * (yproj - v));
            }
        }
        // std::cout << j << "," << error << std::endl;
        dst[i].x = x * fx + cx;
        dst[i].y = y * fy + cy;
    }
    // retrieve new positions
    p1 = dst[0];
    p2 = dst[1];
    p3 = dst[2];
    p4 = dst[3];
}

// refer to book: "Augmented Reality: Principles and Practice"
// decompose homography matrix
void decomposeHomoMatrixARBook(
    const glm::mat3& cameraK, const glm::mat3& cameraInvK, glm::mat3 H, glm::mat4x3& outputM
)
{
    H = cameraInvK * H;
    // recover rotation matrix and translation vector
    float d = 1.0f / std::sqrt(glm::length(H[0]) * glm::length(H[1]));
    // set translation
    outputM[3] = d * H[2];
    glm::vec3 h1 = H[0];
    glm::vec3 h2 = H[1];
    glm::vec3 h12 = glm::normalize(h1 + h2);
    glm::vec3 h21 = glm::normalize(glm::cross(h12, glm::cross(h1, h2)));
    // set rotations
    d = 1.0f / std::sqrt(2.0f);
    outputM[0] = (h12 + h21) * d; // set R1
    outputM[1] = (h12 - h21) * d; // set R2
    outputM[2] = glm::cross(outputM[0], outputM[1]); // set R3
}

// refer to https://stackoverflow.com/questions/8927771/computing-camera-pose-with-homography-matrix-based-on-4-coplanar-points
// decompose homography matrix
void decomposeHomoMatrixInternet(
    const glm::mat3& cameraK, const glm::mat3& cameraInvK, glm::mat3 H, glm::mat4x3& outputM
)
{
    H = cameraInvK * H;
    float tnorm = (glm::length(H[0]) + glm::length(H[1])) * 0.5f;
    // set translation
    outputM[3] = H[2] / tnorm;
    // set rotations
    outputM[0] = glm::normalize(H[0]);
    outputM[1] = glm::normalize(H[1]);
    outputM[2] = glm::cross(outputM[0], outputM[1]);
}

// refer to https://courses.cs.duke.edu//spring22/compsci527/notes/n_10_reconstruction.pdf
// decompose homography matrix
void decomposeHomoMatrixDuke(
    const glm::mat3& cameraK, const glm::mat3& cameraInvK,
    const std::vector<glm::vec2>& objPoints, const std::vector<glm::vec2>& imgPoints,
    glm::mat3 H, glm::mat4x3& outputM
)
{
    H = cameraInvK * H;
    // prepare E
    Eigen::Matrix<float, 3, 3> E;
    E << H[0][0], H[1][0], H[2][0],
         H[0][1], H[1][1], H[2][1],
         H[0][2], H[1][2], H[2][2];
    // decompose E
    Eigen::JacobiSVD<Eigen::MatrixXf> svdSolver;
    svdSolver.compute(E, Eigen::ComputeFullV | Eigen::ComputeFullU);
    // estimate two possible t
    auto& U = svdSolver.matrixU();
    auto& V = svdSolver.matrixV();
    Eigen::Vector3f T1 =  V.col(2);
    Eigen::Vector3f T2 = -V.col(2);
    // prepare alpha, beta
    Eigen::Matrix3f ab1 = U.col(0) * V.col(0).transpose();
    Eigen::Matrix3f ab2 = U.col(1) * V.col(1).transpose();
    Eigen::Matrix3f ab3 = U.col(2) * V.col(2).transpose();
    // compute Q
    Eigen::Matrix3f Q1 = ab1 + ab2 + ab3;
    Eigen::Matrix3f Q2 = ab1 + ab2 - ab3;
    // compute R
    Eigen::Matrix3f R1 = Q1 * Q1.determinant();
    Eigen::Matrix3f R2 = Q2 * Q2.determinant();
    std::vector<glm::mat4x3> candidates;
    candidates.push_back(glm::mat4x3(
        glm::vec3(R1.col(0)[0], R1.col(0)[1], R1.col(0)[2]),
        glm::vec3(R1.col(1)[0], R1.col(1)[1], R1.col(1)[2]),
        glm::vec3(R1.col(2)[0], R1.col(2)[1], R1.col(2)[2]),
        glm::vec3(T1[0], T1[1], T1[2])
    ));
    candidates.push_back(glm::mat4x3(
        glm::vec3(R1.col(0)[0], R1.col(0)[1], R1.col(0)[2]),
        glm::vec3(R1.col(1)[0], R1.col(1)[1], R1.col(1)[2]),
        glm::vec3(R1.col(2)[0], R1.col(2)[1], R1.col(2)[2]),
        glm::vec3(T2[0], T2[1], T2[2])
    ));
    candidates.push_back(glm::mat4x3(
        glm::vec3(R2.col(0)[0], R2.col(0)[1], R2.col(0)[2]),
        glm::vec3(R2.col(1)[0], R2.col(1)[1], R2.col(1)[2]),
        glm::vec3(R2.col(2)[0], R2.col(2)[1], R2.col(2)[2]),
        glm::vec3(T1[0], T1[1], T1[2])
    ));
    candidates.push_back(glm::mat4x3(
        glm::vec3(R2.col(0)[0], R2.col(0)[1], R2.col(0)[2]),
        glm::vec3(R2.col(1)[0], R2.col(1)[1], R2.col(1)[2]),
        glm::vec3(R2.col(2)[0], R2.col(2)[1], R2.col(2)[2]),
        glm::vec3(T2[0], T2[1], T2[2])
    ));
    float minError = std::numeric_limits<float>::max();
    size_t selectedIdx = 0;
    for(size_t i = 0; i < candidates.size(); i++)
    {
        float error = reprojectionError(cameraK, candidates[i], objPoints, imgPoints);
        if(error < minError)
        {
            minError = error;
            selectedIdx = i;
        }
    }
    // set pose matrix
    outputM = candidates[selectedIdx];
}

// refer to https://stanford.edu/class/ee267/notes/ee267_notes_tracking.pdf
// decompose homography matrix
void decomposeHomoMatrixStanford(
    const glm::mat3& cameraK, const glm::mat3& cameraInvK, glm::mat3 H, glm::mat4x3& outputM
)
{
    H = cameraInvK * H;
    float normCol0 = glm::length(H[0]);
    float normCol1 = glm::length(H[1]);
    float s = 2.0f / (normCol0 + normCol1);
    // set translation
    outputM[3] = s * H[2];
    // set rotations
    outputM[0] = H[0] / normCol0;
    outputM[1] = H[1] - outputM[0] * glm::dot(outputM[0], H[1]);
    outputM[1] = glm::normalize(outputM[1]);
    outputM[2] = glm::cross(outputM[0], outputM[1]);
}

// scale the estimated projection matrix
void scalePoseM(const glm::mat3& cameraK, glm::mat4x3& M, const glm::vec2& q, const glm::vec2& p)
{
    auto tmp = cameraK * M * glm::vec4(q, 0.0f, 1.0f);
    float scale = (p.x / tmp.x + p.y / tmp.y) * 0.5f;
    M *= scale;
}

// estimate pose from homography (SVD method)
void Marker::estimatePoseSVD(
    const glm::mat3& cameraK, const glm::mat3& cameraInvK,
    const glm::vec3& cameraDistK, const glm::vec2& cameraDistP
)
{
    if(_marker_borderp1p2.x <= 0.0f)
    {
        _poseM = glm::mat4x3(0.0f);
        _poseMRefined = glm::mat4x3(0.0f);
        return;
    }
    // prepare p
    glm::vec2 p1 = glm::vec2(_marker_borderp1p2.x, _marker_borderp1p2.y);
    glm::vec2 p2 = glm::vec2(_marker_borderp1p2.z, _marker_borderp1p2.w);
    glm::vec2 p4 = glm::vec2(_marker_borderp3p4.x, _marker_borderp3p4.y);
    glm::vec2 p3 = glm::vec2(_marker_borderp3p4.z, _marker_borderp3p4.w);
    // undistort points
    undistortPoints(
        cameraK, cameraDistK, cameraDistP,
        p1, p2, p3, p4
    );
    // glm::mat3x4 mstarT = glm::transpose(
    //     cameraInvK * glm::mat4x3(
    //         glm::vec3(p1, 1.0f),
    //         glm::vec3(p2, 1.0f),
    //         glm::vec3(p3, 1.0f),
    //         glm::vec3(p4, 1.0f)
    //     )
    // );
    // prepare q
    // const glm::vec2 q1 = glm::vec2(0.0f, 0.0f);
    // const glm::vec2 q2 = glm::vec2(0.0f, 1.0f);
    // const glm::vec2 q4 = glm::vec2(1.0f, 0.0f);
    // const glm::vec2 q3 = glm::vec2(1.0f, 1.0f);
    const glm::vec2 q1 = glm::vec2(-1.0f, -1.0f);
    const glm::vec2 q2 = glm::vec2(-1.0f,  1.0f);
    const glm::vec2 q4 = glm::vec2( 1.0f, -1.0f);
    const glm::vec2 q3 = glm::vec2( 1.0f,  1.0f);
    // set up matrix A
    Eigen::Matrix<float, 8, 9> A;
    A << q1.x, q1.y, 1.0f, 0.0f, 0.0f, 0.0f, -p1.x*q1.x, -p1.x*q1.y, -p1.x,
         0.0f, 0.0f, 0.0f, q1.x, q1.y, 1.0f, -p1.y*q1.x, -p1.y*q1.y, -p1.y,

         q2.x, q2.y, 1.0f, 0.0f, 0.0f, 0.0f, -p2.x*q2.x, -p2.x*q2.y, -p2.x,
         0.0f, 0.0f, 0.0f, q2.x, q2.y, 1.0f, -p2.y*q2.x, -p2.y*q2.y, -p2.y,

         q3.x, q3.y, 1.0f, 0.0f, 0.0f, 0.0f, -p3.x*q3.x, -p3.x*q3.y, -p3.x,
         0.0f, 0.0f, 0.0f, q3.x, q3.y, 1.0f, -p3.y*q3.x, -p3.y*q3.y, -p3.y,

         q4.x, q4.y, 1.0f, 0.0f, 0.0f, 0.0f, -p4.x*q4.x, -p4.x*q4.y, -p4.x,
         0.0f, 0.0f, 0.0f, q4.x, q4.y, 1.0f, -p4.y*q4.x, -p4.y*q4.y, -p4.y;
    // solve SVD for A
    Eigen::JacobiSVD<Eigen::MatrixXf> svdSolver(A, Eigen::ComputeFullV);
    auto& matrixV = svdSolver.matrixV();
    auto& h = matrixV.col(matrixV.cols() - 1);
    glm::mat3 H = glm::mat3(
        glm::vec3(h[0], h[3], h[6]),
        glm::vec3(h[1], h[4], h[7]),
        glm::vec3(h[2], h[5], h[8])
    );

    std::vector<glm::vec2> objPoints = {q1, q2, q3, q4};
    std::vector<glm::vec2> imgPoints = {p1, p2, p3, p4};
    // decomposeHomoMatrixZhang(cameraK, cameraInvK, mstarT, H, objPoints, imgPoints, _poseM);
    decomposeHomoMatrixARBook(cameraK, cameraInvK, H, _poseM);
    // decomposeHomoMatrixInternet(cameraK, cameraInvK, H, _poseM);
    scalePoseM(cameraK, _poseM, q1, p1);
    if(_poseMRefined[2][2] != 0.0f)
        _poseM = (_poseMRefined * 0.6f + _poseM * 0.4f);
    refinePoseM(cameraInvK, objPoints, imgPoints);
    // std::cout << "Reproj Err " << reprojectionError(cameraK, _poseMRefined, objPoints, imgPoints) << std::endl;
}

// reference: https://franklinta.com/2014/09/08/computing-css-matrix3d-transforms/
// estimate pose from homography (linear equation method)
void Marker::estimatePoseLinear(
    const glm::mat3& cameraK, const glm::mat3& cameraInvK,
    const glm::vec3& cameraDistK, const glm::vec2& cameraDistP
)
{
    if(_marker_borderp1p2.x <= 0.0f)
    {
        _poseM = glm::mat4x3(0.0f);
        _poseMRefined = glm::mat4x3(0.0f);
        return;
    }
    // prepare p
    glm::vec2 p1 = glm::vec2(_marker_borderp1p2.x, _marker_borderp1p2.y);
    glm::vec2 p2 = glm::vec2(_marker_borderp1p2.z, _marker_borderp1p2.w);
    glm::vec2 p3 = glm::vec2(_marker_borderp3p4.x, _marker_borderp3p4.y);
    glm::vec2 p4 = glm::vec2(_marker_borderp3p4.z, _marker_borderp3p4.w);
    // undistort points
    undistortPoints(
        cameraK, cameraDistK, cameraDistP,
        p1, p2, p3, p4
    );
    // prepare q
    static glm::vec2 q1 = glm::vec2(-1.0f, -1.0f);
    static glm::vec2 q2 = glm::vec2(-1.0f,  1.0f);
    static glm::vec2 q3 = glm::vec2( 1.0f, -1.0f);
    static glm::vec2 q4 = glm::vec2( 1.0f,  1.0f);
    // set up matrix A
    Eigen::Matrix<float, 8, 8> A;
    A << q1.x, q1.y, 1.0f, 0.0f, 0.0f, 0.0f, -p1.x*q1.x, -p1.x*q1.y,
         0.0f, 0.0f, 0.0f, q1.x, q1.y, 1.0f, -p1.y*q1.x, -p1.y*q1.y,

         q2.x, q2.y, 1.0f, 0.0f, 0.0f, 0.0f, -p2.x*q2.x, -p2.x*q2.y,
         0.0f, 0.0f, 0.0f, q2.x, q2.y, 1.0f, -p2.y*q2.x, -p2.y*q2.y,

         q3.x, q3.y, 1.0f, 0.0f, 0.0f, 0.0f, -p3.x*q3.x, -p3.x*q3.y,
         0.0f, 0.0f, 0.0f, q3.x, q3.y, 1.0f, -p3.y*q3.x, -p3.y*q3.y,

         q4.x, q4.y, 1.0f, 0.0f, 0.0f, 0.0f, -p4.x*q4.x, -p4.x*q4.y,
         0.0f, 0.0f, 0.0f, q4.x, q4.y, 1.0f, -p4.y*q4.x, -p4.y*q4.y;

    Eigen::Vector<float, 8> b;
    b << p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y;
    auto h = A.colPivHouseholderQr().solve(b);
    glm::mat3 H = glm::mat3(
        glm::vec3(h[0], h[3], h[6]),
        glm::vec3(h[1], h[4], h[7]),
        glm::vec3(h[2], h[5], 1.0f)
    );

    std::vector<glm::vec2> objPoints = {q1, q2, q3, q4};
    std::vector<glm::vec2> imgPoints = {p1, p2, p3, p4};
    // decomposeHomoMatrixStanford(cameraK, cameraInvK, H, _poseM);
    decomposeHomoMatrixARBook(cameraK, cameraInvK, H, _poseM);
    // decomposeHomoMatrixInternet(cameraK, cameraInvK, H, _poseM);
    // decomposeHomoMatrixDuke(cameraK, cameraInvK, objPoints, imgPoints, H, _poseM);
    scalePoseM(cameraK, _poseM, q1, p1);
    if(_poseMRefined[2][2] != 0.0f)
        _poseM = (_poseMRefined * 0.6f + _poseM * 0.4f);
    refinePoseM(cameraInvK, objPoints, imgPoints);
    // std::cout << "Reproj Err " << reprojectionError(cameraK, _poseMRefined, objPoints, imgPoints) << std::endl;
}

// use opencv as reference
// estimate pose (OpenCV method)
void Marker::estimatePoseOpenCV(
    const glm::mat3& cameraK, const glm::mat3& cameraInvK,
    const glm::vec3& cameraDistK, const glm::vec2& cameraDistP
)
{
    if(_marker_borderp1p2.x <= 0.0f)
    {
        _poseM = glm::mat4x3(0.0f);
        _poseMRefined = glm::mat4x3(0.0f);
        return;
    }
    std::vector<cv::Point2d> imgPoints = {
        cv::Point2d(_marker_borderp1p2.x, _marker_borderp1p2.y),
        cv::Point2d(_marker_borderp1p2.z, _marker_borderp1p2.w),
        cv::Point2d(_marker_borderp3p4.x, _marker_borderp3p4.y),
        cv::Point2d(_marker_borderp3p4.z, _marker_borderp3p4.w),
    };
    std::vector<cv::Point3d> objPoints = {
        cv::Point3d(-1.0, -1.0, 0.0),
        cv::Point3d(-1.0,  1.0, 0.0),
        cv::Point3d( 1.0, -1.0, 0.0),
        cv::Point3d( 1.0,  1.0, 0.0),
    };
    // std::vector<cv::Point3d> objPoints = {
    cv::Mat3d cameraMat = (
        cv::Mat_<double>(3,3) << cameraK[0][0], cameraK[1][0], cameraK[2][0],
            cameraK[0][1], cameraK[1][1], cameraK[2][1],
            cameraK[0][2], cameraK[1][2], cameraK[2][2]
    );
    std::vector<double> cameraDist = {
        cameraDistK.x, cameraDistK.y, cameraDistP.x, cameraDistP.y, cameraDistK.z
    };
    cv::Mat rvec, tvec;
    if(!cv::solvePnP(objPoints, imgPoints, cameraMat, cameraDist, rvec, tvec, false, cv::SOLVEPNP_IPPE))
    {
        _poseM = glm::mat4x3(0.0f);
        return;
    }
    cv::Mat rotMat;
    cv::Rodrigues(rvec, rotMat);
    rotMat.convertTo(rotMat, CV_32F);
    tvec.convertTo(tvec, CV_32F);
    _poseM = glm::mat4x3(
        glm::vec3(rotMat.at<float>(0,0), rotMat.at<float>(1,0), rotMat.at<float>(2,0)),
        glm::vec3(rotMat.at<float>(0,1), rotMat.at<float>(1,1), rotMat.at<float>(2,1)),
        glm::vec3(rotMat.at<float>(0,2), rotMat.at<float>(1,2), rotMat.at<float>(2,2)),
        glm::vec3(tvec.at<float>(0,0), tvec.at<float>(0,1), tvec.at<float>(0,2))
    );
    scalePoseM(cameraK, _poseM, glm::vec2(-1.0f, -1.0f), glm::vec2(_marker_borderp1p2.x, _marker_borderp1p2.y));
    // refinePoseM(cameraK, cameraInvK, objPoints, imgPoints);
    // std::cout << "Reproj Err " << reprojectionError(cameraK, _poseMRefined, objPoints, imgPoints) << std::endl;
}