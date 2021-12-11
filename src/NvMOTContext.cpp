#include "DsTracker.h"
#include "BYTETracker.h"
#include "trackerUtils.h"
#include <fstream>

NvMOTContext::NvMOTContext(const NvMOTConfig &configIn, NvMOTConfigResponse &configResponse) {
    byteTracker = std::make_shared<BYTETracker>(30, 30);
    configResponse.summaryStatus = NvMOTConfigStatus_OK;
}

NvMOTStatus NvMOTContext::processFrame(const NvMOTProcessParams *params,
                                       NvMOTTrackedObjBatch *pTrackedObjectsBatch) {
    NvMOTTrackedObjList   *trackedObjList = &pTrackedObjectsBatch->list[0];
    NvMOTFrame            *frame          = &params->frameList[0];
    std::vector<NvObject> nvObjects(frame->objectsIn.numFilled);
    int           persons    = 0;
    for (uint32_t numObjects = 0; numObjects < frame->objectsIn.numFilled; ++numObjects) {
        NvMOTObjToTrack *objectToTrack = &frame->objectsIn.list[numObjects];
        NvDsObjectMeta  *obj_meta      = (NvDsObjectMeta *) objectToTrack->pPreservedData;
        if (objectToTrack->classId != 0) 
            continue;
        ++persons;
        cv::Rect_<float> detectionResult(objectToTrack->bbox.x,
                                         objectToTrack->bbox.y,
                                         objectToTrack->bbox.width,
                                         objectToTrack->bbox.height);
        Object   object{detectionResult, objectToTrack->classId, objectToTrack->confidence};
        NvObject nvObject;
        nvObject.object             = object;
        nvObject.associatedObjectIn = objectToTrack;
        nvObjects.push_back(nvObject);
    }

    std::vector<STrack> outputTracks = byteTracker->update(nvObjects);

    NvMOTTrackedObj *trackedObjs = new NvMOTTrackedObj[512];
    int             filled       = 0;

    for (STrack &sTrack: outputTracks) {
        std::vector<float> tlwh        = sTrack.tlwh;
        NvMOTTrackedObj* trackedObj = new NvMOTTrackedObj;
        NvMOTRect          motRect{tlwh[0], tlwh[1], tlwh[2], tlwh[3]};
        trackedObj->classId                        = 0;
        trackedObj->trackingId                     = (uint64_t) sTrack.track_id;
        trackedObj->bbox                           = motRect;
        trackedObj->confidence                     = 1;
        trackedObj->age                            = (uint32_t) sTrack.tracklet_len;
        trackedObj->associatedObjectIn             = sTrack.associatedObjectIn;
        sTrack.associatedObjectIn->confidence      = 1;
        trackedObj->associatedObjectIn->doTracking = true;
        trackedObjs[filled++] = *trackedObj;
    }

    trackedObjList->streamID     = frame->streamID;
    trackedObjList->frameNum     = frame->frameNum;
    trackedObjList->valid        = true;
    trackedObjList->list         = trackedObjs;
    trackedObjList->numFilled    = filled;
    trackedObjList->numAllocated = 512;
}

NvMOTStatus NvMOTContext::processFramePast(const NvMOTProcessParams *params,
                                           NvDsPastFrameObjBatch *pPastFrameObjectsBatch) {
    std::cout << "Doing stuff with past frame" << std::endl;
    return NvMOTStatus_OK;
}

NvMOTStatus NvMOTContext::removeStream(const NvMOTStreamId streamIdMask) {
    std::cout << "Removing stuff" << std::endl;
    return NvMOTStatus_OK;
}
