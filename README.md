# OpencvSocket
&emsp;&emsp;使用opencv读取视频并使用socket进行传输视频画面的脚本文件，相较于调用ffmpeg传输节约了90%的数据量
# 数据比对
&emsp;&emsp;对于同一图像进行数据传输，这里采取了调用ffmpeg与socket库进行比对:
| 方法 | 数据型 |数据量 |协议 |
| --- | --- | --- | --- |
| **opencv+ffmpeg** | **bytes** |**451584** |**rtsp\rtmp** |
| **opencv+socket** | **bytes** |**69366** |**tcp**|
| **pencv+encode+socket** | **bytes** |**18304** |**tcp**|

根据上述信息可以看出经过imencode进行压缩jpg数据压缩保留原始30%的数据对图像信息的损失肉眼看不出明显失真且对传输的数据量仅为**opencv+ffmpeg**方式的4.053%
