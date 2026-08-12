/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "DynamicsContourMapper.h"

namespace xLights::AI {

AudioDynamicsContourResult DynamicsContourMapper::AnalyzeDynamicsContour(
    const std::vector<float>& leftChannel,
    const std::vector<float>& rightChannel,
    size_t sampleRate,
    long framePeriodMS,
    std::function<void(int pct)> progress)
{
    AudioDynamicsMapper mapper;
    return mapper.AnalyzeDynamicsContour(leftChannel, rightChannel, sampleRate, framePeriodMS, progress);
}

AudioDynamicsContourResult DynamicsContourMapper::AnalyzeDynamicsContour(
    AudioManager* audioManager,
    long framePeriodMS,
    std::function<void(int pct)> progress)
{
    AudioDynamicsMapper mapper;
    return mapper.AnalyzeDynamicsContour(audioManager, framePeriodMS, progress);
}

std::string DynamicsContourMapper::ExportAsValueCurveString(const AudioDynamicsContourResult& contour) {
    return AudioDynamicsMapper::ExportAsValueCurveString(contour);
}

std::string DynamicsContourMapper::ExportAsValueCurveJson(const AudioDynamicsContourResult& contour) {
    return AudioDynamicsMapper::ExportAsValueCurveJson(contour);
}

} // namespace xLights::AI
