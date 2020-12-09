#include <cstdio>
#include <cstdlib>
#include <thread>
#include <map>
#include <memory>
#include <string>


#ifdef WIN32
#include <windows.h>

using socklen_t = int;
#endif

#ifdef __linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define closesocket close
#endif


struct TcpClient;
std::map<int, std::shared_ptr<TcpClient>> ClientMap;

struct TcpClient
{
    int cfd{-1};
    std::string ip;
    unsigned short port;
    char* buffer{nullptr};
    bool flag{false};
    std::thread sth;
    TcpClient()
    {
        buffer = new char[2048];
    }
    ~TcpClient()
    {
        flag = false;
        if (sth.joinable())
            sth.join();
        delete buffer;
    }

    void process()
    {
        while (flag)
        {
            int rsize = recv(cfd, buffer, 2047, 0);
            if (rsize <= 0)
                break;
            buffer[rsize] = 0;
            printf("%s:%d recv=>%s\n", ip.c_str(), port, buffer);
            int ssize = send(cfd, buffer, rsize, 0);
        }
        closesocket(cfd);
        flag = false;
    }
};

int main(int argc, char* argv[])
{
#ifdef WIN32
    WSADATA ws;
    WSAStartup(MAKEWORD(2, 2), &ws);
#endif
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    do
    {
        unsigned short port = 8000;
        if (argc > 1)
            port = atoi(argv[1]);
        sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_port = htons(port);
        server.sin_addr.s_addr = INADDR_ANY;//inet_addr("192.168.8.98");
        int ret = bind(sfd, (const sockaddr*)(&server), sizeof(server));
        if (ret < 0)
        {
            printf("bind port %d failed!\n", port);
            int en = -1;
            en = WSAGetLastError();
            break;
        }

        listen(sfd, 10);
        while (true)
        {
            sockaddr_in caddr;
            socklen_t len = sizeof(caddr);
            int cfd = 0;
            cfd = accept(sfd, (sockaddr*)(&caddr), &len);
            if (cfd > 0)
            {
                std::shared_ptr<TcpClient> client{new TcpClient};
                client->cfd = cfd;
                client->ip = std::string(inet_ntoa(caddr.sin_addr));
                client->port = htons(caddr.sin_port);
                client->flag = true;
                client->sth = std::thread(&TcpClient::process, client);
                ClientMap[cfd] = client;
                printf("%s:%d connect success!file handle:%d\n", client->ip.c_str(), client->port, cfd);
            }
        }
    }
    while (false);
    closesocket(sfd);
}
