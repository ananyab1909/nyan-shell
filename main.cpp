#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<functional>
#include<cstdlib>
#include<dirent.h>
#include<sys/stat.h>
#include<fstream>
#include<cstdlib>
#include<ctime>
#include<unistd.h>
#include<string.h>
#include<bitset>
#include<unistd.h>
#include<sys/types.h>
#include<sstream>
#include<signal.h>
#include<set>
#include<limits>
#include<algorithm>
#include<list>
#include<csignal>
#include<unordered_map>
#include<unistd.h>
#ifdef _WIN32
#include<windows.h>
#endif

using namespace std;

map<string, void (*)(const vector<string>&)> commandRegister; 
string currentWorkingDirectory;
string prevDir;
vector<string> history;
map<string, bool> loggedInUsers;
string currentUser;
list<string> loginOrder;
vector<string> allUsers;
vector<string> sudoCommands;

vector<string> tokenize(const string& input) {
    vector<string> tokens;
    string token;
    for (char c : input) {
        if (c == ' ') { 
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear(); 
            }
        }
        token += c;
    }
    if (!token.empty()) {
        tokens.push_back(token);
    }
    return tokens;
}

#ifdef __linux__
#include <iostream>

void setColor(int color) {
    cout << "\033[" << color << "m";
}
#endif

void lsCommand() {
    DIR *dir;
    struct dirent *ent;
    dir = opendir(".");
    if (dir != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == DT_DIR) {
                #ifdef __linux__
                setColor(34); 
                #endif
                cout << ent->d_name << endl;
                #ifdef __linux__
                setColor(37); 
                #endif
            } else {
                #ifdef __linux__
                setColor(32); 
                #endif
                cout << ent->d_name << endl;
                #ifdef __linux__
                setColor(37);
                #endif 
            }
        }
        closedir(dir);
    } else {
        cout << "Error: Unable to read directory" << std::endl;
    }
}

void ls_l_command() {
    DIR *dir;
    struct dirent *ent;
    dir = opendir(".");
    if (dir != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            struct stat fileStat;
            stat(ent->d_name, &fileStat);
            if (S_ISDIR(fileStat.st_mode)) {
                cout << "d";
            } else {
                cout << "-";
            }
            cout << ((fileStat.st_mode & 0400)? "r" : "-"); 
            cout << ((fileStat.st_mode & 0200)? "w" : "-");         
            cout << ((fileStat.st_mode & 0100)? "x" : "-"); 
            cout << ((fileStat.st_mode & 0040)? "r" : "-"); 
            cout << ((fileStat.st_mode & 0020)? "w" : "-"); 
            cout << ((fileStat.st_mode & 0010)? "x" : "-"); 
            cout << ((fileStat.st_mode & 0004)? "r" : "-"); 
            cout << ((fileStat.st_mode & 0002)? "w" : "-"); 
            cout << ((fileStat.st_mode & 0001)? "x" : "-"); 
            cout << " " << fileStat.st_nlink;
            cout << " " << fileStat.st_size;
            cout << " " << ctime(&fileStat.st_mtime);
            cout << " " << ent->d_name << endl;
        }
        closedir(dir);
    }
}

void lsLCommand(const vector<string>& args) {
    int numArgs=args.size();
    if (numArgs==1) {
        lsCommand();
    }
    else {
        ls_l_command();
    }
}

void psCommand(const vector<string>& args) {
    #ifdef _WIN32
    string command = "tasklist";
    #else
        string command = "ps aux";
    #endif
    system(command.c_str());
}

void cdCommand(const vector<string>& args) {
    if (args.size() != 2) {
        cout << "Error: cd command expects one argument" << endl;
        return;
    }
    string dir = args[1];
    dir.erase(0, dir.find_first_not_of(" \t")); 
    dir.erase(dir.find_last_not_of(" \t") + 1); 

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        cout << "Current working directory: " << cwd << endl;
        prevDir = cwd;
    } else {
        cout << "Error: Unable to get current working directory" << endl;
    }
    cout << "Trying to change directory to " << dir << endl;
    if (chdir(dir.c_str()) == -1) {
        cout << "Error: Unable to change directory" << endl;
    } else {
        cout << "Directory changed to " << dir << endl;
    }
}

void cdbackCommand(const vector<string>& args) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd))!= NULL) {
        cout << "Current working directory: " << cwd << endl;
    } else {
        cout << "Error: Unable to get current working directory" << endl;
    }
    if (!prevDir.empty()) {
        if (chdir(prevDir.c_str()) == -1) {
            cout << "Error: Unable to change directory" << endl;
        } else {
            cout << "Directory changed" << endl;
            prevDir.clear(); 
        }
    } else {
        cout << "Error: No previous directory to go back to" << endl;
    }
}

void pwdCommand(const vector<string>& args) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        currentWorkingDirectory = cwd;
        cout << currentWorkingDirectory << endl;
    } else {
        cout << "Error: Unable to get current working directory" << endl;
    }
}

void mkdirCommand(const vector<string>& args) {
    if (args.size()!= 2) {
        cout << "Error: mkdir command expects one argument" << endl;
        return;
    }
    string dir = args[1];
    dir.erase(0, dir.find_first_not_of(" \t")); 
    dir.erase(dir.find_last_not_of(" \t") + 1);
    char cmd[256];
    sprintf(cmd, "mkdir %s", dir.c_str());
    if (system(cmd)==0) { 
        cout << "Directory created successfully!" << endl;
    }
    else {
        cout << "Error: Unable to create" << endl;
    }
}

void rmdirCommand(const vector<string>& args) {
    if (args.size()!= 2) {
        cout << "Error: rmdir command expects one argument" << endl;
        return;
    }
    string dir = args[1];
    dir.erase(0, dir.find_first_not_of(" \t")); 
    dir.erase(dir.find_last_not_of(" \t") + 1);
    #ifdef _WIN32
    char cmd[256];
    sprintf(cmd, "del /q /s /f \"%s\\*\"", dir.c_str());
    system(cmd);
    sprintf(cmd, "rmdir /q /s \"%s\"", dir.c_str());
    if (system(cmd) == 0) {
        printf("Directory deleted successfully!\n");
    } else {
        printf("Error: Unable to delete directory\n");
    }
    #elif __linux__
    string cmd = "rm -rf " + dir;
    if (system(cmd.c_str()) == 0) {
        cout << "File deleted successfully!" << endl;
    } else {
        cout << "Error: Unable to delete file" << endl;
    }
    #endif
}

void touchCommand(const vector<string>& args) {
    if (args.size()!= 2) {
        cout << "Error: touch command expects one argument" << endl;
        return;
    }
    string dir = args[1];
    dir.erase(0, dir.find_first_not_of(" \t")); 
    dir.erase(dir.find_last_not_of(" \t") + 1); 
    #ifdef _WIN32
    if (_access(dir.c_str(), 0) == 0) {
        cout << "File already exists!" << endl;
    }
    else {
        ofstream(dir).close(); 
        cout << "File created successfully!" << endl;
    }
    #elif __linux__
    string file = args[1];
    file.erase(0, file.find_first_not_of(" \t"));
    file.erase(file.find_last_not_of(" \t") + 1);
    fstream f(file.c_str());
    if (f.good()) {
        cout << "File already exists!" << endl;
    } else {
        ofstream outfile(file.c_str());
        if (outfile.is_open()) {
            outfile.close();
            cout << "File created successfully!" << endl;
        } else {
            cout << "Error: unable to create file" << endl;
        }
    }
    #endif
}

void catCommand(const vector<string>& args) {
    if (args.size()!= 2) {
        cout << "Error: cat command expects one argument" << endl;
        return;
    }
    string dir = args[1];
    dir.erase(0, dir.find_first_not_of(" \t")); 
    dir.erase(dir.find_last_not_of(" \t") + 1); 
    ifstream file(dir);
    if (!file) {
        cout<< "Error: No such files exist!"<<endl;
    }
    else {
        string line;
        while (getline(file, line)) {
            cout << line << endl;
        }   
    }
}


void echoCommand(const vector<string>& args) {
    for (int i=1;i<args.size();i++) {
        cout << args[i];
        if (i < args.size() -1) {
            cout << " ";
        }
    }
    cout << endl;
}

void cpCommand(const vector<string>& args) {
    if (args.size()!= 3) {
        cout << "Error: cp command expects only two arguments" << endl;
        return;
    }
    string source = args[1];
    source.erase(0, source.find_first_not_of(" \t")); 
    source.erase(source.find_last_not_of(" \t") + 1); 
    string destination = args[2];
    destination.erase(0, destination.find_first_not_of(" \t")); 
    destination.erase(destination.find_last_not_of(" \t") + 1);
    ifstream sourceFileStream(source, ios::binary);
    ofstream destinationFileStream(destination, ios::binary);

    if (!sourceFileStream.is_open() || !destinationFileStream.is_open()) {
        cout << "Error: unable to open file" << endl;
    }

    destinationFileStream << sourceFileStream.rdbuf();
    sourceFileStream.close();
    destinationFileStream.close();
    cout << "File copied successfully!" << endl;
}

void dateCommand(const vector<string>& args){
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%a %b %d %H:%M:%S %Z %Y", ltm);
    cout << buffer << endl;
}

void diffCommand(const vector<string>& args) {
    string file1=args[1];
    string file2 = args[2];
    string cmd = "fc" + file1 + " " + file2;
    system(cmd.c_str());
}

void hexdumpCommand(const vector<string>& args) {
    string source = args[1];
    source.erase(0, source.find_first_not_of(" \t")); 
    source.erase(source.find_last_not_of(" \t") + 1); 
    ifstream FileStream(source, ios::binary);
    if (!FileStream) {
        cerr << "Error opening file!" << endl;
    }
    char byte;
    while (FileStream.get(byte)) {
        bitset<8> b(byte); 
        cout << hex << b.to_ulong() << " ";
    }

    cout << endl;
    FileStream.close();
}

void wcCommand(const vector<string>& args){
    string source = args[1];
    source.erase(0, source.find_first_not_of(" \t")); 
    source.erase(source.find_last_not_of(" \t") + 1); 
    ifstream FileStream(source, ios::binary);
    if (!FileStream) {
        cout << "Error opening file!" << endl;
    }
    int lines = 0;
    int words = 0;
    int chars = 0;
    string line;
    while (getline(FileStream, line)) {
        ++lines;
        chars += line.size(); 

        size_t wordStart = 0;
        while (wordStart < line.size()) {
            while (wordStart < line.size() && isspace(line[wordStart])) {
                ++wordStart;
            }
            if (wordStart < line.size()) {
                ++words;
                while (wordStart < line.size() && !isspace(line[wordStart])) {
                    ++wordStart;
                }
            }
        }
    }
    FileStream.close();
    cout << " " << lines << " " << words << " " << chars << " " << "number" << endl;
}

void unameCommand(const vector<string>& args) {
    string osName;

#ifdef _WIN32
    osName = "Windows";
#elif __APPLE__
    osName = "macOS";
#elif __linux__
    osName = "Linux";
#else
    osName = "Unknown";
#endif

    cout << osName << endl;

}

void vimCommand(const vector<string>& args) {
    #ifdef __linux__
    if (args.size()!= 2) {
        cout << "Error: vim command expects only one argument" << endl;
    }
    string fileName = args[1];
    string command = "vim" + fileName;
    system(command.c_str());
    #else
    cout << "Not supported" << endl;
    #endif
}

void nanoCommand(const vector<string>& args) {
    if (args.size()!= 2) {
        cout << "Error: nano command expects only one argument" << endl;
    }
    #ifdef __linux__
    string fileName = args[1];
    string command = "nano" + fileName;
    system(command.c_str());
    #else
    cout << "Not supported" << endl;
    #endif
}

void rmCommand(const vector<string>& args) {
    if (args.size()!= 2) {
        cout << "Error: rm command expects only one arguments" << endl;
        return;
    }
    string file = args[1];
    #ifdef _WIN32
    file.erase(0, file.find_first_not_of(" \t")); 
    file.erase(file.find_last_not_of(" \t") + 1);
    char cmd[256];
    sprintf(cmd, "del /q /f \"%s\"", file.c_str());

    if (system(cmd) == 0) {
        cout << "File deleted successfully!" << endl;
    } else {
        cout << "Error: Unable to delete file." << endl;
    }
    #elif __linux__
    string cmd = "rm -rf " + file;
    if (system(cmd.c_str()) == 0) {
        cout << "File deleted successfully!" << endl;
    } else {
        cout << "Error: Unable to delete file" << endl;
    }
    #endif
}


void gzipCommand(const vector<string>& args) {
    if (args.size()!= 2) {
        cout << "Error: gzip command expects only one argument" << endl;
        return;
    }
    string file = args[1];
    file.erase(0, file.find_first_not_of(" \t")); 
    file.erase(file.find_last_not_of(" \t") + 1);
    #ifdef _WIN32
    cout << "gzip not supported in Windows" << endl;
    #elif __linux__
    string outputFile = file + ".gz";
    string command = "gzip -c " + file + " > " + outputFile;
    system(command.c_str());
    cout << "File created sucsessfully!" << endl;
    #endif
}

void freeCommand(const vector<string>& args) {
    #ifdef _WIN32
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memStatus);

    std::cout << "Memory Information:" << std::endl;
    std::cout << "-------------------" << std::endl;
    std::cout << "Total Physical Memory: " << memStatus.ullTotalPhys / 1024 << " MB" << std::endl;
    std::cout << "Available Physical Memory: " << memStatus.ullAvailPhys / 1024 << " MB" << std::endl;

    #elif __linux__
    std::ifstream ifs("/proc/meminfo");
    std::string line;

    std::string memTotal, memFree, memAvailable;

    while (std::getline(ifs, line)) {
        std::size_t found = line.find("MemTotal:");
        if (found != std::string::npos) {
            memTotal = line.substr(found + 9);
            memTotal.pop_back(); // remove the trailing " kB" suffix
        }

        found = line.find("MemFree:");
        if (found != std::string::npos) {
            memFree = line.substr(found + 8);
            memFree.pop_back(); // remove the trailing " kB" suffix
        }

        found = line.find("MemAvailable:");
        if (found != std::string::npos) {
            memAvailable = line.substr(found + 13);
            memAvailable.pop_back(); // remove the trailing " kB" suffix
        }
    }

    std::cout << "Memory Information:" << std::endl;
    std::cout << "-------------------" << std::endl;
    std::cout << "Total Physical Memory: " << memTotal << " kB" << std::endl;
    std::cout << "Available Physical Memory: " << memAvailable << " kB" << std::endl;

    #endif
}

void dfCommand(const vector<string>& args) {
    #ifdef _WIN32
    ULONGLONG totalBytes, freeBytes, totalFreeBytes, totalBytesAvailable;
    string path = "C:\\"; 

    if (GetDiskFreeSpaceExA(path.c_str(), (PULARGE_INTEGER)&freeBytes, (PULARGE_INTEGER)&totalBytes, (PULARGE_INTEGER)&totalBytesAvailable)) {
        double totalSpace = totalBytes / (1024.0 * 1024.0);
        double availableSpace = totalBytesAvailable / (1024.0 * 1024.0);
        double usedSpace = totalSpace - availableSpace;

        cout << "Filesystem      1K-blocks      Used Available Use% Mounted on\n";
        cout << path << "         " << totalSpace << "         " << usedSpace << "         " << availableSpace << "         " << (int)((usedSpace / totalSpace) * 100) << "%" << "         " << path << std::endl;
    } else {
        cerr << "Error: Unable to retrieve disk space information." << endl;
    }

    #elif __linux__
    system("df -h");
    #endif
}

void mvCommand(const vector<string>& args) {
    struct stat st;
    bool isMove = true; 
    string source = args[1];
    cout << "Source: [" << source << "]" << endl;
    source.erase(0, source.find_first_not_of(" \t")); 
    source.erase(source.find_last_not_of(" \t") + 1); 

    string destination = args[2];
    cout << "Destination: [" << destination << "]" << endl;
    destination.erase(0, destination.find_first_not_of(" \t")); 
    destination.erase(destination.find_last_not_of(" \t") + 1); 
    if (stat(destination.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
        cout << "Destination is a directory" << endl;
        string destFile = destination + "/" + source.substr(source.find_last_of('/') + 1);
        cout << "Destination file: [" << destFile << "]" << endl;
        if (isMove) {
            cout << "Moving file" << endl;
        } else {
            cout << "Renaming file" << endl;
        }
        if (rename(source.c_str(), destFile.c_str()) == 0) {
            cout << "File " << (isMove? "moved" : "renamed") << " successfully!" << endl;
        } else {
            cout << "Error: Unable to " << (isMove? "move" : "rename") << " file." << endl;
        }
    } else {
        cout << "Destination is not a directory" << endl;
        if (isMove) {
            cout << "Renaming file" << endl;
        } else {
            cout << "Renaming file" << endl;
        }
        if (rename(source.c_str(), destination.c_str()) == 0) {
            cout << "File " << (isMove? "moved" : "renamed") << " successfully!" << endl;
        } else {
            cout << "Error: Unable to " << (isMove? "move" : "rename") << " file." << endl;
        }
    }
    
}

void chmodCommand(const vector<string>& args) {
    string permissions = args[1];
    permissions.erase(0, permissions.find_first_not_of(" \t")); 
    permissions.erase(permissions.find_last_not_of(" \t") + 1);
    int numPer = stoi(permissions);
    string file = args[2];
    if (chmod(file.c_str(), numPer) == -1) {
        cout << "Error changing permissions: " << endl;
        return;
    }
    cout << "Permissions changed successfully!" << endl;

}

void uptimeCommand(const vector<string>& args) {
    
    #ifdef _WIN32
    cout << "Not supported in Windows" << endl; 
    #elif __linux__
    system("uptime");
    #endif  
}

void duCommand(const vector<string>& args) {
    string path;
    path = "C:\\";
    #ifdef _WIN32
    string command = "dir /s " + path;
    system(command.c_str());
    #elif __linux__
    string command = "du -d 1 " + path;
    system(command.c_str());
    #endif
}

void wgetCommand(const vector<string>& args) {
    #ifdef _WIN32
    cout << "Windows doesnt support it" << endl;
    #elif __linux__
    string url = args[1];
    string cmd = "wget " + url;
    system(cmd.c_str());
    cout << "File downloaded and stored successfully!" << endl;
    #endif
}

void uniqCommand(const vector<string>& args) {
    string source = args[1];
    source.erase(0, source.find_first_not_of(" \t")); 
    source.erase(source.find_last_not_of(" \t") + 1); 
    ifstream file(source);
    if (!file) {
        cerr << "Error opening file!" << endl;
    }
    set<string> uniqueLines;
    string line;
    while (getline(file, line)) {
        uniqueLines.insert(line);
    }
    if (uniqueLines.size() == 1) {
        cout << "No unique lines found!" << endl;
    } else {
        for (const auto& line : uniqueLines) {
            cout << line << endl;
        }
    }
    file.close();
}

void lessCommand(const vector<string>& args) {
    #ifdef __linux__
    string source = args[1];
    source.erase(0, source.find_first_not_of(" \t")); 
    source.erase(source.find_last_not_of(" \t") + 1);
    string command = "less " + source;
    system(command.c_str());
    #endif
}

void addCommand(const string& command) {
    history.push_back(command);
}

void displayHistory(const vector<string>& args) {
    int count = 1;
    for (const auto& command : history) {
        cout << "  " << count << "  " << command << endl;
        count++;
    }
}

void allCommand(const vector<string>& args) {
    #ifdef __linux__
    string command = "service --status-all";
    system(command.c_str());
    #endif
}

void envCommand(const vector<string>& args) {
    #ifdef _WIN32
    system("set");
    #elif __linux__
    system("env");
    #endif
}

void killCommand(const vector<string>& args) {
    #ifdef __linux__
    string pname;
    cout << "Enter name of process" << endl;
    cin >> pname;
    char cmd[256];
    sprintf(cmd, "pkill %s", pname.c_str());
    if (system(cmd) == 0) {
        cout << "Process killed successfully!" << endl;
    } else {
        cout << "Failed to kill process." << endl;
    }
    #endif
}

void execCommand(const vector<string>& args) {
    #ifdef __linux__
    string filename = args[1];
    filename.erase(0, filename.find_first_not_of(" \t")); 
    filename.erase(filename.find_last_not_of(" \t") + 1);

    string compileCommand = "g++ -o " + filename.substr(0, filename.find_last_of('.')) + " " + filename;
    system(compileCommand.c_str());
    string execCommand = "./" + filename.substr(0, filename.find_last_of('.'));
    system(execCommand.c_str());
    #else
    cout << "Not supported!" << endl;
    #endif
}

void bashCommand(const vector<string>& args) {
    #ifdef __linux__
    string filename = args[1];
    filename.erase(0, filename.find_first_not_of(" \t")); 
    filename.erase(filename.find_last_not_of(" \t") + 1); 
    string command = "chmod 744 " + filename;
    system(command.c_str());
    command = "./" + filename ;
    system(command.c_str());
    cout << "Script executed successfully!" << endl;
    #endif  
}

void netstatCommand(const vector<string>& args) {
    #ifdef __linux__
    string command = "netstat -an";
    system(command.c_str());
    #else
    cout << "Not supported!" << endl;
    #endif
}

void exitCommand(const vector<string>& args) {
    cout << "[SESSION TERMINATED]" << endl;
    cout<< "Logging out from .nyan" <<endl;
    sleep(1);
    exit(1);
}

void clsCommand(const vector<string>& args) {
    cout << "\x1B[2J\x1B[H";
}


void loginAsRoot() {
    loggedInUsers["root"] = true;
    cout << "Logged in as root" << endl;
    loginOrder.push_back("root");
}

void addRoot() {
    allUsers.push_back("root");
}

void getCurrentUser() {
    cout << "You are logged in as " << loginOrder.back() << endl;
}

void loginUser(const string& username) {
    if (loggedInUsers.find(username) != loggedInUsers.end()) {
        cout << "User already logged in." << endl;
        return;
    }
    loggedInUsers[username] = true;
    currentUser = username;
    cout << "Logged in as " << username << endl;
}

void loginCommand(const vector<string>& args) {
    cout << "Enter username: ";
    string username;
    cin >> username;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    loginUser(username);
    loginOrder.push_back(username);
    allUsers.push_back(username);
}

void logoutCommand(const vector<string>& args) {
    if (loggedInUsers.size() == 1) {
        cout << "Root user cannot be logged out" << endl;
        return;
    }
    string lastUser;
    for (const auto& pair : loggedInUsers) {
        if (pair.first != "root") {
            lastUser = pair.first;
        }
    }
    loggedInUsers.erase(lastUser);
    loginOrder.remove(lastUser);
    cout << "Log out successful" << endl;
}

void whoamiCommand(const vector<string>& args) {
    getCurrentUser();
}

void activeCommand(const vector<string>& args) {
    cout << "Logged in users:" << endl;
    for (const auto& user : loggedInUsers) {
        if (user.first == "root")  {
            cout << user.first << endl;
        }
    }
    for (const auto& user : loggedInUsers) {
    if (user.first != "root") {
        cout << user.first << endl;
    }
}
}

void usersCommand(const vector<string>& arg) {
    cout << "All users:" << endl;
    for (const auto& user : allUsers) {
        cout << user << endl;
    }
} 

string quote(const string& str) {
    string quoted;
    quoted.push_back('"');
    for (char c : str) {
        if (c == '"' || c == '\\') {
            quoted.push_back('\\');
        }
        quoted.push_back(c);
    }
    quoted.push_back('"');
    return quoted;
}

void playCommand(const vector<string>& args) {
    #ifdef __linux__
    string filename;
    cout << "Enter the audio file name: " << endl;;
    getline(cin, filename);
    string command = "mplayer " + quote(filename);
    cout << "Executing command: " << command << endl;
    int status = system(command.c_str());
    if (status == -1) {
        cerr << "Error playing audio file: " << endl;
        return;
    }
    if (WEXITSTATUS(status) != 0) {
        cerr << "Error playing audio file: " << strerror(WEXITSTATUS(status)) << endl;
        return;
    }
    cout << "Command executed successfully." << endl;
    #else
    cout << "Not supported!" << endl;
    #endif
}

void streamCommand(const vector<string>& args) {
    #ifdef __linux__
    string filename;
    cout << "Enter the video file name: " << endl;;
    getline(cin, filename);
    string command = "mplayer " + quote(filename);
    cout << "Executing command: " << command << endl;
    int status = system(command.c_str());
    if (status == -1) {
        cerr << "Error playing audio file: " << endl;
        return;
    }
    if (WEXITSTATUS(status) != 0) {
        cerr << "Error playing video file: " << strerror(WEXITSTATUS(status)) << endl;
        return;
    }
    cout << "Command executed successfully." << endl;
    #else
    cout << "Not supported!" << endl;
    #endif   
}

void topCommand(const vector<string>& args) {
    #ifdef __linux__
    string command = "top";
    system(command.c_str());
    #endif
}

void rsyncCommand(const vector<string>& args) {
    #ifdef __linux__
    string source = args[1];
    source.erase(0, source.find_first_not_of(" \t")); 
    source.erase(source.find_last_not_of(" \t") + 1);
    string destination = args[2];
    destination.erase(0, destination.find_first_not_of(" \t")); 
    destination.erase(destination.find_last_not_of(" \t") + 1);
    cout << source << endl;
    cout << destination << endl;
    string command = "rsync -av " + source + " " + destination;
    if (system(command.c_str()) != 0) {
        cout << "Error: Unable to run command" << endl;
    }
    cout << "Command executed sucessfully" << endl;
    #endif
}

void sysinfoCommand(const vector<string>& args) {
    #ifdef __linux__
    string command = "xsysinfo";
    if (system(command.c_str()) != 0) {
        cout << "Error: Unable to run command" << endl;
    }
    cout << "Command executed sucessfully" << endl;
    #endif

}

void serviceCommand(const vector<string>& args) {
    #ifdef __linux__
    if (args.size() < 3)
    {
        cout << "Error: service expects only three arguements";
        return;
    }
    string name = args[1];
    name.erase(0, name.find_first_not_of(" \t")); 
    name.erase(name.find_last_not_of(" \t") + 1);
    string action = args[2];
    action.erase(0, action.find_first_not_of(" \t")); 
    action.erase(action.find_last_not_of(" \t") + 1); 
    string command = "service " + name + " " + action;
    system(command.c_str());  
    cout << "Command execution success!" << endl;
    #endif
}

void writeCommand(const vector<string>& args) {
    #ifdef __linux__
    string source = args[1];
    source.erase(0, source.find_first_not_of(" \t")); 
    source.erase(source.find_last_not_of(" \t") + 1);

    string fileExtension = ".odt";
    size_t dotPos = source.find_last_of('.');
    if (dotPos == string::npos) {
        source += fileExtension;
    } else {
        source.replace(dotPos, fileExtension.size(), fileExtension);
    }

    ofstream file(source);
    if (!file) {
        cout << "Error: Unable to create file " << source << endl;
        return;
    }

    string command = "libreoffice --writer \"" + source + "\"";
    system(command.c_str());
    #endif
}

void excelCommand(const vector<string>& args) {
    #ifdef __linux__
    string source = args[1];
    source.erase(0, source.find_first_not_of(" \t")); 
    source.erase(source.find_last_not_of(" \t") + 1);

    string fileExtension = ".ods";
    size_t dotPos = source.find_last_of('.');
    if (dotPos == string::npos) {
        source += fileExtension;
    } else {
        source.replace(dotPos, fileExtension.size(), fileExtension);
    }
    ofstream file(source);
    if (!file) {
        cout << "Error: Unable to create file " << source << endl;
        return;
    }

    string command = "libreoffice --calc \"" + source + "\"";
    system(command.c_str());   
    #endif
}

void presentationCommand(const vector<string>& args) {
    #ifdef __linux__
    string source = args[1];
    source.erase(0, source.find_first_not_of(" \t")); 
    source.erase(source.find_last_not_of(" \t") + 1);

    string fileExtension = ".odp";
    size_t dotPos = source.find_last_of('.');
    if (dotPos == string::npos) {
        source += fileExtension;
    } else {
        source.replace(dotPos, fileExtension.size(), fileExtension);
    }
    ofstream file(source);
    if (!file) {
        cout << "Error: Unable to create file " << source << endl;
        return;
    }

    string command = "libreoffice --impress \"" + source + "\"";
    system(command.c_str());
    #endif    
}

void mathCommand(const vector<string>& args) {
    #ifdef __linux__
    string command = "libreoffice --math";
    system(command.c_str());
    #endif
}

void watchCommand(const vector<string>& args) {
    #ifdef __linux__
    string source = args[1];
    source.erase(0, source.find_first_not_of(" \t")); 
    source.erase(source.find_last_not_of(" \t") + 1); 
    string command = "watch " + source;
    system(command.c_str());
    cout << "Execution successful" << endl;
    #endif
}

void ifstatCommand(const vector<string>& args) {
    #ifdef __linux__
    string command = "ifstat";
    system(command.c_str());
    cout << "Command executed successfully" << endl;
    #endif   
}

void htopCommand(const vector<string>& args) {
    #ifdef __linux__
    string command = "htop";
    system(command.c_str());
    cout << "Command executed successfully" << endl;
    #endif   
}

void shutdownCommand(const vector<string>& args) {
    cout << "Shutting down the system" << endl;
    system("shutdown -h now");
}

void mountCommand(const vector<string>& args) {
    #ifdef __linux__
    string source = args[1];
    source.erase(0, source.find_first_not_of(" \t")); 
    source.erase(source.find_last_not_of(" \t") + 1);
    string dest = args[2];
    dest.erase(0, dest.find_first_not_of(" \t")); 
    dest.erase(dest.find_last_not_of(" \t") + 1); 
    if (args.size() < 3)
    {
        cout << "Error: Unable to execute" << endl;
        return;
    }  
    string command = "mount " + source + " " + dest;
    system(command.c_str()); 
    #endif
}

void umountCommand(const vector<string>& args) {
    #ifdef __linux__
    string source = args[1];
    source.erase(0, source.find_first_not_of(" \t")); 
    source.erase(source.find_last_not_of(" \t") + 1);
    if (args.size() < 2)
    {
        cout << "Error: Unable to execute!" << endl;
        return;
    }  
    string command = "mount " + source;
    system(command.c_str()); 
    #endif
}

void tarCommand(const vector<string>& args) {
    #ifdef __linux__
    string argString;
    for (const auto& arg : args) {
        if (!argString.empty()) {
            argString += " ";
        }
        argString += arg;
    }
    string source = args[0];
    source.erase(0, source.find_first_not_of(" \t")); 
    source.erase(source.find_last_not_of(" \t") + 1);
    string cmd = args[1];
    cmd.erase(0, cmd.find_first_not_of(" \t")); 
    cmd.erase(cmd.find_last_not_of(" \t") + 1);
    if (source == "tar") {
        if (args.size() == 3) {
            string cmd1 = args[2];
            cmd1.erase(0, cmd1.find_first_not_of(" \t")); 
            cmd1.erase(cmd1.find_last_not_of(" \t") + 1);
            string command;
            if (cmd == "-xvf") {
                command = source + " " + cmd + " " + cmd1;
                system(command.c_str()); 
            }
            else if (cmd == "-tvf") {
                command = source + " " + cmd + " " + cmd1;
                system(command.c_str());
            }
            else if (cmd == "-cvf") {
                command = source + " " + cmd + " " + cmd1;
                system(command.c_str());
            }
            else {
                cout << "Incorrect syntax" << endl;
            }
        }
        else if (args.size() > 3)
        {
            if (cmd == "-cvf") {
                system(argString.c_str());
            }
            else {
                cout << "INCORRECT SYNTAX" << endl;
            }
        }
        else {
            cout << "INCORRECT SYNTAX" << endl;
        }
    }
    else {
        cout << "Error: Incorrect command" << endl;
    }
    #endif
}

void tailCommand(const vector<string>& args) {
    #ifdef __linux__
    string argString;
    for (const auto& arg : args) {
        if (!argString.empty()) {
            argString += " ";
        }
        argString += arg;
    }
    string source = args[0];
    source.erase(0, source.find_first_not_of(" \t")); 
    source.erase(source.find_last_not_of(" \t") + 1);
    string cmd = args[1];
    cmd.erase(0, cmd.find_first_not_of(" \t")); 
    cmd.erase(cmd.find_last_not_of(" \t") + 1);
    if (source == "tail") {
        if (cmd == "-f") {
            system(argString.c_str());
        }
        else if (cmd == "-n") {
            system(argString.c_str());
        }
        else if (cmd == "-q") {
            system(argString.c_str());
        }
        else {
            ifstream file(cmd);
            if (!file) {
                cout << "Error: File does not exist!" << endl;
            }
            else {
                system(argString.c_str());
            }
        }
    }
    #endif
}

void sshCommand(const vector<string>& args) {
    #ifdef __linux__
    if (args.size() < 2)
    {
        cout << "Error: ssh command only expects one arguement" << endl;
        return;
    }
    string source = args[1];
    source.erase(0, source.find_first_not_of(" \t")); 
    source.erase(source.find_last_not_of(" \t") + 1);
    string command = "ssh " + source;
    for (size_t i = 2; i < args.size(); ++i)
    {
        command += " " + args[i];
    }
    system(command.c_str());
    cout << "COMMAND EXECUTION SUCCESS!" << endl;
    #endif
}

void nmapCommand(const vector<string>& args) {
    #ifdef __linux__
    if (args.size() < 2)
    {
        cout << "Error: nmap command only expects one arguement" << endl;
        return;
    }
    string source = args[1];
    source.erase(0, source.find_first_not_of(" \t")); 
    source.erase(source.find_last_not_of(" \t") + 1);
    string command = "nmap " + source;
    for (size_t i = 2; i < args.size(); ++i)
    {
        command += " " + args[i];
    }
    system(command.c_str());
    cout << "COMMAND EXECUTION SUCCESS!" << endl;
    #endif
}

void lnCommand(const vector<string>& args) {
    #ifdef __linux__
    if (args.size() < 3)
    {
        cout << "Error: Unable to execute" << endl;
        return;
    }
    string source = args[1];
    source.erase(0, source.find_first_not_of(" \t")); 
    source.erase(source.find_last_not_of(" \t") + 1);
    string cmd = args[2];
    cmd.erase(0, cmd.find_first_not_of(" \t")); 
    cmd.erase(cmd.find_last_not_of(" \t") + 1);
    string command = "ln -s " + source + " " + cmd;
    system(command.c_str());
    cout << "Executed successfully" << endl;
    #endif
}

void sedCommand(const vector<string>& args) {
    #ifdef __linux__
    string source = args[0];
    source.erase(0, source.find_first_not_of(" \t")); 
    source.erase(source.find_last_not_of(" \t") + 1);
    string cmd = args[1];
    cmd.erase(0, cmd.find_first_not_of(" \t")); 
    cmd.erase(cmd.find_last_not_of(" \t") + 1);
    if (args.size() < 3)
    {
        cout << "Error: Unable to execute" << endl;
        return;
    }
    string command = "sed '" + source + "' " + cmd;
    system(command.c_str());
    #endif
}

void iptablesCommand(const vector<string>& args) {
    #ifdef __linux__
    if (args.size() < 2)
    {
        cout << "Error: Unable to execute"<<endl;
        return;
    }
    string source = args[1];
    source.erase(0, source.find_first_not_of(" \t")); 
    source.erase(source.find_last_not_of(" \t") + 1); 
    string command = "iptables " + source;
    system(command.c_str());
    cout << "Execution successful" << endl;
    #endif 
}

void initCommand(const vector<string>& args) {
    #ifdef __linux__
    if (args.size() < 2)
    {
        cout << "Error: Unable to execute";
        return;
    }
    string source = args[1];
    source.erase(0, source.find_first_not_of(" \t")); 
    source.erase(source.find_last_not_of(" \t") + 1); 
    string command = "init " + source;
    system(command.c_str());
    #endif
}

void ifconfigCommand(const vector<string>& args) {
    #ifdef _WIN32
        system("ipconfig");
    #elif __linux__
        system("ifconfig");
    #endif
}

void manCommand(const vector<string>& args) {
    static const map<string, string> commandHelp = {
        {"pwd", "NAME  pwd - print name of current/working directory\nSYNTAX  pwd"},
        {"whoami", "NAME  who - show who is logged on\nSYNTAX  whoami"},
        {"ls", "NAME ls - list directory contents\nSYNTAX  ls"},
        {"ls -l", "NAME   ls -l - use a long listing format\nSYNTAX  ls -l"},
        {"cd", "NAME  cd - enter into a directory\nSYNTAX  cd"},
        {"mkdir", "NAME  mkdir - make new directories\nSYNTAX  mkdir [filename]"},
        {"rmdir", "NAME  rmdir - deletes empty as well non empty directories\nSYNTAX  rmdir [filename]"},
        {"touch", "NAME  touch - creates empty files\nSYNTAX  touch [filename]"},
        {"exit", "NAME  exit - exits the shell\nSYNTAX  exit"},
        {"cat", "NAME  cat - concatenate files and print on the standard output\nSYNTAX  cat [filename]"},
        {"echo", "NAME  echo - display a line of text\nSYNTAX  echo [text]"},
        {"cp", "NAME  cp - copy files and directories\nSYNTAX  cp [sourcefile] [destinationfile]"},
        {"date", "NAME  date - print or set the system date and time\nSYNTAX  date"},
        {"diff", "NAME  diff - compare files line by line\nSYNTAX  diff [filename]"},
        {"rm", "NAME  rm - remove files\nSYNTAX  rm [filename]"},
        {"cls", "NAME  cls - clears screen\nSYNTAX  cls"},
        {"clear", "NAME  clear - clears screen\nSYNTAX  clear"},
        {"gzip", "NAME  gzip - compress or expand files\nSYNTAX  gzip [filename]"},
        {"history", "NAME  history - GNU History Library\nSYNTAX  history"},
        {"chmod", "NAME  chmod - change file mode bits\nSYNTAX chmod [permission octal digits] [filename]"},
        {"hexdump", "NAME  hexdump - display file contents in octal\nSYNTAX  hexdump [filename]"},
        {"ps", "NAME  ps - report a snapshot of the current processes\nSYNTAX  ps"},
        {"wc", "NAME  wc - print newline, word, and byte counts for each file\nSYNTAX  wc [filename]"},
        {"uname", "NAME  uname - print system information\nSYNTAX  uname"},
        {"uptime", "NAME  uptime - Tell how long the system has been running\nSYNTAX  uptime"},
        {"free", "NAME  free - Display amount of free and used memory in the system\nSYNTAX  free"},
        {"df", "NAME  df - report file system disk space usage\nSYNTAX  df"},
        {"uniq", "NAME  uniq - report or omit repeated lines\nSYNTAX  uniq [filename]"},
        {"exec", "NAME  exec - compiles and runs code written only in cpp\nSYNTAX  exec [filename]"},
        {"bash", "NAME  bash - compiles and runs code written in bash\nSYNTAX  bash [filename]"},
        {"env", "NAME  env - run a program in a modified environment\nSYNTAX  env"},
        {"du" , "NAME   du - estimate file space usage\nSYNTAX du"},
        {"python" , "NAME   python - compiles and runs python scripts\nSYNTAX python [filename]"},
        {"git" , "NAME   git - git commands for version control\nSYNTAX git"},
        {"vim" , "NAME   vim - opens files in vim editor\nSYNTAX vim [filename]"},
        {"nano" , "NAME   nano - opens files in nano editor\nSYNTAX nano [filename]"},
        {"neofetch" , "NAME   neofetch - displays system information accompanied by ascii art\nSYNTAX neofetch"},
        {"screen" , "NAME   screen - opens another screen\nSYNTAX screen"},
        {"kill" , "NAME   kill - kills the process\nSYNTAX kill"},
        {"ssh" , "NAME   ssh - connects to a host via SSH\nSYNTAX ssh [user@hostname] [options]"},
        {"nmap" , "NAME   nmap - network exploration tool and security scanner\nSYNTAX nmap [options]"},
        {"tcpdump" , "NAME   tcpdump - works as a command-line packet analyzer\nSYNTAX tcpdump [options]"},
        {"service" , "NAME   service - manages system services\nSYNTAX service [service name] [start|stop|restart]"},
        {"play" , "NAME   play - plays music directly from terminal\nSYNTAX play"},
        {"stream" , "NAME   stream - plays video directly from terminal\nSYNTAX video"},
        {"top" , "NAME   top - displays real-time system resource usage\nSYNTAX top"},
        {"htop" , "NAME   htop - provides detailed system performance information\nSYNTAX htop"},
        {"ln" , "NAME   ln - creates a symbolic link\nSYNTAX ln [target] [linkname]"},
        {"sed" , "NAME   sed - performs text transformations\nSYNTAX sed [expression] [file]"},
        {"http" , "NAME   http - opens a simple http server\nSYNTAX http"},
        {"ifconfig" , "NAME   ifconfig - lists all network interface configurations\nSYNTAX ifconfig"},
        {"ifstat" , "NAME   ifstat - displays network interface statistics\nSYNTAX ifstat"},
        {"iptables" , "NAME   iptables - administrates IP packet filter rules\nSYNTAX iptables [filename]"},
        {"shutdown" , "NAME   shutdown - shutdowns your system\nSYNTAX shutdown"},
        {"sysinfo" , "NAME  sysinfo - displays system information\nSYNTAX sysinfo"},
        {"firefox" , "NAME  firefox - opens firefox from command line\nSYNTAX firefox"},
        {"about" , "NAME   about - provides information about the shell\nSYNTAX about"},
        {"math" , "NAME   math - assists any mathematical operation\nSYNTAX math"},
        {"traceroute" , "NAME   traceroute - traces the route packets take to a network host\nSYNTAX traceroute [host]"},
        {"init" , "NAME   init - changes the runlevel of the system\nSYNTAX init [runlevel]"},
        {"mount" , "NAME   mount - mounts filesystems\nSYNTAX mount [source] [destination]"},
        {"umount" , "NAME   unmount - unmounts filesystems\nSYNTAX umount [source]"},
        {"all" , "NAME   all - lists all services, whether they're running or not, with a + next to the ones that are.\nSYNTAX all"},
        {"inotify" , "NAME   inotify - monitor the specified directory and its subdirectories for create, delete, modify, and other events\nSYNTAX inotify [filename]"},
        {"rsync" , "NAME   rsync - syncs files and directories between two locations\nSYNTAX rsync [source] [destination]"},
        {"write" , "NAME   write - writes in a Microsoft Word compatible\nSYNTAX write [filename]"},
        {"tar" , "NAME   tar - manages archives for backup and restoration\nSYNTAX tar [command] [filename]"},
        {"tail" , "NAME   tail - follows the tail of a file (real-time update)\nSYNTAX tail [command] [filename]"},
        {"excel" , "NAME   excel - makes spreadsheets in a Microsoft Excel compatible\nSYNTAX excel [filename]"},
        {"presentation" , "NAME   presentation - makes slides in a Microsoft Powerpoint compatible\nSYNTAX presentation [filename]"},
        {"netstat" , "NAME   netstat - display information about active network connections, routing tables, and interface statistics\nSYNTAX netstat"},
        {"man", "NAME  man - an interface to the system reference manuals\nSYNTAX  man"}
    };

    string cmd = args[1];
    cmd.erase(0, cmd.find_first_not_of(" \t"));
    cmd.erase(cmd.find_last_not_of(" \t") + 1);

    auto it = commandHelp.find(cmd);
    if (it != commandHelp.end()) {
        cout << it->second << endl;
    } else {
        cout << "Error: unknown command" << endl;
    }
    
}

void neofetchCommand() {
    #ifdef __linux__
    cout << "Running installation for neofetch" << endl;
    cout << " " <<endl;
    string command = "sudo apt install neofetch";
    if (system(command.c_str()) != 0) {
        cout << "Error installing neofetch" << endl;
    }
    cout << "NEOFETCH INSTALLATION SUCCESS" << endl;
    #else
    cout << "Not supported here!" << endl;
    #endif
}

void tracerouteInstall() {
    #ifdef __linux__
    string command = "sudo apt install inetutils-traceroute";
    if (system(command.c_str()) != 0) {
        cout << "Error installing traceroute" << endl;
    }
    cout << "TRACEROUTE INSTALLATION SUCCESS" << endl;
    #endif
}

void tracerouteCommand(const vector<string>& args) {
    #ifdef __linux__
    if (args.size() < 2)
    {
        cout << "traceroute command expects only two arguements" << endl;
        return;
    }
    string cmd = args[1];
    cmd.erase(0, cmd.find_first_not_of(" \t"));
    cmd.erase(cmd.find_last_not_of(" \t") + 1);
    string command = "traceroute " + cmd;
    system(command.c_str());
    #endif
}

void netstatInstall() {
    #ifdef __linux__
    string command = "sudo apt install net-tools";
    system(command.c_str());
    cout << "INSTALLATION SUCCESS" << endl;
    #endif
}

void httpSnapInstall() {
    string command = "sudo snap install http";
    system(command.c_str());
    cout << "INSTALLATION SUCCESS" << endl;
}

void httpCommand(const vector<string>& args) {
    #ifdef __linux__
    string cmd = args[1];
    cmd.erase(0, cmd.find_first_not_of(" \t"));
    cmd.erase(cmd.find_last_not_of(" \t") + 1);
    string command = "http " + cmd;
    if (system(command.c_str()) != 0) {
        cout << "Error: Unable to execute. Check if there is https before the url" << endl;
    }
    cout << "Command execution success" << endl;
    #endif
}

void neofetchDisplay() {
    #ifdef __linux__
    string ascii_art;
    ifstream file("princess-ascii.txt");
    getline(file, ascii_art, '\0'); 
    file.close();
    setenv("USER", ".nyan", 1);
    string command = "neofetch --ascii \"" + ascii_art + "\"";
    system(command.c_str());
    #endif
}

void neofetchTerminalDisplay(const vector<string>& args) {
    #ifdef __linux__
    string ascii_art;
    ifstream file("princess-ascii.txt");
    getline(file, ascii_art, '\0'); 
    file.close();
    setenv("USER", ".nyan", 1);
    string command = "neofetch --ascii \"" + ascii_art + "\"";
    system(command.c_str());
    #endif
}

void firefoxInstall() {
    #ifdef __linux__
    string command;
    command = "sudo apt install firefox";
    system(command.c_str());
    cout << "FIREFOX INSTALLATION SUCCESS" << endl;
    #endif
}

void firefoxCommand(const vector<string>& args) {
    string command = "firefox";
    system(command.c_str());
}

void inotifyCommand(const vector<string>& args) {
    #ifdef __linux__
    string cmd = args[1];
    cmd.erase(0, cmd.find_first_not_of(" \t"));
    cmd.erase(cmd.find_last_not_of(" \t") + 1);
    string command = "inotifywait -m -r " + cmd;
    system(command.c_str());
    #endif
}

void pythonInstall() {
#ifdef __linux__
    cout << "PYTHON INSTALLATION IN PROGRESS" << std::endl;
    string installCommand = "sudo apt install python3";
    if (system(installCommand.c_str()) != 0) {
        cout << "Error installing Python!" << endl;
    } else {
        cout << "PYTHON INSTALLATION SUCCESSFUL!" << endl;
    }
#endif
}

void pythonCommand(const vector<string>& args) {
#ifdef __linux__
    if (args.size() < 2) {
        cout << "Usage: pythonCommand <filename>" << endl;
        return;
    }

    string filename = args[1];
    filename.erase(0, filename.find_first_not_of(" \t"));
    filename.erase(filename.find_last_not_of(" \t") + 1);

    string compileCommand = "python3 " + filename;
    if (system(compileCommand.c_str()) != 0) {
        cout << "Error running Python script!" << endl;
    }
#else
    cout << "Python command not supported on this platform!" << endl;
#endif
}

void gitInstall() {
    #ifdef __linux__
    cout << "GIT INSTALLATION IN PROGRESS" << endl;
    string command = "sudo apt-get install git";
    system(command.c_str());
    cout << "GIT INSTALLATION SUCCESS" << endl;
    #endif
}

void gitCommand(const vector<string>& args) {
    #ifdef __linux__
    int numArgs = args.size();
    string argString;
    for (const auto& arg : args) {
        if (!argString.empty()) {
            argString += " ";
        }
        argString += arg;
    }
    string cmd = args[0];
    if (cmd == "git") {
        if (numArgs == 1) {
            string command = "git --version";
            system(command.c_str());
            string command1 = "git --help";
            system(command1.c_str());  
        }
        else {
            cout << argString << endl;
            system(argString.c_str());
        }
    }
    #else
    cout << "Not supported" << endl;
    #endif
}

void tcpdumpCommand(const vector<string>& args) {
    #ifdef __linux__
    string argString;
    for (const auto& arg : args) {
        if (!argString.empty()) {
            argString += " ";
        }
        argString += arg;
    }
    system(argString.c_str());
    #endif
}

void libreOfficeInstall() {
    string command = "sudo apt install libreoffice";
    system(command.c_str());
}

void xsysinfo() {
    string command = "sudo apt install xsysinfo";
    system(command.c_str());
    cout << "SUCCESS!" << endl;
}

void musicInstall() {
    string command = "sudo add-apt-repository universe";
    system(command.c_str());
    command = "sudo apt update";
    system(command.c_str());
    command = "sudo apt install mplayer mplayer-gui";
    system(command.c_str());
    cout << "SUCCESS!" << endl;
}

void inotifytoolsInstall() {
    string command = "sudo apt install inotify-tools";
    system(command.c_str());
    cout << " INOTIFY SUCCESS" << endl;
}

void aptInstall() {
    string command = "sudo apt-get update && sudo apt-get install -y apt";
    cout << "APT INSTALLATION IN PROGRESS" << endl;
    cout << " " << endl;
    if (system(command.c_str()) != 0) {
        cout << "Error installing apt" << endl;
    }
    cout << "APT INSTALLATION SUCCESS" << endl;
    cout << " " << endl;
    system("sudo apt update");
    cout << " " <<endl;
    cout << "APT UPDATE SUCCESS" << endl;
    cout << " " << endl;
}

void run(string filename) {
    #ifdef __linux__
    string compileCommand = "g++ -o " + filename.substr(0, filename.find_last_of('.')) + " " + filename;
    system(compileCommand.c_str());
    string execCommand = "./" + filename.substr(0, filename.find_last_of('.'));
    system(execCommand.c_str());
    #else
    cout << "Not supported!" << endl;
    #endif
}

void screenCommand(const vector<string>& args) {
    cout << "Press Enter to enter a new screen" << endl;
    cin.get();
    cout << "\x1B[2J\x1B[H";
    run("main.cpp");
}

void aboutCommand(const vector<string>& args) {
    cout << "A shell written in cpp language and is powered by apt package manager" << endl;
    cout << "Suitable for cross platforms, but not entirely" << endl;
    cout << "More suited for Linux" << endl; 
    cout << "Supported by an inbuilt firefox browser" << endl;
}

int countWords(const string& input) {
    int count = 0;
    string word;

    for (char c : input) {
        if (c == ' ') {
            if (!word.empty()) {
                count++;
                word.clear();
            }
        } else {
            word += c;
        }
    }

    if (!word.empty()) {
        count++;
    }
    return count;
}

int main() {
    bool commandFound = false;

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        currentWorkingDirectory = cwd;
    } else {
        cout << "Error: Unable to get current working directory" << endl;
        return 1;
    }
    commandRegister["pwd"] = pwdCommand;
    commandRegister["whoami"] = whoamiCommand;
    commandRegister["active"] = activeCommand;
    commandRegister["logout"] = logoutCommand;
    commandRegister["users"] = usersCommand;
    commandRegister["ls"] = lsLCommand;
    commandRegister["ls -l"] = lsLCommand;
    commandRegister["cd"] = cdCommand;
    commandRegister["mkdir"] = mkdirCommand;
    commandRegister["rmdir"] = rmdirCommand;
    commandRegister["touch"] = touchCommand;
    commandRegister["exit"] = exitCommand;
    commandRegister["cat"] = catCommand;
    commandRegister["echo"] = echoCommand;
    commandRegister["cp"] = cpCommand;
    commandRegister["date"] = dateCommand;
    commandRegister["diff"] = diffCommand;
    commandRegister["vim"] = vimCommand;
    commandRegister["rm"] = rmCommand;
    commandRegister["cls"] = clsCommand;
    commandRegister["clear"] = clsCommand;
    commandRegister["gzip"] = gzipCommand;
    commandRegister["cd .."] = cdbackCommand;
    commandRegister["history"] = displayHistory;
    commandRegister["chmod"] = chmodCommand;
    commandRegister["hexdump"] = hexdumpCommand;
    commandRegister["ps"] = psCommand;
    commandRegister["wc"] = wcCommand;
    commandRegister["uname"] = unameCommand;
    commandRegister["uptime"] = uptimeCommand;
    commandRegister["free"] = freeCommand;
    commandRegister["df"] = dfCommand;
    commandRegister["uniq"] = uniqCommand;
    commandRegister["man"] = manCommand;
    commandRegister["wget"] = wgetCommand;
    commandRegister["mv"] = mvCommand;
    commandRegister["exec"] = execCommand;
    commandRegister["env"] = envCommand;
    commandRegister["du"] = duCommand;
    commandRegister["python"] = pythonCommand;
    commandRegister["git"] = gitCommand;
    commandRegister["nano"] = nanoCommand;
    commandRegister["neofetch"] = neofetchTerminalDisplay;
    commandRegister["screen"] = screenCommand;
    commandRegister["play"] = playCommand;
    commandRegister["stream"] = streamCommand;
    commandRegister["netstat"] = netstatCommand;
    commandRegister["top"] = topCommand;
    commandRegister["shutdown"] = shutdownCommand;
    commandRegister["write"] = writeCommand;
    commandRegister["presentation"] = presentationCommand;
    commandRegister["excel"] = excelCommand;
    commandRegister["math"] = mathCommand;
    commandRegister["about"] = aboutCommand;
    commandRegister["kill"] = killCommand;
    commandRegister["rsync"] = rsyncCommand;
    commandRegister["bash"] = bashCommand;
    commandRegister["ifconfig"] = ifconfigCommand;
    commandRegister["sysinfo"] = sysinfoCommand;
    commandRegister["less"] = lessCommand;
    commandRegister["inotify"] = inotifyCommand;
    commandRegister["iptables"] = iptablesCommand;
    commandRegister["init"] = initCommand;
    commandRegister["firefox"] = firefoxCommand;
    commandRegister["ifstat"] = ifstatCommand;
    commandRegister["htop"] = htopCommand;
    commandRegister["http"] = httpCommand;
    commandRegister["watch"] = watchCommand;
    commandRegister["umount"] = umountCommand;
    commandRegister["mount"] = mountCommand;
    commandRegister["all"] = allCommand;
    commandRegister["service"] = serviceCommand;
    commandRegister["traceroute"] = tracerouteCommand;
    commandRegister["tar"] = tarCommand;
    commandRegister["tail"] = tailCommand;
    commandRegister["sed"] = sedCommand;
    commandRegister["ln"] = lnCommand;
    commandRegister["tcpdump"] = tcpdumpCommand;
    commandRegister["ssh"] = sshCommand;
    commandRegister["nmap"] = nmapCommand;

    #ifdef __linux__
    aptInstall();
    pythonInstall();
    musicInstall();
    xsysinfo();
    gitInstall();
    libreOfficeInstall();
    httpSnapInstall();
    tracerouteInstall();
    inotifytoolsInstall();
    firefoxInstall();
    netstatInstall();
    cout << "\x1B[2J\x1B[H";
    neofetchCommand();
    #endif

    #ifdef __linux__
    neofetchDisplay();
    #endif
    cout << "WELCOME TO .nyan" << endl;
    cout << " " << endl;
    loginAsRoot();
    cout << " " << endl;
    addRoot();

    cout << endl;

    string input;
    int count = countWords(input);
    while (true) {
        #ifdef __linux__
        setColor(35);
        #endif
        cout << ".nyan:~$ ";
        #ifdef __linux__
        setColor(37);
        #endif
        getline(cin, input);

        vector<string> tokens = tokenize(input);

        if (tokens[0] == "login"){
            loginCommand(tokens);
        }   
        else {
            if (commandRegister.find(tokens[0]) != commandRegister.end()) {
                commandRegister[tokens[0]](tokens);
            } else {
                if (find(sudoCommands.begin(), sudoCommands.end(), tokens[0]) != sudoCommands.end()) {
                    cout << "RUNNING COMMAND!" << endl;
                    if (count == 1) {
                        system(tokens[0].c_str());
                    }
                    else {
                        system(input.c_str());
                    }
                } else {
                    if (tokens[0]!= "sudo")
                    {
                        string aptCommand = "sudo apt install " + tokens[0];
                        if (system(aptCommand.c_str()) == 0) {
                            cout << "COMMAND INSTALLATION SUCCESS!" << endl;
                            cout << "RUNNING COMMAND!" << endl;
                            system(tokens[0].c_str());
                            sudoCommands.push_back(tokens[0]);
                        } else {
                            cout << "COMMAND CANNOT BE INSTALLED!" << endl;  
                        }
                    }
                }
            }
        }

        if (tokens[0] == "sudo") {
            cout << "Prompt only the command to install" << endl;
        }
    }
    return 0;
}
