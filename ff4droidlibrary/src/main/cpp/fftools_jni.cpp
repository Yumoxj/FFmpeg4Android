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
#include "pthread.h"

static JavaVM *ff_vm;
static jclass ff_class;
static jmethodID ff_msg_methodID;

static pthread_key_t current_env;
static pthread_once_t once = PTHREAD_ONCE_INIT;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static void jni_detach_env(void *)
{
    if (ff_vm) {
        ff_vm->DetachCurrentThread();
    }
}

static void jni_create_pthread_key(void)
{
    pthread_key_create(&current_env, jni_detach_env);
}

static JNIEnv *get_current_env()
{
    int ret;
    JNIEnv *env = NULL;

    pthread_mutex_lock(&lock);

    pthread_once(&once, jni_create_pthread_key);

    if ((env = static_cast<JNIEnv *>(pthread_getspecific(current_env))) != NULL) {
        goto done;
    }

    ret = ff_vm->GetEnv((void **)&env, JNI_VERSION_1_6);
    switch(ret) {
        case JNI_EDETACHED:
            if (ff_vm->AttachCurrentThread(&env, NULL) != 0) {
                LOGE("Failed to attach the JNI environment to the current thread");
                env = NULL;
            } else {
                pthread_setspecific(current_env, env);
            }
            break;
        case JNI_OK:
            break;
        case JNI_EVERSION:
            LOGE("The specified JNI version is not supported");
            break;
        default:
            LOGE("Failed to get the JNI environment attached to this thread");
            break;
    }

done:
    pthread_mutex_unlock(&lock);
    return env;
}

static void msg_callback(const char *msg, int level);

static void post_msg(const char *msg, int level, bool is_help_msg) {
    JNIEnv *env = get_current_env();

    if (env && ff_class && (level <= AV_LOG_INFO || is_help_msg)) {
        jstring jmsg = env->NewStringUTF(msg);
        if (jmsg) {
            env->CallStaticVoidMethod(ff_class, ff_msg_methodID, jmsg, level);
            env->DeleteLocalRef(jmsg);
        } else {
            LOGE("failed to post the msg '%s'", msg);
        }
    }
}

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *) {
    JNIEnv *env;
    if (vm->GetEnv((void **)&env, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    ff_vm = vm;
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_tlens_ff4droidlibrary_FFTools_init(JNIEnv *env, jclass) {
    env = env;

    jclass clazz = env->FindClass("com/tlens/ff4droidlibrary/FFTools");
    if (clazz == NULL) {
        LOGE("Cannot find class com/tlens/ff4droidlibrary/FFTools");
        return;
    }
    ff_class = (jclass)env->NewGlobalRef(clazz);

    ff_msg_methodID = env->GetStaticMethodID(
            clazz, "onMsgCallback", "(Ljava/lang/String;I)V");
    if (ff_msg_methodID == NULL) {
        LOGE("Cannot find method onMsgCallback in class com/tlens/ff4droidlibrary/FFTools");
        env->DeleteLocalRef(clazz);
        return;
    }

    fftools_init(NULL);
    fftools_set_msg_callback(msg_callback);
    fftools_jni_set_java_vm(ff_vm, NULL);

    env->DeleteLocalRef(clazz);
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

    post_msg("😈 ffmpeg begin ...", AV_LOG_INFO, true);
    result = fftools_ffmpeg(argc, argv);
    char result_msg[128];
    sprintf(result_msg,"😊 ffmpeg end, result=%d", result);
    post_msg(result_msg, AV_LOG_INFO, true);

    for (int i = 0; i < argc; i++) {
        free(argv[i]);
    }
    free(argv);
    return result;
}

static bool starts_with(const char *str, const char *prefix) {
    size_t prefix_len = strlen(prefix);
    size_t str_len = strlen(str);

    if (prefix_len > str_len) {
        return false;
    }

    return strncmp(str, prefix, prefix_len) == 0;
}

static void msg_callback(const char *msg, int level) {
    if (msg == NULL)
        return;

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

    bool help_msg = false;
    if (starts_with(msg, "show_help")) {
        help_msg = true;
        msg = msg + strlen("show_help");
    }

    post_msg(msg, level, help_msg);
}