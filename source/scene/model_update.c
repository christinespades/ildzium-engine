#include "pch.h"
#include "scene/model_update.h"

float last_spatial_stream_check = 0.0f;

void update_models(double lastTime) {
    // Throttled dynamic cell streamer execution
    if ((lastTime - last_spatial_stream_check) >= get_param_float(PARAM_RENDER_SPATIAL_STREAM_CHECK_INTERVAL)) {
        update_model_streaming_and_culling(camera.x, camera.y, camera.z, get_param_float(PARAM_RENDER_SPATIAL_STREAM_RADIUS));
        last_spatial_stream_check = lastTime;
    }
}