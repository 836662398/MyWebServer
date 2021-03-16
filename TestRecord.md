# 测试记录

## 命令：

```
webbench -c 1000 -t 60 -2 http://127.0.0.1:8000/
```

### ET_MyWebServer:

单进程，5个sub线程：
! [image] (https://github.com/836662398/MyWebServer/blob/ET/pic.assets/60S/et_p1_s5.png)

双进程，每个进程3个sub线程：
! [image] (https://github.com/836662398/MyWebServer/blob/ET/pic.assets/60S/ET_P2_S3.png)

### LT_MyWebServer:

单进程，5个sub线程：
! [image] (https://github.com/836662398/MyWebServer/blob/ET/pic.assets/60S/LT_P1_S5.png)

双进程，每个进程3个sub线程：
! [image] (https://github.com/836662398/MyWebServer/blob/ET/pic.assets/60S/lt_p2_s3.png)

### muduo:

5个sub线程：
! [image] (https://github.com/836662398/MyWebServer/blob/ET/pic.assets/60S/muduo_5.png)





