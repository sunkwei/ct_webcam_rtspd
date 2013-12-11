#ifndef _zqsender__hh
#define _zqsender__hh

#ifdef __cplusplus
extern "C" {
#endif // c++

typedef void ZqSenderCtx;
typedef void ZqSenderAudioUdpCtx;
typedef void ZqSenderTcpCtx;

/** 打开sender

  	@param ctx: 出参, 返回操作句柄
	@param sock: 调用者提供的 tcp socket 句柄, 必须是非阻塞模式
	@param maxdelay: 最大缓冲延迟, 如果超过改值, zqsnd_send() 将返回错误
	@return >0 成功, ctx有效
 */
int zqsnd_open (ZqSenderCtx **ctx, int sock, double maxdelay);

/** 启动udp server, 用于发送音频数据

  	@param ctx: 出参, 对应一个udp server
	@param port: udp port
	@param bindip: local ip
	@return > 0 成功
 */
int zqsnd_open_audio_udp_server (ZqSenderAudioUdpCtx **ctx, int port, const char *bindip);

/** 启动 tcp server
 */
int zqsnd_open_tcp_server (ZqSenderTcpCtx **ctx, int port, const char *bindip);

/** 关闭
 */
int zqsnd_close (ZqSenderCtx *ctx);
int zqsnd_close_audio_udp_server (ZqSenderAudioUdpCtx *ctx);
int zqsnd_close_tcp_server (ZqSenderTcpCtx *ctx);

/** 返回 tcp sender 端口后，当 zqsnd_open_tcp_server() 时，port 参数为0时，需要调用这个函数，获取实际使用的端口号
 */
int zqsnd_tcp_get_listen_port(ZqSenderTcpCtx *ctx);

/** 发送数据，不必关心数据的分段，但是必须是 epkt, zqpkt 或 flv 格式之一

  	@return >= 0 正确
		<0 网络失败, 需要 zqsnd_close()
 */
int zqsnd_send (ZqSenderCtx *ctx, const void *data, int len);

/** 通过udp发送音频数据, 必须是完整帧, 将自动发送到所有 connect() 到本端口的udp clients
  	@param frame_data: 帧数据, 必须为 epkt, zqpkt 之一, 并且完整的一帧数据
 */
int zqsnd_sendframe_audio (ZqSenderAudioUdpCtx *ctx, const void *frame_data, int len);
int zqsnd_tcp_send (ZqSenderTcpCtx *ctx, const void *data, int len);

/** 返回当前缓冲的数据的时间
 */
double zqsnd_get_delay (ZqSenderCtx *ctx);

#ifdef __cplusplus
}
#endif // c++

#endif // zqsender.h
