/**
 * This module was automatically generated by `ts-interface-builder`
 */
import * as t from "ts-interface-checker";
// tslint:disable:object-literal-key-quotes

export const IAudioFrameObserverBase = t.iface([], {
  "onRecordAudioFrame": t.opt(t.func("boolean", t.param("channelId", "string"), t.param("audioFrame", "AudioFrame"))),
  "onPlaybackAudioFrame": t.opt(t.func("boolean", t.param("channelId", "string"), t.param("audioFrame", "AudioFrame"))),
  "onMixedAudioFrame": t.opt(t.func("boolean", t.param("channelId", "string"), t.param("audioFrame", "AudioFrame"))),
  "onEarMonitoringAudioFrame": t.opt(t.func("boolean", t.param("audioFrame", "AudioFrame"))),
});

export const IAudioFrameObserver = t.iface(["IAudioFrameObserverBase"], {
  "onPlaybackAudioFrameBeforeMixing": t.opt(t.func("boolean", t.param("channelId", "string"), t.param("uid", "number"), t.param("audioFrame", "AudioFrame"))),
});

export const IAudioSpectrumObserver = t.iface([], {
  "onLocalAudioSpectrum": t.opt(t.func("boolean", t.param("data", "AudioSpectrumData"))),
  "onRemoteAudioSpectrum": t.opt(t.func("boolean", t.param("spectrums", t.array("UserAudioSpectrumInfo")), t.param("spectrumNumber", "number"))),
});

export const IVideoEncodedFrameObserver = t.iface([], {
  "onEncodedVideoFrameReceived": t.opt(t.func("boolean", t.param("uid", "number"), t.param("imageBuffer", "Uint8Array"), t.param("length", "number"), t.param("videoEncodedFrameInfo", "EncodedVideoFrameInfo"))),
});

export const IVideoFrameObserver = t.iface([], {
  "onCaptureVideoFrame": t.opt(t.func("boolean", t.param("type", "VideoSourceType"), t.param("videoFrame", "VideoFrame"))),
  "onPreEncodeVideoFrame": t.opt(t.func("boolean", t.param("type", "VideoSourceType"), t.param("videoFrame", "VideoFrame"))),
  "onMediaPlayerVideoFrame": t.opt(t.func("boolean", t.param("videoFrame", "VideoFrame"), t.param("mediaPlayerId", "number"))),
  "onRenderVideoFrame": t.opt(t.func("boolean", t.param("channelId", "string"), t.param("remoteUid", "number"), t.param("videoFrame", "VideoFrame"))),
  "onTranscodedVideoFrame": t.opt(t.func("boolean", t.param("videoFrame", "VideoFrame"))),
});

export const IMediaRecorderObserver = t.iface([], {
  "onRecorderStateChanged": t.opt(t.func("void", t.param("state", "RecorderState"), t.param("error", "RecorderErrorCode"))),
  "onRecorderInfoUpdated": t.opt(t.func("void", t.param("info", "RecorderInfo"))),
});

const exportedTypeSuite: t.ITypeSuite = {
  IAudioFrameObserverBase,
  IAudioFrameObserver,
  IAudioSpectrumObserver,
  IVideoEncodedFrameObserver,
  IVideoFrameObserver,
  IMediaRecorderObserver,
};
export default exportedTypeSuite;
