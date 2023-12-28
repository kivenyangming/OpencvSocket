import socket
import cv2
# http://localhost:4399/camera

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # tcp协议
s.bind(("0.0.0.0", 4399))  # 4399为开通的端口号
s.listen(5)  # 可监听数量


# 图像数据压缩转字节
def mat2byte(cvmat):
    img_bytes = cv2.imencode('.jpg', cvmat, [1, 20])[1].tobytes()  # 仅保留20%的数据量 可自行涉及
    return img_bytes


if __name__ == "__main__":
    capture = cv2.VideoCapture(0)  # 读取摄像头\视频\rtsp流
    ref, frame = capture.read()

    if not ref:
        raise ValueError("未能正确读取摄像头（视频），请注意是否正确安装摄像头（是否正确填写视频路径）。")

    while True:
        c, a = s.accept()
        c.recv(1024).decode()
        header = b'HTTP/1.1 200 OK\r\nContent-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n'
        c.sendall(header)
        print(header)
        while True:
            ref, img = capture.read()  # 读取图像
            body = mat2byte(img)
            r = b' --frame\r\nContent-Type: image/jpeg\r\n\r\n' + body
            # 使其客户端断开后仍可重新连接
            try:
                c.sendall(r)
            except ConnectionAbortedError:
                break
