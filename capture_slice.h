#ifndef _capture_slice__hh
#define _capture_slice__hh

#ifdef __cplusplus
extern "C" {
#endif // c++

typedef struct capture_t capture_t;

/** 打开 capture，指定 name，大小，帧率
 */
capture_t *capture_open(const char *name, int width, int height, int fps, int kbits);

/** 得到下一帧，压缩后，输出的是 annexb 格式的 slice
    如果 data 有效，则返回字节长度
 */
int capture_next_slice(capture_t *cap, void **data, int64_t *pts);

/** 关闭
 */
void capture_close(capture_t *cap);


#ifdef __cplusplus
}
#endif // c++

#endif //

