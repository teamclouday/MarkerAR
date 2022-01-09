#include "marker.hpp"
// This file implements the Levenberg-Marquardt method
// for pose projection matrix refinement

void Marker::refinePoseM(
    const glm::mat3& cameraK,
    const glm::mat4x3& prevM, const glm::mat4x3& currM,
    const std::vector<glm::vec2>& objPoints,
    const std::vector<glm::vec2>& imgPoints
)
{

}