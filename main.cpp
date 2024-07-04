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
#include<list>
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

void lsCommand() {
    DIR *dir;
    struct dirent *ent;
    dir = opendir(".");
    if (dir!= NULL) {
        while ((ent = readdir(dir))!= NULL) {
            cout << ent->d_name << endl;
        }
        closedir(dir);
    } else {
        cout << "Error: Unable to read directory" << endl;
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
    system(cmd); 
    cout << "Directory created successfully!" << endl;
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
    if (args.size()!= 2) {
        cout << "Error: vim command expects only one argument" << endl;
    }
    string fileName = args[1];
    string command = "vim" + fileName;
    system(command.c_str());
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

string basename(const string& path) {
    size_t pos = path.find_last_of('/');
    if (pos!= string::npos) {
        return path.substr(pos + 1);
    }
    return path;
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
#include <stdlib.h>
clock_t start = clock();

void uptimeCommand(const vector<string>& args) {
    time_t now;
    #ifdef _WIN32
    time(&now);
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    int elapsed_minutes = static_cast<int>(elapsed / 60);
    int elapsed_hours = static_cast<int>(elapsed_minutes / 60);
    elapsed_minutes %= 60;
    printf("%02d:%02d:%02d ", (localtime(&now))->tm_hour, (localtime(&now))->tm_min, (localtime(&now))->tm_sec);
    cout << "up " << elapsed_hours << ":" << elapsed_minutes << ","  << " " << "user" << "," <<endl;   
    #elif __linux__
    system("uptime");
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

void exitCommand(const vector<string>& args) {
    cout<< "Logging out from .nyan" <<endl;
    exit(1);
}

void clsCommand(const vector<string>& args) {
    cout << "\x1B[2J\x1B[H";
}


void loginAsRoot() {
    loggedInUsers["root"] = true;
    cout << "Logged in as root" << endl;
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


// void manCommand(const vector<string>& args) {
//     string cmd = args[1];
//     string nextcmd=args[2];
//     cmd.erase(0, cmd.find_first_not_of(" \t")); 
//     cmd.erase(cmd.find_last_not_of(" \t") + 1); 
//     nextcmd.erase(0, nextcmd.find_first_not_of(" \t")); 
//     nextcmd.erase(nextcmd.find_last_not_of(" \t") + 1); 
//     if (cmd == "pwd") {
//         cout << "NAME  pwd - print name of current/working directory" << endl;
//         cout<< "SYNTAX  pwd" << endl;
//     }
//     else if (cmd == "whoami") {
//         cout<< "NAME  who - show who is logged on" << endl;
//         cout<< "SYNTAX  whoami" <<endl;
//     }
//     else if (cmd == "ls") {
//         if (nextcmd == "-l") {
//             cout << "NAME   ls -l - use a long listing format"  << endl;
//             cout << "SYNTAX  ls -l" << endl;
//         }
//         else {
//             cout << "NAME ls - list directory contents"  << endl;
//             cout << "SYNTAX  ls" << endl;  
//         }
//     }
//     else if (cmd == "cd") {
//         cout << "NAME  cd - enter into a directory" << endl;
//         cout << "SYNTAX  cd"  << endl;
//     }
//     else if (cmd == "mkdir") {
//         cout << "NAME  mkdir - make new directories" << endl;
//         cout << "SYNTAX  mkdir [filename]" << endl;
//     }
//     else if (cmd == "rmdir") {
//         cout << "NAME  rmdir - deletes empty as well non empty directories" << endl;
//         cout << "SYNTAX  rmdir [filename]" << endl;
//     }
//     else if (cmd == "touch") {
//         cout << "NAME  touch - creates empty files" << endl;
//         cout << "SYNTAX  touch [filename]" << endl;
//     }
//     else if (cmd == "exit") {
//         cout << "NAME  exit - exits the shell"  << endl;
//         cout << "SYNTAX  exit"  << endl;
//     }
//     else if (cmd == "cat") {
//         cout << "NAME  cat - concatenate files and print on the standard output" << endl;
//         cout << "SYNTAX  cat [filename]" << endl;
//     }
//     else if (cmd == "echo") {
//         cout << "NAME  echo - display a line of text" << endl;
//         cout << "SYNTAX  echo [text]" << endl;
//     }
//     else if (cmd == "cp") {
//         cout << "NAME  cp - copy files and directories" << endl;
//         cout << "SYNTAX  cp [sourcefile] [destinationfile]" << endl;
//     }
//     else if (cmd == "date") {
//         cout << "NAME  date - print or set the system date and time" << endl;
//         cout << "SYNTAX  date" << endl;
//     }
//     else if (cmd == "diff") {
//         cout << "NAME  diff - compare files line by line" << endl;
//         cout << "SYNTAX  diff [filename]";
//     }
//     else if (cmd == "rm") {
//         cout << "NAME  rm - remove files" << endl;
//         cout << "SYNTAX  rm [filename]" << endl;
//     }
//     else if (cmd == "cls") {
//         cout << "NAME  cls - clears screen" << endl;
//         cout << "SYNTAX  cls" << endl;
//     }
//     else if (cmd == "clear") {
//         cout << "NAME  clear - clears screen" << endl;
//         cout << "SYNTAX  clear" << endl;
//     }
//     else if (cmd == "gzip") {
//         cout << "NAME  gzip - compress or expand files"  << endl;
//         cout << "SYNTAX  gzip [filename]" << endl;
//     }
//     else if (cmd == "history") {
//         cout << "NAME  history - GNU History Library" << endl;
//         cout << "SYNTAX  history" << endl;
//     }
//     else if (cmd == "chmod") {
//         cout << "NAME  chmod - change file mode bits" << endl;
//         cout << "SYNTAX chmod [permission octal digits] [filename]" << endl;
//     }
//     else if (cmd == "hexdump") {
//         cout << "NAME  hexdump - display file contents in octal" << endl;
//         cout << "SYNTAX  hexdump [filename]" <<endl;
//     }
//     else if (cmd == "ps") {
//         cout << "NAME  ps - report a snapshot of the current processes" << endl;
//         cout << "SYNTAX  ps" <<endl;
//     }
//     else if (cmd == "wc") {
//         cout << "NAME  wc - print newline, word, and byte counts for each file" << endl;
//         cout << "SYNTAX  wc [filename]" << endl;
//     }
//     else if (cmd == "uname") {
//         cout << "NAME  uname - print system information" << endl;
//         cout << "SYNTAX  uname" <<endl;
//     }
//     else if (cmd == "uptime") {
//         cout << "NAME  uptime - Tell how long the system has been running" << endl;
//         cout << "SYNTAX  uptime" << endl;
//     }
//     else if (cmd == "free") {
//         cout << "NAME  free - Display amount of free and used memory in the system" << endl;
//         cout << "SYNTAX  free" <<endl;
//     }
//     else if (cmd == "df") {
//         cout << "NAME  df - report file system disk space usage" << endl;
//         cout << "SYNTAX  df" <<endl;
//     }
//     else if (cmd == "uniq") {
//         cout << "NAME  uniq - report or omit repeated lines" << endl;
//         cout << "SYNTAX  uniq [filename]" << endl;
//     }
//     else if (cmd == "man") {
//         cout << "NAME  man - an interface to the system reference manuals" << endl;
//         cout << "SYNTAX  man" << endl;
//     }
// }

int main() {
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
    // commandRegister["man"] = manCommand;
    commandRegister["wget"] = wgetCommand;
    commandRegister["mv"] = mvCommand;

    loginAsRoot();
    addRoot();

    cout << endl;
    
    string input;
    while (true) {
        cout << ".nyan:~$ ";
        getline(cin, input);
        addCommand(input);

        vector<string> tokens = tokenize(input);
        if (tokens[0] == "login") {
            loginCommand(tokens);
        }

        if (commandRegister.find(tokens[0]) != commandRegister.end()) {
                commandRegister[tokens[0]](tokens);
        } 
    }
    return 0;
}