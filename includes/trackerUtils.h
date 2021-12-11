#ifndef TRACKER_UTILS_H
#define TRACKER_UTILS_H

#include "nvdstracker.h"
#include <opencv2/opencv.hpp>

struct Object {
    cv::Rect_<float> rect;
    int              label;
    float            prob;
};

struct NvObject {
    Object          object;
    NvMOTObjToTrack *associatedObjectIn;
};

#endif
