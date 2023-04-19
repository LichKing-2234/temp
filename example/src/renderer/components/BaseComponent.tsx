import {
  ErrorCodeType,
  IRtcEngine,
  IRtcEngineEventHandler,
  RtcConnection,
  RtcStats,
  UserOfflineReasonType,
  VideoCanvas,
  VideoSourceType,
} from 'agora-electron-sdk';
import React, { Component, ReactElement, ReactNode } from 'react';

import RtcSurfaceView from './RtcSurfaceView';
import {
  AgoraButton,
  AgoraCard,
  AgoraDivider,
  AgoraList,
  AgoraStyle,
  AgoraText,
  AgoraTextInput,
  AgoraView,
} from './ui';

export interface BaseComponentState {
  appId: string;
  enableVideo: boolean;
  channelId?: string;
  token?: string;
  uid?: number;
  joinChannelSuccess?: boolean;
  remoteUsers?: number[];
  startPreview?: boolean;
}

export interface BaseAudioComponentState extends BaseComponentState {
  channelId: string;
  token: string;
  uid: number;
  joinChannelSuccess: boolean;
  remoteUsers: number[];
}

export interface BaseVideoComponentState extends BaseAudioComponentState {
  startPreview: boolean;
}

export abstract class BaseComponent<
    P = {},
    S extends BaseComponentState = BaseComponentState
  >
  extends Component<P, S>
  implements IRtcEngineEventHandler
{
  protected engine?: IRtcEngine;

  constructor(props: P) {
    super(props);
    this.state = this.createState();
  }

  componentDidMount() {
    this.initRtcEngine();
    window.agoraRtcEngine = this.engine;
  }

  componentWillUnmount() {
    this.releaseRtcEngine();
  }

  protected abstract createState(): S;

  protected abstract initRtcEngine(): void;

  protected joinChannel() {}

  protected leaveChannel() {}

  protected abstract releaseRtcEngine(): void;

  onError(err: ErrorCodeType, msg: string) {
    this.info('onError', 'err', err, 'msg', msg);
  }

  onJoinChannelSuccess(connection: RtcConnection, elapsed: number) {
    this.info(
      'onJoinChannelSuccess',
      'connection',
      connection,
      'elapsed',
      elapsed
    );
    this.setState({ joinChannelSuccess: true });
  }

  onLeaveChannel(connection: RtcConnection, stats: RtcStats) {
    this.info('onLeaveChannel', 'connection', connection, 'stats', stats);
    this.setState(this.createState());
  }

  onUserJoined(connection: RtcConnection, remoteUid: number, elapsed: number) {
    this.info(
      'onUserJoined',
      'connection',
      connection,
      'remoteUid',
      remoteUid,
      'elapsed',
      elapsed
    );
    const { remoteUsers } = this.state;
    if (remoteUsers === undefined) return;
    this.setState({
      remoteUsers: [...remoteUsers!, remoteUid],
    });
  }

  onUserOffline(
    connection: RtcConnection,
    remoteUid: number,
    reason: UserOfflineReasonType
  ) {
    this.info(
      'onUserOffline',
      'connection',
      connection,
      'remoteUid',
      remoteUid,
      'reason',
      reason
    );
    const { remoteUsers } = this.state;
    if (remoteUsers === undefined) return;
    this.setState({
      remoteUsers: remoteUsers!.filter((value) => value !== remoteUid),
    });
  }

  render() {
    const users = this.renderUsers();
    const configuration = this.renderConfiguration();
    return (
      <AgoraView className={AgoraStyle.screen}>
        <AgoraView className={AgoraStyle.content}>
          {users ? this.renderUsers() : undefined}
        </AgoraView>
        <AgoraView className={AgoraStyle.rightBar}>
          {this.renderChannel()}
          {configuration ? (
            <>
              <AgoraDivider>
                {`The Configuration of ${this.constructor.name}`}
              </AgoraDivider>
              {configuration}
            </>
          ) : undefined}
          <AgoraDivider />
          {this.renderAction()}
        </AgoraView>
      </AgoraView>
    );
  }

  protected renderChannel(): ReactNode {
    const { channelId, joinChannelSuccess } = this.state;
    return (
      <>
        <AgoraTextInput
          onChangeText={(text) => {
            this.setState({ channelId: text });
          }}
          placeholder={`channelId`}
          value={channelId}
        />
        <AgoraButton
          title={`${joinChannelSuccess ? 'leave' : 'join'} Channel`}
          onPress={() => {
            joinChannelSuccess ? this.leaveChannel() : this.joinChannel();
          }}
        />
      </>
    );
  }

  protected renderUsers(): ReactNode {
    const { startPreview, joinChannelSuccess, remoteUsers } = this.state;
    return (
      <>
        {!!startPreview || joinChannelSuccess
          ? this.renderUser({
              uid: 0,
              sourceType: VideoSourceType.VideoSourceCamera,
            })
          : undefined}
        {!!startPreview || joinChannelSuccess ? (
          <AgoraList
            data={remoteUsers ?? []}
            renderItem={(item) =>
              this.renderUser({
                uid: item,
                sourceType: VideoSourceType.VideoSourceRemote,
              })
            }
          />
        ) : undefined}
      </>
    );
  }

  protected renderUser(user: VideoCanvas): ReactElement {
    const { enableVideo } = this.state;
    return (
      <AgoraCard title={`${user.uid} - ${user.sourceType}`}>
        {enableVideo ? (
          <>
            <AgoraText>Click view to mirror</AgoraText>
            {this.renderVideo(user)}
          </>
        ) : undefined}
      </AgoraCard>
    );
  }

  protected renderVideo(user: VideoCanvas): ReactElement {
    return <RtcSurfaceView canvas={user} />;
  }

  protected renderConfiguration(): ReactNode {
    return undefined;
  }

  protected renderAction(): ReactNode {
    return undefined;
  }

  private _logSink(
    level: 'debug' | 'log' | 'info' | 'warn' | 'error',
    message?: any,
    ...optionalParams: any[]
  ): string {
    console[level](message, optionalParams);
    return `${optionalParams.map((v) => JSON.stringify(v))}`;
  }

  protected debug(message?: any, ...optionalParams: any[]): void {
    this.alert(message, this._logSink('debug', message, optionalParams));
  }

  protected log(message?: any, ...optionalParams: any[]): void {
    this._logSink('log', message, optionalParams);
  }

  protected info(message?: any, ...optionalParams: any[]): void {
    this._logSink('info', message, optionalParams);
  }

  protected warn(message?: any, ...optionalParams: any[]): void {
    this._logSink('warn', message, optionalParams);
  }

  protected error(message?: any, ...optionalParams: any[]): void {
    this._logSink('error', message, optionalParams);
  }

  protected alert(title: string, message?: string): void {
    alert(`${title}: ${message}`);
  }
}
