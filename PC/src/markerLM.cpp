// This file implements the Levenberg-Marquardt method
// for pose projection matrix refinement
#include "marker.hpp"
#include <Eigen/Dense>
#include <cmath>
#include <limits>
#include <iostream>

#define ERROR_EPS       0.001
#define MAX_ITER        30

/////////////////////////////////////////////////////////////////
// objective: min(sum(p - K M q)^2)
// where
// K: 3x3 matrix
// M: 3x4 matrix
// q: 4x1 vector (obj points)
// p: 3x1 vector (img points)
// variable: M
/////////////////////////////////////////////////////////////////
// let's say an update of delta on M
// brings us closer to local minimum
// S = sum(p - K M q - J delta)^2
// where
// J: gradient of function (K M q) with respect to M
// take derivative of S with respect to delta and set to zero:
// (J^T J) delta = J^T (p - K M q)
// which is replaced to: (by Levenberg)
// (J^T J + lambda I) delta = J^T (p - K M q)
// where
// lambda: a damping factor adjusted each iteration
// finally, to make solution scale invariant:
// (J^T J + lambda diag(J^T J)) delta = J^T (p - K M q)
/////////////////////////////////////////////////////////////////

// refer to: https://github.com/opencv/opencv/blob/master/modules/calib3d/src/compat_ptsetreg.cpp#L121
// refer to book "Augmented Reality: Principles and Practice"
// refer to: https://en.wikipedia.org/wiki/Levenberg%E2%80%93Marquardt_algorithm
// refine pose matrix
void Marker::refinePoseM(
    const glm::mat3& cameraInvK,
    const std::vector<glm::vec2>& objPoints,
    const std::vector<glm::vec2>& imgPoints
)
{
    if(objPoints.size() < 4 || imgPoints.size() < 4)
        return;
    // camera matrix in eigen
    Eigen::Matrix3f Kinv;
    Kinv << cameraInvK[0][0], cameraInvK[1][0], cameraInvK[2][0],
            cameraInvK[0][1], cameraInvK[1][1], cameraInvK[2][1],
            cameraInvK[0][2], cameraInvK[1][2], cameraInvK[2][2];
    // pose matrix
    Eigen::Matrix<float, 3, 4> M;
    M << _poseM[0][0], _poseM[1][0], _poseM[2][0], _poseM[3][0],
         _poseM[0][1], _poseM[1][1], _poseM[2][1], _poseM[3][1],
         _poseM[0][2], _poseM[1][2], _poseM[2][2], _poseM[3][2];
    Eigen::Matrix<float, 3, 4> Mshaped = Eigen::MatrixXf::Constant(3, 4, 0.0f);
    // point matrix
    Eigen::Matrix4f objMatrix;
    objMatrix << objPoints[0].x, objPoints[1].x, objPoints[2].x, objPoints[3].x,
                 objPoints[0].y, objPoints[1].y, objPoints[2].y, objPoints[3].y,
                 0.0f,           0.0f,           0.0f,           0.0f,
                 1.0f,           1.0f,           1.0f,           1.0f;
    Eigen::Matrix<float, 3, 4> imgMatrix;
    imgMatrix << imgPoints[0].x, imgPoints[1].x, imgPoints[2].x, imgPoints[3].x,
                 imgPoints[0].y, imgPoints[1].y, imgPoints[2].y, imgPoints[3].y,
                 0.0f,           0.0f,           0.0f,           0.0f;
    // here we are minimizing sum(K^-1 p - M q)^2
    // if K M q = p, then M q = K^-1 p
    imgMatrix = Kinv * imgMatrix;
    // Jacobian
    // mxn matrix
    // m: number of points
    // n: number of parameters
    Eigen::Matrix<float, 12, 12> J = Eigen::MatrixXf::Constant(12, 12, 0.0f);
    // initialize Jacobian
    for(int i = 0; i < 4; i++)
    {
        // J(i+0, Eigen::seqN(0, 4)) = objMatrix.col(i);
        // J(i+4, Eigen::seqN(4, 4)) = objMatrix.col(i);
        // J(i+8, Eigen::seqN(8, 4)) = objMatrix.col(i);
        J(i*3+0, Eigen::seqN(0, 4)) = objMatrix.col(i);
        J(i*3+1, Eigen::seqN(4, 4)) = objMatrix.col(i);
        J(i*3+2, Eigen::seqN(8, 4)) = objMatrix.col(i);
    }
    Eigen::Matrix<float, 12, 12> JT = J.transpose();
    Eigen::Matrix<float, 12, 12> JTJ = JT * J;
    Eigen::Matrix<float, 12, 12> JTJdiag = Eigen::MatrixXf::Constant(12, 12, 0.0f);
    JTJdiag.diagonal() += JTJ.diagonal();
    // delta matrix
    Eigen::Vector<float, 12> delta = Eigen::VectorXf::Constant(12, 0.0f);
    // left and right hand side of equation
    Eigen::Matrix<float, 12, 12> eqLeft;
    Eigen::Vector<float, 12> eqRight;
    // damping factor (same as OpenCV)
    // lambda = 1.0 + exp(lambdaLog10 * log10val)
    const float log10val = std::log(10.0f);
    int lambdaLog10 = -3;
    // start iteration
    int iter = 0;
    float err = std::numeric_limits<float>::max();
    float prevErr = std::numeric_limits<float>::max();
    while(iter++ < MAX_ITER)
    {
        float lambda = 1.0f + std::exp(lambdaLog10 * log10val);
        // (J^T J + lambda diag(J^T J)) delta = J^T (p - K M q)
        eqLeft = JTJ + lambda * JTJdiag;
        Mshaped = imgMatrix - M * objMatrix;
        Eigen::Map<Eigen::VectorXf> flatten(Mshaped.data(), Mshaped.size());
        eqRight = JT * flatten;
        delta = eqLeft.colPivHouseholderQr().solve(eqRight);
        // update M
        M.row(0) += delta(Eigen::seqN(0, 4));
        M.row(1) += delta(Eigen::seqN(4, 4));
        M.row(2) += delta(Eigen::seqN(8, 4));
        // compute error
        prevErr = err;
        err = 0.0f;
        Eigen::Matrix<float, 3, 4> residual = imgMatrix - M * objMatrix;
        Eigen::Map<Eigen::VectorXf> residualVec(residual.data(), residual.size());
        for(int i = 0; i < 12; i++)
            err += residualVec[i] * residualVec[i];
        // std::cout << iter << "|" << err << std::endl;
        if(err <= ERROR_EPS) break;
        if(err > prevErr)
        {
            lambdaLog10 = std::min(lambdaLog10+1, 16);
            err = std::numeric_limits<float>::max();
            continue;
        }
        // update lambda
        lambdaLog10 = std::max(lambdaLog10-1, -16);
    }
    _err_LM = err;
    // std::cout << "LM err: " << err << "(" << iter << ")" << std::endl;
    // record new M
    _poseMRefined = glm::mat4x3(
        glm::vec3(M.col(0)[0], M.col(0)[1], M.col(0)[2]),
        glm::vec3(M.col(1)[0], M.col(1)[1], M.col(1)[2]),
        glm::vec3(M.col(2)[0], M.col(2)[1], M.col(2)[2]),
        glm::vec3(M.col(3)[0], M.col(3)[1], M.col(3)[2])
    );
}