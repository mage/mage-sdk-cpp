#include <iostream>
#include <mage.h>
#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include <list>

#define  LOG_TAG    "UnityCpp"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern "C" {
    void MAGE_free(void* ptr);

    mage::RPC* MAGE_RPC_Connect(const char* mageApplication,
                                const char* mageDomain,
                                const char* mageProtocol);
    void MAGE_RPC_Disconnect(mage::RPC* client);
    const char* MAGE_RPC_Call(mage::RPC* client,
                        const char* methodName,
                        const char* params,
                        int* code);
    void MAGE_RPC_SetSession(mage::RPC* client, const char* sessionKey);
    void MAGE_RPC_ClearSession(mage::RPC* client);
    int MAGE_RPC_PullEvents(mage::RPC* client, int transport);
    void MAGE_SendAllMessages();
}

void MAGE_free(void* ptr) {
    free(ptr);
}

// The Java VM used to make JNI calls
JavaVM* javavm = nullptr;

/*
 * Because we can access to Unity only from the main thread,
 * we have to build a list with the messages which can't be send directly when we are in another thread.
 */
struct Message {
    std::string object;
    std::string method;
    std::string parameter;

    Message(const char* _object, const char* _method, const char* _parameter)
            : object(std::string(_object))
            , method(std::string(_method))
            , parameter(std::string(_parameter))
    {}
};
std::list<Message*> messageList;
pthread_mutex_t messageListMutex = PTHREAD_MUTEX_INITIALIZER;

// Store the JavaVM in a global variable
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    javavm = vm;

    JNIEnv* jni_env = nullptr;
    if (vm->GetEnv((void**) &jni_env, JNI_VERSION_1_6) != JNI_OK)
        return -1;

    return JNI_VERSION_1_6;
}

/**
* Send a message to unity through UnitySendMessage
* @param    object      Name of the GameObject which will receive the message
* @param    method      Name of the method to call
* @param    parameter   Parameter to pass to the method
*/
void SendMessage(const char* object, const char* method, const char* parameter) {
    if (javavm == nullptr) {
        LOGI("JNI not initialized");
        return;
    }

    bool attached = false;
    JNIEnv* jni_env = nullptr;
    switch (javavm->GetEnv((void**) &jni_env, JNI_VERSION_1_6)) {
        case JNI_OK:
            break;
        case JNI_EDETACHED:
            if (javavm->AttachCurrentThread(&jni_env, nullptr) != 0) {
                LOGI("Could not attach current thread");
            }
            attached = true;
            break;
        case JNI_EVERSION:
            LOGI("JNI_EVERSION");
            break;
    }

    // Get the UnityPlayer class through JNI to be able to send a message to unity
    jclass UnityPlayer = jni_env->FindClass("com/unity3d/player/UnityPlayer");
    jthrowable exception = jni_env->ExceptionOccurred();
    if (exception) {
        LOGI("Exception: unable to find the UnityPlayer class");
        jni_env->ExceptionDescribe();
        jni_env->DeleteLocalRef(exception);
        jni_env->ExceptionClear();
        if (attached) {
            javavm->DetachCurrentThread();
        }

        /*
         * Here, we are probably not in the main thread,
         * that's why JNI is not able to find the UnityPlayer class.
         * In this case, we are storing the message in a list,
         * which should be handled from the main thread.
         */
        Message* msg = new Message(object, method, parameter);
        pthread_mutex_lock(&messageListMutex);
        messageList.push_back(msg);
        pthread_mutex_unlock(&messageListMutex);
        return;
    }

    // Get the UnitySendMessage static method
    jmethodID UnitySendMessage = jni_env->GetStaticMethodID(UnityPlayer,
                                                            "UnitySendMessage",
                                                            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    exception = jni_env->ExceptionOccurred();
    if (exception) {
        LOGI("Unable to initialize UnitySendMessage");
        jni_env->ExceptionDescribe();
        jni_env->DeleteLocalRef(exception);
        jni_env->ExceptionClear();
    }

    jstring objectStr = jni_env->NewStringUTF(object);
    jstring methodStr = jni_env->NewStringUTF(method);
    jstring parameterStr = jni_env->NewStringUTF(parameter);

    // Call UnityPlayer::UnitySendMessage
    jni_env->CallStaticVoidMethod(UnityPlayer, UnitySendMessage, objectStr, methodStr, parameterStr);

    exception = jni_env->ExceptionOccurred();
    if (exception) {
        LOGI("Unable to call UnitySendMessage for: %s - %s - %s", object, method, parameter);
        jni_env->ExceptionDescribe();
        jni_env->DeleteLocalRef(exception);
        jni_env->ExceptionClear();
    } else {
        LOGI("Called UnitySendMessage for: %s - %s - %s", object, method, parameter);
    }

    if (attached) {
        javavm->DetachCurrentThread();
    }
}

/**
 * Send all the messages in the messageList.
 * It should be called from the main thread.
 */
void MAGE_SendAllMessages() {
    pthread_mutex_lock(&messageListMutex);
    std::list<Message*>::const_iterator citr = messageList.cbegin();
    for (; citr != messageList.cend(); ++citr) {
        LOGI("Send message for: %s - %s - %s", (*citr)->object.c_str(), (*citr)->method.c_str(), (*citr)->parameter.c_str());
        SendMessage((*citr)->object.c_str(), (*citr)->method.c_str(), (*citr)->parameter.c_str());
        delete (*citr);
    }
    messageList.clear();
    pthread_mutex_unlock(&messageListMutex);
}

class UnityEventObserver : public mage::EventObserver {
    public:
        explicit UnityEventObserver(mage::RPC* client) : m_pClient(client) {}
        virtual void ReceiveEvent(const std::string& name,
                                  const Json::Value& data = Json::Value::null) const {
            Json::Value event;
            event["name"] = name;
            event["data"] = data;
            Json::FastWriter writer;
            std::string str = writer.write(event);
            SendMessage("Network", "ReceiveEvent", str.c_str());

            if (name == "session.set") {
                HandleSessionSet(data);
            }
        }

        void HandleSessionSet(const Json::Value& data) const {
            m_pClient->SetSession(data["key"].asString());
        }

    private:
        mage::RPC* m_pClient;
};

mage::RPC* MAGE_RPC_Connect(const char* mageApplication,
                            const char* mageDomain,
                            const char* mageProtocol) {
    mage::RPC* client = new mage::RPC(mageApplication, mageDomain, mageProtocol);

    UnityEventObserver *eventObserver = new UnityEventObserver(client);
    client->AddObserver(eventObserver);

    return client;
}

void MAGE_RPC_Disconnect(mage::RPC* client) {
    std::list<mage::EventObserver*> observers = client->GetObservers();
    std::list<mage::EventObserver*>::const_iterator citr;
    for (citr = observers.begin(); citr != observers.end(); ++citr) {
        delete (*citr);
    }

    delete client;
}

const char* MAGE_RPC_Call(mage::RPC* client,
                    const char* methodName,
                    const char* strParams,
                    int* code) {
    Json::Reader reader;
    Json::Value params;
    if (!reader.parse(strParams, params)) {
        *code = -4;
        return NULL;
    }
    std::string str;

    *code = 0;

    try {
        Json::Value result = client->Call(methodName, params);
        Json::FastWriter writer;
        str = writer.write(result);
    } catch (mage::MageRPCError e) {
        str = e.code() + " - " + e.what();
        *code = -3;
    } catch (mage::MageErrorMessage e) {
        str = e.code() + " - " + e.what();
        *code = -2;
    } catch (...) {
        *code = -1;
        return NULL;
    }

    char* out = (char*)malloc((str.length() + 1) * sizeof(char));
    strncpy(out, str.c_str(), str.length() + 1);
    return out;
}

void MAGE_RPC_SetSession(mage::RPC* client, const char* sessionKey) {
    client->SetSession(sessionKey);
}

void MAGE_RPC_ClearSession(mage::RPC* client) {
    client->ClearSession();
}

int MAGE_RPC_PullEvents(mage::RPC* client, int transport) {
    try {
        client->PullEvents((mage::Transport)transport);
    } catch (mage::MageClientError error) {
        return -2;
    } catch (...) {
        return -1;
    }

    return 0;
}

