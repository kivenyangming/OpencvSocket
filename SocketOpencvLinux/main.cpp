#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;
using namespace cv;

// 图像数据压缩转字节
vector<uchar> mat2byte(const Mat& cvmat) {
    vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 20}; // 仅保留20%的数据量，可自行调整
    vector<uchar> imgBytes;
    imencode(".jpg", cvmat, imgBytes, params);
    return imgBytes;
}

int main() {
    // 初始化摄像头
    VideoCapture capture(0); // 读取摄像头
    if (!capture.isOpened()) {
        cerr << "未能正确读取摄像头，请注意是否正确安装摄像头。" << endl;
        return -1;
    }

    // 创建TCP套接字
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Failed to create socket");
        return -1;
    }

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(4399);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // 绑定端口
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Failed to bind socket");
        close(serverSocket);
        return -1;
    }

    // 监听
    listen(serverSocket, 5);

    cout << "等待连接..." << endl;

    while (true) {
        sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);
        if (clientSocket < 0) {
            perror("Failed to accept connection");
            continue;
        }

        char buffer[1024];
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            perror("Failed to receive data");
            close(clientSocket);
            continue;
        }

        string header = "HTTP/1.1 200 OK\r\nContent-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
        send(clientSocket, header.c_str(), header.size(), 0);
        cout << header << endl;

        while (true) {
            Mat frame;
            capture >> frame; // 读取图像帧
            if (frame.empty()) {
                cerr << "未能捕获到图像帧。" << endl;
                break;
            }

            vector<uchar> body = mat2byte(frame);
            string boundary = "--frame\r\nContent-Type: image/jpeg\r\n\r\n";
            string sendData = boundary + string(body.begin(), body.end());

            ssize_t sentBytes = send(clientSocket, sendData.c_str(), sendData.size(), 0);
            if (sentBytes <= 0) {
                perror("Failed to send data");
                if (errno == EPIPE) { // 客户端断开连接
                    cerr << "客户端已断开连接。" << endl;
                    break;
                }
            }
        }

        close(clientSocket);
    }

    capture.release();
    close(serverSocket);
    return 0;
}