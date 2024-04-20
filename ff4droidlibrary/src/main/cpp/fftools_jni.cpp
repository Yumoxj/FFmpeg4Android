#include <jni.h>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#include "libfftools/fftools.h"
#include "libavutil/log.h"

#ifdef __cplusplus
}
#endif

#define FFTOOLS_TAG "fftools"
#include "fftools_log.h"

static JNIEnv *ff_env;
static jclass ff_class;
static jmethodID ff_msg_methodID;

static void msg_callback(const char *msg, int level);

extern "C"
JNIEXPORT void JNICALL
Java_com_tlens_ff4droidlibrary_FFTools_init(JNIEnv *env, jclass) {
    ff_env = env;

    jclass clazz = ff_env->FindClass("com/tlens/ff4droidlibrary/FFTools");
    if (clazz == NULL) {
        LOGE("Cannot find class com/tlens/ff4droidlibrary/FFTools");
        return;
    }
    ff_class = (jclass)env->NewGlobalRef(clazz);

    ff_msg_methodID = ff_env->GetStaticMethodID(
            clazz, "onMsgCallback", "(Ljava/lang/String;I)V");
    if (ff_msg_methodID == NULL) {
        LOGE("Cannot find method onMsgCallback in class com/tlens/ff4droidlibrary/FFTools");
        ff_env->DeleteLocalRef(clazz);
        return;
    }

    fftools_init(NULL);
    fftools_set_msg_callback(msg_callback);

    JavaVM *vm;
    env->GetJavaVM(&vm);
    fftools_jni_set_java_vm(vm, NULL);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_tlens_ff4droidlibrary_FFTools_deinit(JNIEnv *env, jclass ) {
    if (ff_class != NULL) {
        env->DeleteGlobalRef(ff_class);
    }
    fftools_deinit();
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_tlens_ff4droidlibrary_FFTools_version(JNIEnv *env, jclass) {
    return env->NewStringUTF(fftools_ffmpeg_version());
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_tlens_ff4droidlibrary_FFTools_ffmpeg(JNIEnv *env, jclass, jobjectArray commands) {
    int argc = env->GetArrayLength(commands);
    if (argc < 2) {
        msg_callback("invalid command", AV_LOG_WARNING);
        return 0;
    }

    char **argv = (char **) malloc(argc * sizeof(char *));

    int result;
    for (int i = 0; i < argc; i++) {
        jstring jstr = (jstring) env->GetObjectArrayElement(commands, i);
        const char *temp = env->GetStringUTFChars(jstr, nullptr);

        int len = strlen(temp);
        argv[i] = (char *)(malloc( len + 1));
        memset(argv[i], 0, len + 1);
        strcpy(argv[i], temp);

        env->ReleaseStringUTFChars(jstr, temp);
    }

    result = fftools_ffmpeg(argc, argv);

    for (int i = 0; i < argc; i++) {
        free(argv[i]);
    }
    free(argv);
    return result;
}

static void msg_callback(const char *msg, int level) {
    switch (level)
    {
        case AV_LOG_FATAL:
        case AV_LOG_ERROR:
            LOGE("%s", msg);
            break;
        case AV_LOG_WARNING:
            LOGW("%s", msg);
            break;
        case AV_LOG_INFO:
            LOGI("%s", msg);
            break;
        case AV_LOG_DEBUG:
            LOGD("%s", msg);
            break;
        case AV_LOG_VERBOSE:
        case AV_LOG_TRACE:
            LOGV("%s", msg);
            break;
        default:
            LOGI("%s", msg);
            break;
    }

    jstring jmsg = ff_env->NewStringUTF(msg);
    ff_env->CallStaticVoidMethod(ff_class, ff_msg_methodID, jmsg, level);
    ff_env->DeleteLocalRef(jmsg);
}