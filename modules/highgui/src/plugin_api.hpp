// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://opencv.org/license/.

#ifndef UI_PLUGIN_API_HPP
#define UI_PLUGIN_API_HPP

#include <opencv2/core/cvdef.h>
#include <opencv2/core/llapi/llapi.h>

#include "backend.hpp"

#if !defined(BUILD_PLUGIN)

/// increased for backward-compatible changes, e.g. add new function
/// Caller API <= Plugin API -> plugin is fully compatible
/// Caller API > Plugin API -> plugin is not fully compatible, caller should use extra checks to use plugins with older API
#define API_VERSION 0 // preview

/// increased for incompatible changes, e.g. remove function argument
/// Caller ABI == Plugin ABI -> plugin is compatible
/// Caller ABI > Plugin ABI -> plugin is not compatible, caller should use shim code to use old ABI plugins (caller may know how lower ABI works, so it is possible)
/// Caller ABI < Plugin ABI -> plugin can't be used (plugin should provide interface with lower ABI to handle that)
#define ABI_VERSION 0 // preview

#else // !defined(BUILD_PLUGIN)

#if !defined(ABI_VERSION) || !defined(API_VERSION)
#error "Plugin must define ABI_VERSION and API_VERSION before including plugin_api.hpp"
#endif

#endif // !defined(BUILD_PLUGIN)

typedef cv::highgui_backend::UIBackend* CvPluginUIBackend;

struct OpenCV_UI_Plugin_API_v0_0_api_entries
{
    /** @brief Get backend API instance

    @param[out] handle pointer on backend API handle

    @note API-CALL 1, API-Version == 0
     */
    CvResult (CV_API_CALL *getInstance)(CV_OUT CvPluginUIBackend* handle) CV_NOEXCEPT;
}; // OpenCV_UI_Plugin_API_v0_0_api_entries

typedef struct OpenCV_UI_Plugin_API_v0
{
    OpenCV_API_Header api_header;
    struct OpenCV_UI_Plugin_API_v0_0_api_entries v0;
} OpenCV_UI_Plugin_API_v0;

#if ABI_VERSION == 0 && API_VERSION == 0
typedef OpenCV_UI_Plugin_API_v0 OpenCV_UI_Plugin_API;
#else
#error "Not supported configuration: check ABI_VERSION/API_VERSION"
#endif

#ifdef BUILD_PLUGIN
extern "C" {

CV_PLUGIN_EXPORTS
const OpenCV_UI_Plugin_API* CV_API_CALL opencv_ui_plugin_init_v0
        (int requested_abi_version, int requested_api_version, void* reserved /*NULL*/) CV_NOEXCEPT;

}  // extern "C"
#else  // BUILD_PLUGIN
typedef const OpenCV_UI_Plugin_API* (CV_API_CALL *FN_opencv_ui_plugin_init_t)
        (int requested_abi_version, int requested_api_version, void* reserved /*NULL*/);
#endif  // BUILD_PLUGIN

#endif // UI_PLUGIN_API_HPP
