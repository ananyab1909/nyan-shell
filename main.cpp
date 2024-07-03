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
#ifdef _WIN32
#include<windows.h>
#endif

using namespace std;

map<string, void (*)(const vector<string>&)> commandRegister; 
string currentWorkingDirectory;
string prevDir;
vector<string> history;

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

void whoamiCommand(const vector<string>& args) {
    const char* username = getenv("USERNAME");
    if (username) {
        cout<<""<<username<<endl;
    } else {
        cout <<"Error: Unable to get name"<< endl;
    }
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

struct ProcessInfo {
    string pid;
    string user;
    string cpu;
    string mem;
    string vsz;
    string rss;
    string tty;
    string stat;
    string start;
    string time;
    string cmd;
};

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
        cout << "Error: rmdir command expects one argument" << endl;
        return;
    }
    string dir = args[1];
    dir.erase(0, dir.find_first_not_of(" \t")); 
    dir.erase(dir.find_last_not_of(" \t") + 1);
    char cmd[256];
    sprintf(cmd, "mkdir \"%s\"", dir.c_str());
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
    DIR* dp;
    if ((dp = opendir(dir.c_str())) == NULL) {
        cout << "Error: Directory does not exist" << endl;
        return;
    }
    closedir(dp);

    if (remove(dir.c_str()) == 0) {
        cout << "Directory deleted successfully!" << endl;
    } else {
        cout << "Error: Unable to delete directory" << endl;
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
        ofstream(args[1]).close();
        cout<< "File created successfully!"<<endl;
    }
    #elif __linux__
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
    if (remove(file.c_str()) == 0) {
        cout << "File deleted successfully!" << endl;
    } else {
        cout << "Error: Unable to delete file." << endl;
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
    string cmd = "gzip " + file;
    system(cmd.c_str());
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
    ifstream file("/proc/mounts");
    string line, path, mountPoint, fsType, dummy;

    cout << "Filesystem      1K-blocks      Used Available Use% Mounted on\n";

    while (getline(file, line)) {
        istringstream iss(line);
        iss >> dummy >> path >> dummy >> dummy >> dummy >> mountPoint >> fsType >> dummy;

        ifstream statFile("/proc/diskstats");
        string statLine;
        bool found = false;

        while (getline(statFile, statLine)) {
            istringstream statIss(statLine);
            string statDummy, statPath;
            statIss >> statDummy >> statPath >> dummy >> dummy >> dummy >> dummy >> dummy >> dummy >> dummy >> dummy >> dummy;

            if (statPath == path) {
                found = true;
                break;
            }
        }

        statFile.close();

        if (found) {
            ifstream sysFile("/sys/block/" + path.substr(5) + "/size");
            string sysLine;
            getline(sysFile, sysLine);
            int totalBlocks = stoi(sysLine) * 1024;

            ifstream usedFile("/sys/block/" + path.substr(5) + "/used");
            getline(usedFile, sysLine);
            int usedBlocks = stoi(sysLine) * 1024;

            int availableBlocks = totalBlocks - usedBlocks;
            double usePercent = (double)usedBlocks / totalBlocks * 100;

            cout << mountPoint << "         " << totalBlocks / 1024 << "         " << usedBlocks / 1024 << "         " << availableBlocks / 1024 << "         " << (int)usePercent << "%" << "         " << mountPoint << endl;
        }
    }

    file.close();
    #endif
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
    time(&now);
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    int elapsed_minutes = static_cast<int>(elapsed / 60);
    int elapsed_hours = static_cast<int>(elapsed_minutes / 60);
    elapsed_minutes %= 60;
    printf("%02d:%02d:%02d ", (localtime(&now))->tm_hour, (localtime(&now))->tm_min, (localtime(&now))->tm_sec);
    
    cout << "up " << elapsed_hours << ":" << elapsed_minutes << ","  << " " << "user" << "," <<endl;     
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

    cout << "Logging in as root" << endl;
    
    string input;
    while (true) {
        cout << ".nyan:~$ ";
        getline(cin, input);
        addCommand(input);

        vector<string> tokens = tokenize(input);
        if (commandRegister.find(tokens[0]) != commandRegister.end()) {
                commandRegister[tokens[0]](tokens);
        } 
    }
    return 0;
}