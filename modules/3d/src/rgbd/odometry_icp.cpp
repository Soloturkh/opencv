// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html

#include "odometry_icp.hpp"
#include "odometry_functions.hpp"

namespace cv
{

OdometryICP::OdometryICP(OdometrySettings _settings, OdometryAlgoType _algtype)
{
	this->settings = _settings;
    this->algtype = _algtype;
}

OdometryICP::~OdometryICP()
{
}

OdometryFrame OdometryICP::createOdometryFrame()
{
#ifdef HAVE_OPENCL
    return OdometryFrame(OdometryFrameStoreType::UMAT);
#endif
    return OdometryFrame(OdometryFrameStoreType::MAT);
}

void OdometryICP::prepareFrame(OdometryFrame frame)
{
	prepareICPFrame(frame, frame, this->settings, this->algtype);
}

void OdometryICP::prepareFrames(OdometryFrame srcFrame, OdometryFrame dstFrame)
{
	prepareICPFrame(srcFrame, dstFrame, this->settings, this->algtype);
}

bool OdometryICP::compute(OdometryFrame srcFrame, OdometryFrame dstFrame, OutputArray Rt) const
{
	Matx33f cameraMatrix;
	settings.getCameraMatrix(cameraMatrix);
	std::vector<int> iterCounts;
	Mat miterCounts;
	settings.getIterCounts(miterCounts);
	for (int i = 0; i < miterCounts.size().height; i++)
		iterCounts.push_back(miterCounts.at<int>(i));
	bool isCorrect = RGBDICPOdometryImpl(Rt, Mat(), srcFrame, dstFrame, cameraMatrix,
		this->settings.getMaxDepthDiff(), this->settings.getAngleThreshold(),
		iterCounts, this->settings.getMaxTranslation(),
		this->settings.getMaxRotation(), settings.getSobelScale(),
		OdometryType::DEPTH, OdometryTransformType::RIGID_TRANSFORMATION, this->algtype);
    return isCorrect;
}

}
