#include "ftpclient.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>

FTPClient::FTPClient()
{
    this->downloadDir = "client_downloads";
    this->currentUser = "";
    this->isAuthed = false;
    this->serverHost = "127.0.0.1";
    this->serverPort = 21;

    // Tao thu muc client_downloads neu chua ton tai
    CreateDirectoryA(downloadDir.c_str(), NULL);
}

FTPClient::~FTPClient()
{
    quitSession();
}

string FTPClient::readResponse()
{
    char buffer[512];
    int bytes = recvGetLine(buffer, sizeof(buffer));
    if (bytes > 0)
    {
        // Loai bo \r\n o cuoi dong
        if (buffer[bytes - 1] == '\n') buffer[bytes - 1] = '\0';
        if (bytes >= 2 && buffer[bytes - 2] == '\r') buffer[bytes - 2] = '\0';
        return string(buffer);
    }
    return "";
}

bool FTPClient::sendCommand(const string& cmd, string& response)
{
    string fullCmd = cmd + "\r\n";
    int sent = sendStringRequest(fullCmd);
    if (sent <= 0)
    {
        return false;
    }
    response = readResponse();
    return !response.empty();
}

bool FTPClient::login(const string& host, unsigned short port, const string& user, const string& pass)
{
    // Neu dang co phien truoc do, dong lai
    quitSession();

    this->serverHost = host;
    this->serverPort = port;

    cout << "[FTP Client] Dang ket noi den " << host << ":" << port << "..." << endl;
    if (!open(host, port))
    {
        cout << "[-] Loi: Khong the ket noi den Server FTP!" << endl;
        return false;
    }

    // Doc thong bao chao mung tu Server (ma 220)
    string greeting = readResponse();
    cout << "[Server] " << greeting << endl;
    if (greeting.find("220") != 0)
    {
        cout << "[-] Server khong san sang tiep nhan." << endl;
        close();
        return false;
    }

    // 1. Gui lenh USER
    string resp;
    if (!sendCommand("USER " + user, resp))
    {
        cout << "[-] Loi khi gui lenh USER!" << endl;
        close();
        return false;
    }
    cout << "[Server] " << resp << endl;

    // 2. Gui lenh PASS
    if (!sendCommand("PASS " + pass, resp))
    {
        cout << "[-] Loi khi gui lenh PASS!" << endl;
        close();
        return false;
    }
    cout << "[Server] " << resp << endl;

    if (resp.find("230") == 0)
    {
        isAuthed = true;
        currentUser = user;
        cout << "[+] Dang nhap thanh cong voi tai khoan: " << currentUser << endl;
        return true;
    }
    else
    {
        isAuthed = false;
        currentUser = "";
        cout << "[-] Dang nhap that bai! Vui long kiem tra lai tai khoan va mat khau." << endl;
        close();
        return false;
    }
}

bool FTPClient::openDataConnection(TcpSocket& dataSocket)
{
    // Gui lenh PASV de yeu cau Server mo kenh du lieu thu dong
    string resp;
    if (!sendCommand("PASV", resp))
    {
        return false;
    }

    // Parse ma phan hoi: 227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)
    size_t startPos = resp.find('(');
    size_t endPos = resp.find(')');
    if (startPos == string::npos || endPos == string::npos || endPos <= startPos)
    {
        cerr << "[-] Loi phan tich phan hoi PASV tu Server: " << resp << endl;
        return false;
    }

    string numbersStr = resp.substr(startPos + 1, endPos - startPos - 1);
    stringstream ss(numbersStr);
    string token;
    vector<int> nums;
    while (getline(ss, token, ','))
    {
        nums.push_back(atoi(token.c_str()));
    }

    if (nums.size() < 6)
    {
        cerr << "[-] Thong tin IP/Port trong ban tin PASV khong hop le!" << endl;
        return false;
    }

    ostringstream ipOss;
    ipOss << nums[0] << "." << nums[1] << "." << nums[2] << "." << nums[3];
    string dataIP = ipOss.str();
    unsigned short dataPort = (unsigned short)(nums[4] * 256 + nums[5]);

    // Ket noi den kenh du lieu cua Server
    try
    {
        dataSocket.connect(dataIP, dataPort);
        return true;
    }
    catch (SocketException& e)
    {
        cerr << "[-] Khong the ket noi kenh du lieu Data Socket: " << e.what() << endl;
        return false;
    }
}

vector<string> FTPClient::listFiles(const string& folder)
{
    vector<string> files;
    if (!isAuthed)
    {
        cout << "[-] Ban chua dang nhap! Vui long dung lenh login truoc." << endl;
        return files;
    }

    TcpSocket dataSocket;
    if (!openDataConnection(dataSocket))
    {
        cout << "[-] Khong the mo kenh du lieu PASV!" << endl;
        return files;
    }

    // Gui lenh LIST
    string listCmd = "LIST";
    if (!folder.empty() && folder != ".")
    {
        listCmd += " " + folder;
    }

    string resp;
    if (!sendCommand(listCmd, resp))
    {
        dataSocket.close();
        return files;
    }
    cout << "[Server] " << resp << endl;

    // Doc toan bo du lieu danh sach tu dataSocket
    string rawData;
    char buffer[1024];
    int bytes = 0;
    while ((bytes = dataSocket.recv(buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytes] = '\0';
        rawData += buffer;
    }
    dataSocket.close();

    // Nhan thong bao ket thuc tu Server (226)
    string finishResp = readResponse();
    cout << "[Server] " << finishResp << endl;

    // Tach cac dong thanh danh sach file
    stringstream ss(rawData);
    string line;
    while (getline(ss, line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r')
        {
            line.erase(line.size() - 1);
        }
        if (!line.empty())
        {
            files.push_back(line);
        }
    }

    return files;
}

bool FTPClient::downloadFile(const string& remotePath)
{
    if (!isAuthed)
    {
        cout << "[-] Ban chua dang nhap! Vui long dung lenh login truoc." << endl;
        return false;
    }

    TcpSocket dataSocket;
    if (!openDataConnection(dataSocket))
    {
        cout << "[-] Khong the mo kenh du lieu PASV!" << endl;
        return false;
    }

    // Gui lenh RETR de tai file
    string resp;
    if (!sendCommand("RETR " + remotePath, resp))
    {
        dataSocket.close();
        return false;
    }
    cout << "[Server] " << resp << endl;

    if (resp.find("150") != 0)
    {
        dataSocket.close();
        return false;
    }

    // Lay ten file tu remotePath (vi du "docs/tailieu.txt" -> "tailieu.txt")
    string localFileName = remotePath;
    size_t lastSlash = localFileName.find_last_of("/\\");
    if (lastSlash != string::npos)
    {
        localFileName = localFileName.substr(lastSlash + 1);
    }

    string localFilePath = downloadDir + "/" + localFileName;
    ofstream outFile(localFilePath.c_str(), ios::binary);
    if (!outFile.is_open())
    {
        cout << "[-] Khong the tao file tai: " << localFilePath << endl;
        dataSocket.close();
        return false;
    }

    // Doc du lieu tu dataSocket va ghi vao file
    char buffer[4096];
    int bytesRead = 0;
    long long totalBytes = 0;
    while ((bytesRead = dataSocket.recv(buffer, sizeof(buffer))) > 0)
    {
        outFile.write(buffer, bytesRead);
        totalBytes += bytesRead;
    }
    outFile.close();
    dataSocket.close();

    string finishResp = readResponse();
    cout << "[Server] " << finishResp << endl;

    if (finishResp.find("226") == 0)
    {
        cout << "[+] Da luu tep: " << localFilePath << " (" << totalBytes << " bytes)" << endl;
        return true;
    }
    else
    {
        cout << "[-] Tai file that bai hoac bi gian doan!" << endl;
        return false;
    }
}

void FTPClient::bulkDownload(const string& folder)
{
    cout << "\n=======================================================" << endl;
    cout << "  THUC HIEN BULK DOWNLOAD (TAI HANG LOAT TOAN BO THU MUC)" << endl;
    cout << "  Thu muc muc tieu tren Server: '" << (folder.empty() ? "goc" : folder) << "'" << endl;
    cout << "=======================================================" << endl;

    // 1. Lay danh sach file trong thu muc tu Server
    vector<string> items = listFiles(folder);
    if (items.empty())
    {
        cout << "[*] Khong co tep tin nao trong thu muc nay de tai ve." << endl;
        return;
    }

    // Loc ra cac file thuc su (bo qua thu muc con ket thuc bang '/')
    vector<string> filesToDownload;
    for (size_t i = 0; i < items.size(); ++i)
    {
        string item = items[i];
        if (!item.empty() && item[item.size() - 1] != '/')
        {
            filesToDownload.push_back(item);
        }
    }

    if (filesToDownload.empty())
    {
        cout << "[*] Chi co thu muc con, khong co tap tin truc tiep de tai ve." << endl;
        return;
    }

    int totalFiles = (int)filesToDownload.size();
    cout << "[*] Phat hien " << totalFiles << " tap tin can tai ve." << endl;

    int successCount = 0;
    for (int i = 0; i < totalFiles; ++i)
    {
        string fileName = filesToDownload[i];
        string remotePath = folder.empty() ? fileName : (folder + "/" + fileName);

        cout << "\n-------------------------------------------------------" << endl;
        cout << "[" << (i + 1) << "/" << totalFiles << "] Dang tai: " << fileName << "..." << endl;

        if (downloadFile(remotePath))
        {
            successCount++;
        }
    }

    cout << "\n=======================================================" << endl;
    cout << "  HOAN TAT BULK DOWNLOAD!" << endl;
    cout << "  Ket qua: " << successCount << "/" << totalFiles << " file tai ve thanh cong." << endl;
    cout << "  Cac file duoc luu tai thu muc: ./" << downloadDir << "/" << endl;
    cout << "=======================================================" << endl;
}

void FTPClient::quitSession()
{
    if (isConnected())
    {
        try
        {
            string resp;
            sendCommand("QUIT", resp);
        }
        catch (...) {}
        close();
    }
    isAuthed = false;
    currentUser = "";
}
