#ifndef FFMPEG4DROID_FFTOOLS_LOG_H
#define FFMPEG4DROID_FFTOOLS_LOG_H

#include <android/log.h>

#ifndef FFTOOLS_TAG
#define FFTOOLS_TAG "default"
#endif

// #define LOG_NDEBUG 0

#define LOGV(FORMAT, ...) __android_log_print(ANDROID_LOG_VERBOSE, FFTOOLS_TAG, FORMAT, ##__VA_ARGS__)
#define LOGD(FORMAT, ...) __android_log_print(ANDROID_LOG_DEBUG, FFTOOLS_TAG, FORMAT, ##__VA_ARGS__)
#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO, FFTOOLS_TAG, FORMAT, ##__VA_ARGS__)
#define LOGW(FORMAT, ...) __android_log_print(ANDROID_LOG_WARN, FFTOOLS_TAG, FORMAT, ##__VA_ARGS__)
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR, FFTOOLS_TAG, FORMAT, ##__VA_ARGS__)

#endif //FFMPEG4DROID_FFTOOLS_LOG_H
