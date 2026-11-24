#ifndef FTPCLIENT_H
#define FTPCLIENT_H

#include "tcpclient.h"
#include <string>
#include <vector>

using namespace std;

// ============================================================================
// LOP FTPClient: Ke thua tu TcpClient (Framework cua Thay Bui Phong)
// Thuc hien giao thuc FTP phia Client: dang nhap, liet ke thu muc,
// tai file don le va tai toan bo thu muc (Bulk Download - De tai 13)
// ============================================================================
class FTPClient : public TcpClient
{
private:
    string downloadDir;      // Thu muc luu tru file tai ve o phia Client
    string currentUser;      // Ten nguoi dung hien tai dang dang nhap
    bool isAuthed;           // Trang thai da xac thuc thanh cong hay chua
    string serverHost;       // Dia chi IP hoac ten mien cua Server
    unsigned short serverPort; // Cong dieu khien cua Server (mac dinh 21)

public:
    FTPClient();
    virtual ~FTPClient();

    // 1. Dang nhap vao FTP Server voi IP, Port, Username, Password
    bool login(const string& host, unsigned short port, const string& user, const string& pass);

    // 2. Lay danh sach tep tin/thu muc tu Server
    vector<string> listFiles(const string& folder = "");

    // 3. Tai mot file don le tu Server ve client_downloads/
    bool downloadFile(const string& remotePath);

    // 4. CHUC NANG CHINH DE TAI 13: Tai toan bo mot thu muc (Bulk Download)
    void bulkDownload(const string& folder);

    // 5. Ket thuc phien lam viec va dong ket noi
    void quitSession();

    // Kiem tra da dang nhap chua
    bool isAuthenticated() const { return isAuthed; }
    const string& getCurrentUser() const { return currentUser; }
    const string& getDownloadDir() const { return downloadDir; }

private:
    // Gui lenh va nhan phan hoi tren kenh dieu khien (localsocket)
    bool sendCommand(const string& cmd, string& response);

    // Doc mot dong phan hoi tu kenh dieu khien
    string readResponse();

    // Thiet lap kenh du lieu thu dong (PASV) de truyen nhan file hoac danh sach
    bool openDataConnection(TcpSocket& dataSocket);
};

#endif // FTPCLIENT_H
