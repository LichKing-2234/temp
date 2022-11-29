/*
 * @Author: zhangtao@agora.io
 * @Date: 2021-04-22 20:53:37
 * @Last Modified by: zhangtao@agora.io
 * @Last Modified time: 2022-08-05 11:12:05
 */
#include "agora_electron_bridge.h"
#include <memory>
#include "iris_base.h"
#include "node_iris_event_handler.h"

namespace agora {
namespace rtc {
namespace electron {
using namespace iris::rtc;
using namespace agora::iris::rtc;
const char* AgoraElectronBridge::_class_name = "AgoraElectronBridge";
const char* AgoraElectronBridge::_ret_code_str = "callApiReturnCode";
const char* AgoraElectronBridge::_ret_result_str = "callApiResult";
napi_ref* AgoraElectronBridge::_ref_construcotr_ptr = nullptr;

AgoraElectronBridge::AgoraElectronBridge() {
  LOG_F(INFO, "AgoraElectronBridge::AgoraElectronBridge()");
  memset(_result, '\0', kBasicResultLength);
}

AgoraElectronBridge::~AgoraElectronBridge() {
  LOG_F(INFO, "AgoraElectronBridge::~AgoraElectronBridge");
  Release();
}

napi_value AgoraElectronBridge::Init(napi_env env, napi_value exports) {
  LOG_F(INFO, "AgoraElectronBridge::Init()");
  napi_property_descriptor properties[] = {
      DECLARE_NAPI_METHOD("CallApi", CallApi),
      DECLARE_NAPI_METHOD("OnEvent", OnEvent),
      DECLARE_NAPI_METHOD("GetBuffer", GetBuffer),
      DECLARE_NAPI_METHOD("EnableVideoFrameCache", EnableVideoFrameCache),
      DECLARE_NAPI_METHOD("DisableVideoFrameCache", DisableVideoFrameCache),
      DECLARE_NAPI_METHOD("GetVideoFrame", GetVideoFrame),
      DECLARE_NAPI_METHOD("SetAddonLogFile", SetAddonLogFile),
      DECLARE_NAPI_METHOD("InitializeEnv", InitializeEnv),
      DECLARE_NAPI_METHOD("ReleaseEnv", ReleaseEnv)};

  napi_value cons;

  // method count !!!
  NAPI_CALL(env, napi_define_class(env, _class_name, NAPI_AUTO_LENGTH, New,
                                   nullptr, 9, properties, &cons));

  AgoraElectronBridge::_ref_construcotr_ptr = new napi_ref();
  NAPI_CALL(env, napi_create_reference(
                     env, cons, 1, AgoraElectronBridge::_ref_construcotr_ptr));

  NAPI_CALL(env, napi_set_named_property(env, exports, _class_name, cons));
  return exports;
}

napi_value AgoraElectronBridge::New(napi_env env, napi_callback_info info) {
  napi_value target;
  NAPI_CALL(env, napi_get_new_target(env, info, &target));
  bool is_constructor = target != nullptr;

  if (is_constructor) {
    LOG_F(INFO, "New");
    napi_value jsthis;
    NAPI_CALL(env,
              napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr));

    auto irisEngine = new AgoraElectronBridge();
    irisEngine->_env = env;
    NAPI_CALL(env, napi_wrap(env, jsthis, reinterpret_cast<void *>(irisEngine),
                             AgoraElectronBridge::Destructor, nullptr,
                             &irisEngine->_ref));

    return jsthis;
  } else {
    napi_value instance;
    NAPI_CALL(env,
              napi_new_instance(env, Constructor(env), 0, nullptr, &instance));

    return instance;
  }
}

napi_value AgoraElectronBridge::Constructor(napi_env env) {
  napi_value cons;

  NAPI_CALL(env, napi_get_reference_value(
                     env, *AgoraElectronBridge::_ref_construcotr_ptr, &cons));

  return cons;
}

void AgoraElectronBridge::Destructor(napi_env env, void *nativeObject,
                                     void *finalize_hint) {
  reinterpret_cast<AgoraElectronBridge *>(nativeObject)->~AgoraElectronBridge();
  LOG_F(INFO, "AgoraElectronBridge::Destructor()");
}

napi_value AgoraElectronBridge::CallApi(napi_env env, napi_callback_info info) {
  size_t argc = 4;
  napi_value args[4];
  napi_value jsthis;
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr));

  AgoraElectronBridge *agoraElectronBridge = nullptr;
  NAPI_CALL(env, napi_unwrap(env, jsthis, (void **)(&agoraElectronBridge)));

  std::string funcName = "";
  std::string parameter = "";
  uint32_t bufferCount = 0;

  NAPI_CALL(env, napi_get_value_utf8string(env, args[0], funcName));
  NAPI_CALL(env, napi_get_value_utf8string(env, args[1], parameter));
  NAPI_CALL(env, napi_get_value_uint32(env, args[3], &bufferCount));

  if (strcmp(parameter.c_str(), "") == 0) {
    parameter = "{}";
  }

  memset(agoraElectronBridge->_result, '\0', kBasicResultLength);

  int ret = ERROR_PARAMETER_1;
  std::shared_ptr<IrisApiEngine> irisApiEngine =
      agoraElectronBridge->_iris_api_engine;

  if (irisApiEngine) {
    try {
      if (bufferCount > 0) {
        std::vector<void *> data;
        data.resize(bufferCount);
        bool result = false;
        NAPI_CALL(env, napi_is_array(env, args[2], &result));
        assert(result == true);

        std::vector<napi_value> itemVec;
        itemVec.reserve(bufferCount);
        for (int i = 0; i < bufferCount; i++) {
          NAPI_CALL(env, napi_get_element(env, args[2], i, &itemVec[i]));
          NAPI_CALL(env,
                    napi_get_typedarray_info(env, itemVec[i], nullptr, nullptr,
                                             &data[i], nullptr, nullptr));
        }

        ret = irisApiEngine->CallIrisApi(
            funcName.c_str(), parameter.c_str(), parameter.length(),
            data.data(), bufferCount, agoraElectronBridge->_result);

      } else {
        bool registerApi = funcName.find("_register") != std::string::npos;
        bool unRegisterApi = funcName.find("_unregister") != std::string::npos;

        if (registerApi) {
          auto observer = irisApiEngine->CreateObserver(
              funcName.c_str(),
              agoraElectronBridge->_iris_observer_event_handler.get(),
              parameter.c_str(), parameter.length());

          void *ptrList[1];
          ptrList[0] = observer;
          ret = irisApiEngine->CallIrisApi(funcName.c_str(), parameter.c_str(),
                                           parameter.length(), ptrList, 1,
                                           agoraElectronBridge->_result);
        } else if (unRegisterApi) {
          ret = irisApiEngine->CallIrisApi(funcName.c_str(), parameter.c_str(),
                                           parameter.length(), nullptr, 0,
                                           agoraElectronBridge->_result);
          auto observer = irisApiEngine->GetObserver(funcName.c_str());
          irisApiEngine->DestroyObserver(funcName.c_str(), observer);
        } else {
          ret = irisApiEngine->CallIrisApi(funcName.c_str(), parameter.c_str(),
                                           parameter.length(), nullptr, 0,
                                           agoraElectronBridge->_result);
        }
      }
    } catch (std::exception &e) {
      agoraElectronBridge->OnApiError(e.what());
      LOG_F(INFO, "CallApi(func name:%s) parameter: catch excepton msg: %s",
            funcName.c_str(), e.what());
    }
  } else {
    LOG_F(INFO, "CallApi(func name:%s) fail, not init engine",
          funcName.c_str());
    ret = ERROR_NOT_INIT;
  }

  RETURE_NAPI_OBJ();
}

napi_value AgoraElectronBridge::GetBuffer(napi_env env,
                                          napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2];
  napi_value jsthis;
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr));

  long long bufferPtr;
  int bufferSize;

  NAPI_CALL(env, napi_get_value_int64(env, args[0], &bufferPtr));

  NAPI_CALL(env, napi_get_value_int32(env, args[1], &bufferSize));

  napi_value value;
  NAPI_CALL(env,
            napi_create_buffer_copy(env, bufferSize, (const void *)bufferPtr,
                                    nullptr, &value));

  return value;
}

napi_value AgoraElectronBridge::OnEvent(napi_env env, napi_callback_info info) {
  size_t argc = 3;
  napi_value args[3];
  napi_value jsthis;
  int ret = ERROR_PARAMETER_1;
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr));

  AgoraElectronBridge *agoraElectronBridge;
  NAPI_CALL(env, napi_unwrap(env, jsthis,
                             reinterpret_cast<void **>(&agoraElectronBridge)));

  uint32_t callBackModule = CallBackModule::RTC;
  NAPI_CALL(env, napi_get_value_uint32(env, args[0], &callBackModule));

  std::string eventName = "";
  NAPI_CALL(env, napi_get_value_utf8string(env, args[1], eventName));

  napi_value cb = args[2];

  LOG_F(INFO, "AgoraElectronBridge::OnEvent(%s)", eventName.c_str());

  napi_value global;
  NAPI_CALL(env, napi_get_global(env, &global));

  if (agoraElectronBridge->_iris_api_engine) {
    if (callBackModule == CallBackModule::RTC)
      agoraElectronBridge->_iris_rtc_event_handler->addEvent(eventName, env, cb,
                                                             global);
    else if (callBackModule == CallBackModule::MPK)
      agoraElectronBridge->_iris_mpk_event_handler->addEvent(eventName, env, cb,
                                                             global);
    else if (callBackModule == CallBackModule::OBSERVER)
      agoraElectronBridge->_iris_observer_event_handler->addEvent(
          eventName, env, cb, global);

    ret = ERROR_OK;
  } else {
    ret = ERROR_NOT_INIT;
    LOG_F(INFO, "AgoraElectronBridge::OnEvent error Not Init Engine");
  }

  RETURE_NAPI_OBJ();
}

napi_value AgoraElectronBridge::SetAddonLogFile(napi_env env,
                                                napi_callback_info info) {
  napi_value jsthis;
  size_t argc = 2;
  napi_value args[2];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr));

  AgoraElectronBridge *agoraElectronBridge;
  NAPI_CALL(env, napi_unwrap(env, jsthis,
                             reinterpret_cast<void **>(&agoraElectronBridge)));

  std::string file_path = "";
  NAPI_CALL(env, napi_get_value_utf8string(env, args[0], file_path));

  char result[kBasicStringLength];
  int ret = ERROR_PARAMETER_1;
  memset(result, '\0', kBasicStringLength);

  ret = startLogService(file_path.c_str());

  RETURE_NAPI_OBJ();
}

void AgoraElectronBridge::OnApiError(const char *errorMessage) {
  _iris_rtc_event_handler->OnEvent("onApiError", errorMessage, nullptr, nullptr,
                                   0);
}

napi_value AgoraElectronBridge::EnableVideoFrameCache(napi_env env,
                                                      napi_callback_info info) {
  napi_value jsthis;
  size_t argc = 1;
  napi_value args[1];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr));

  AgoraElectronBridge *agoraElectronBridge;
  NAPI_CALL(env, napi_unwrap(env, jsthis,
                             reinterpret_cast<void **>(&agoraElectronBridge)));

  IrisVideoFrameBufferConfig config;
  napi_value obj = args[0];

  int videoSourceType;
  std::string channelId = "";
  unsigned int width = 0;
  unsigned int height = 0;

  NAPI_CALL(env, napi_obj_get_property(env, obj, "uid", config.id));
  NAPI_CALL(
      env, napi_obj_get_property(env, obj, "videoSourceType", videoSourceType));
  config.type = (IrisVideoSourceType)videoSourceType;
  NAPI_CALL(env, napi_obj_get_property(env, obj, "channelId", channelId));
  strcpy(config.key, channelId.c_str());
  NAPI_CALL(env, napi_obj_get_property(env, obj, "width", width));
  NAPI_CALL(env, napi_obj_get_property(env, obj, "height", height));

  char result[kBasicStringLength];
  memset(result, '\0', kBasicStringLength);
  int ret = ERROR_PARAMETER_1;

  if (!agoraElectronBridge->_iris_video_frame_buffer_manager) {
    ret = ERROR_NOT_INIT;
    LOG_F(INFO, "IrisVideoFrameBufferManager Not Init");
  } else {
    try {
      iris::IrisVideoFrameBuffer buffer(
          IrisVideoFrameType::kVideoFrameTypeYUV420);
      agoraElectronBridge->_iris_video_frame_buffer_manager
          ->EnableVideoFrameBuffer(buffer, &config);
      ret = ERROR_OK;
    } catch (std::exception &e) {
      LOG_F(INFO, "EnableVideoFrameCache catch exception %s", e.what());
      agoraElectronBridge->OnApiError(e.what());
    }
  }

  RETURE_NAPI_OBJ();
}

napi_value
AgoraElectronBridge::DisableVideoFrameCache(napi_env env,
                                            napi_callback_info info) {
  napi_value jsthis;
  size_t argc = 1;
  napi_value args[1];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr));

  AgoraElectronBridge *agoraElectronBridge;
  NAPI_CALL(env, napi_unwrap(env, jsthis,
                             reinterpret_cast<void **>(&agoraElectronBridge)));

  napi_value obj = args[0];
  IrisVideoFrameBufferConfig config;

  int videoSourceType;
  std::string channelId = "";

  NAPI_CALL(env, napi_obj_get_property(env, obj, "uid", config.id));
  NAPI_CALL(
      env, napi_obj_get_property(env, obj, "videoSourceType", videoSourceType));
  config.type = (IrisVideoSourceType)videoSourceType;
  NAPI_CALL(env, napi_obj_get_property(env, obj, "channelId", channelId));
  strcpy(config.key, channelId.c_str());

  char result[kBasicStringLength];
  memset(result, '\0', kBasicStringLength);
  int ret = ERROR_PARAMETER_1;

  if (!agoraElectronBridge->_iris_video_frame_buffer_manager) {
    ret = ERROR_NOT_INIT;
    LOG_F(INFO, "IrisVideoFrameBufferManager Not Init");
  } else {
    try {
      agoraElectronBridge->_iris_video_frame_buffer_manager
          ->DisableVideoFrameBuffer(&config);
      ret = ERROR_OK;
    } catch (std::exception &e) {
      LOG_F(INFO, "DisableVideoFrameCache catch exception %s", e.what());
      agoraElectronBridge->OnApiError(e.what());
    }
  }

  RETURE_NAPI_OBJ();
}

napi_value AgoraElectronBridge::GetVideoFrame(napi_env env,
                                              napi_callback_info info) {
  napi_value jsthis;
  size_t argc = 1;
  napi_value args[1];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr));

  AgoraElectronBridge *agoraElectronBridge;
  NAPI_CALL(env, napi_unwrap(env, jsthis,
                             reinterpret_cast<void **>(&agoraElectronBridge)));

  IrisVideoFrameBufferConfig config;

  napi_value obj = args[0];
  int videoSourceType;
  std::string channel_id;
  napi_value y_buffer_obj;
  void *y_buffer;
  size_t y_length;
  napi_value u_buffer_obj;
  void *u_buffer;
  size_t u_length;
  napi_value v_buffer_obj;
  void *v_buffer;
  size_t v_length;
  int height;
  int width;

  NAPI_CALL(env, napi_obj_get_property(env, obj, "uid", config.id));
  NAPI_CALL(
      env, napi_obj_get_property(env, obj, "videoSourceType", videoSourceType));
  config.type = (IrisVideoSourceType)videoSourceType;
  NAPI_CALL(env, napi_obj_get_property(env, obj, "channelId", channel_id));
  strcpy(config.key, channel_id.c_str());

  NAPI_CALL(env, napi_obj_get_property(env, obj, "yBuffer", y_buffer_obj));
  NAPI_CALL(env, napi_get_buffer_info(env, y_buffer_obj, &y_buffer, &y_length));

  NAPI_CALL(env, napi_obj_get_property(env, obj, "uBuffer", u_buffer_obj));
  NAPI_CALL(env, napi_get_buffer_info(env, u_buffer_obj, &u_buffer, &u_length));

  NAPI_CALL(env, napi_obj_get_property(env, obj, "vBuffer", v_buffer_obj));
  NAPI_CALL(env, napi_get_buffer_info(env, v_buffer_obj, &v_buffer, &v_length));

  NAPI_CALL(env, napi_obj_get_property(env, obj, "height", height));
  NAPI_CALL(env, napi_obj_get_property(env, obj, "width", width));

  IrisVideoFrame videoFrame = IrisVideoFrame_default;
  videoFrame.y_buffer = y_buffer;
  videoFrame.u_buffer = u_buffer;
  videoFrame.v_buffer = v_buffer;
  videoFrame.height = height;
  videoFrame.width = width;

  bool isFresh = false;
  napi_value retObj;
  int32_t ret = ERROR_NOT_INIT;
  NAPI_CALL(env, napi_create_object(env, &retObj));

  if (!agoraElectronBridge->_iris_video_frame_buffer_manager.get()) {
    NAPI_CALL(env, napi_obj_set_property(env, retObj, "ret", ret));
    LOG_F(INFO, "IrisVideoFrameBufferManager Not Init");
    return retObj;
  }

  ret = agoraElectronBridge->_iris_video_frame_buffer_manager->GetVideoFrame(
      videoFrame, isFresh, &config);

  unsigned int rotation = 0;
  NAPI_CALL(env, napi_obj_set_property(env, retObj, "ret", ret));
  NAPI_CALL(env, napi_obj_set_property(env, retObj, "isNewFrame", isFresh));
  NAPI_CALL(env, napi_obj_set_property(env, retObj, "width", videoFrame.width));
  NAPI_CALL(env,
            napi_obj_set_property(env, retObj, "height", videoFrame.height));
  NAPI_CALL(env,
            napi_obj_set_property(env, retObj, "yStride", videoFrame.y_stride));
  NAPI_CALL(env, napi_obj_set_property(env, retObj, "rotation", rotation));
  NAPI_CALL(env, napi_obj_set_property(env, retObj, "timestamp",
                                       videoFrame.render_time_ms));
  return retObj;
}

napi_value AgoraElectronBridge::InitializeEnv(napi_env env,
                                              napi_callback_info info) {
  napi_value jsthis;
  size_t argc = 2;
  napi_value args[2];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr));

  AgoraElectronBridge *agoraElectronBridge;
  NAPI_CALL(env, napi_unwrap(env, jsthis,
                             reinterpret_cast<void **>(&agoraElectronBridge)));

  // create
  auto engine = std::make_shared<IrisApiEngine>();
  auto bufferManager = std::make_shared<iris::IrisVideoFrameBufferManager>();
  auto rtcEventHandler = std::make_shared<NodeIrisEventHandler>();
  auto mpkEventHandler = std::make_shared<NodeIrisEventHandler>();
  auto observerEventHandler = std::make_shared<NodeIrisEventHandler>();
  ::enableUseJsonArray(true);

  // combine
  engine->SetIrisRtcEngineEventHandler(rtcEventHandler.get());
  engine->SetIrisMediaPlayerEventHandler(mpkEventHandler.get());
  engine->SetIrisMediaRecorderEventHandler(rtcEventHandler.get());
  engine->Attach(bufferManager.get());

  // assign
  agoraElectronBridge->_iris_api_engine = engine;
  agoraElectronBridge->_iris_video_frame_buffer_manager = bufferManager;
  agoraElectronBridge->_iris_rtc_event_handler = rtcEventHandler;
  agoraElectronBridge->_iris_mpk_event_handler = mpkEventHandler;
  agoraElectronBridge->_iris_observer_event_handler = observerEventHandler;

  LOG_F(INFO, "AgoraElectronBridge::InitializeEnv");
  napi_value retValue = nullptr;
  return retValue;
}

napi_value AgoraElectronBridge::ReleaseEnv(napi_env env,
                                           napi_callback_info info) {
  napi_value jsthis;
  size_t argc = 2;
  napi_value args[2];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr));

  AgoraElectronBridge *agoraElectronBridge;
  NAPI_CALL(env, napi_unwrap(env, jsthis,
                             reinterpret_cast<void **>(&agoraElectronBridge)));

  agoraElectronBridge->Release();
  LOG_F(INFO, "AgoraElectronBridge::ReleaseEnv");
  napi_value retValue = nullptr;
  return retValue;
}

void AgoraElectronBridge::Release() {
  LOG_F(INFO, "AgoraElectronBridge::Release()");

  if (_iris_api_engine) {
    LOG_F(INFO, "AgoraElectronBridge::Release() reset rtcEngine");
    // uncontrol
    _iris_api_engine->Detach(_iris_video_frame_buffer_manager.get());
    _iris_api_engine->UnsetIrisRtcEngineEventHandler(
        _iris_rtc_event_handler.get());
    _iris_api_engine->UnsetIrisMediaPlayerEventHandler(
        _iris_mpk_event_handler.get());
    _iris_api_engine->UnsetIrisMediaRecorderEventHandler(
        _iris_rtc_event_handler.get());
    // reset
    _iris_rtc_event_handler.reset();
    _iris_mpk_event_handler.reset();
    _iris_observer_event_handler.reset();
    _iris_video_frame_buffer_manager.reset();
    _iris_api_engine.reset();
    LOG_F(INFO, "AgoraElectronBridge::Release");
  }
}
} // namespace electron
} // namespace rtc
} // namespace agora
