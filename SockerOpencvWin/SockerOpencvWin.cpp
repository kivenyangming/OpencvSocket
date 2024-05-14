#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>

#include <ws2tcpip.h>  
#include <mstcpip.h>  

using namespace cv;
#pragma comment(lib, "ws2_32.lib")  // 指定链接winsock库
//http://192.168.2.186:8564/camera

// 图像数据压缩转字节
std::vector<uchar> mat2byte(const Mat& cvmat) {
    std::vector<int> params = { cv::IMWRITE_JPEG_QUALITY, 20 };
    std::vector<uchar> imgBytes;
    imencode(".jpg", cvmat, imgBytes, params);
    return imgBytes;
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    SOCKADDR_IN serverAddr, clientAddr;
    
    int addrLen = sizeof(clientAddr);
    char buffer[1024];
    std::string header = "HTTP/1.1 200 OK\r\nContent-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
    std::string boundary = "--frame\r\nContent-Type: image/jpeg\r\n\r\n";

    // 初始化Winsock
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // 创建TCP套接字
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        return -1;
    }

    // 绑定端口
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(8564);

    int resurts = bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (resurts == SOCKET_ERROR) {
        std::cerr << "Failed to bind socket: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup(); // 清理Winsock
        return -1;
    }




    // 监听
    listen(serverSocket, 5);
    std::cout << "Waiting for connection..." << std::endl;

    while (true) {
        clientSocket = accept(serverSocket, (PSOCKADDR)&clientAddr, &addrLen);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Failed to accept connection: " << WSAGetLastError() << std::endl;
            continue;
        }

        // 注意这里不需要发送header的大小，因为send的第三个参数是int类型
        if (send(clientSocket, header.c_str(), static_cast<int>(header.size()), 0) == SOCKET_ERROR) {
            std::cerr << "Failed to send data" << std::endl;
            closesocket(clientSocket);
            continue;
        }
        std::cout << header << std::endl;

        VideoCapture capture("D:/simaRPN/output.mp4");
        if (!capture.isOpened()) {
            std::cerr << "Could not open camera" << std::endl;
            closesocket(clientSocket);
            continue;
        }

        while (true) {
            Mat frame;
            capture >> frame;
            if (frame.empty()) {
                std::cerr << "Could not capture frame" << std::endl;
                break;
            }

            std::vector<uchar> body = mat2byte(frame);
            std::string sendData = boundary + std::string(body.begin(), body.end());
            int sentBytes = send(clientSocket, sendData.c_str(), static_cast<int>(sendData.size()), 0);
            if (sentBytes == SOCKET_ERROR) {
                std::cerr << "Failed to send data" << std::endl;
                break;
            }
        }

        capture.release();
        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}