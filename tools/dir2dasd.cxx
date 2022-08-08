// dir2dasd.cxx - Recursively convert a directory to a DASD image

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <ctype.h>
#include <sys/stat.h>

static std::vector<std::string> used_fullnames;
static std::vector<std::string> paths;
static std::vector<std::string> excluded_extensions;
static inline bool is_extension_excluded(const std::string& extension) {
    return std::find(excluded_extensions.begin(), excluded_extensions.end(), extension) != excluded_extensions.end();
}

static int reclen = 3450, trklen = 14336;

int main(int argc, char **argv) {
    if(argc == 1) {
        std::cout << argv[0] << " - Convert directories to DASDs" << std::endl;
        std::cout << "-v [diskname] - Set volume label" << std::endl;
        std::cout << "-i [path]     - Use an IPL loader" << std::endl;
        std::cout << "-e [name]     - Exclude extension from inclusion on disk" << std::endl;
        std::cout << "-k [path]     - Include a kernel beforehand (easier for simpler IPLs)" << std::endl;
        std::cout << "-o            - Output the resulting DASDCTL to stdout" << std::endl;
    }

    std::string diskname = "DISK00", iplname = "", kernel_name = "";
    bool use_cd = true;

    excluded_extensions.push_back(".DCF");
    excluded_extensions.push_back(".D");
    excluded_extensions.push_back(".A");
    excluded_extensions.push_back(".O");
    excluded_extensions.push_back(".OBJ");
    excluded_extensions.push_back(".O1");
    excluded_extensions.push_back("."); // Extension-less
    excluded_extensions.push_back("");

    for(int i = 1; i < argc; i++) {
        if(!strcmp(argv[i], "-v")) {
            i++;
            if(i >= argc) throw std::runtime_error("Expected an argument for " + std::string(argv[i - 1]));
            diskname = argv[i];
        } else if(!strcmp(argv[i], "-p")) {
            i++;
            if(i >= argc) throw std::runtime_error("Expected an argument for " + std::string(argv[i - 1]));
            paths.push_back(argv[i]);
            use_cd = false;
        } else if(!strcmp(argv[i], "-i")) {
            i++;
            if(i >= argc) throw std::runtime_error("Expected an argument for " + std::string(argv[i - 1]));
            iplname = argv[i];
        } else if(!strcmp(argv[i], "-e")) {
            i++;
            if(i >= argc) throw std::runtime_error("Expected an argument for " + std::string(argv[i - 1]));
            excluded_extensions.push_back(argv[i]);
        } else if(!strcmp(argv[i], "-k")) {
            i++;
            if(i >= argc) throw std::runtime_error("Expected an argument for " + std::string(argv[i - 1]));
            kernel_name = argv[i];
        } else if(!strcmp(argv[i], "-o")) {
            if(use_cd) {
                paths.push_back(".");
                use_cd = false;
            }

            std::cout << diskname << " 2305-1 * " << iplname << std::endl;
            if(!kernel_name.empty()) {
                struct stat st = {};
                stat(kernel_name.c_str(), &st);
                int tracks = (st.st_size / reclen) + 1;
                std::cout << "KERNEL SEQ " << kernel_name << " TRK " << std::to_string(tracks) << " 1 0 PS FB 1 " << std::to_string(reclen) << std::endl;
            }

            std::cout << "SYSVTOC VTOC TRK 13" << std::endl;

            for(const auto& path : paths) {
                for(const auto& p: std::filesystem::recursive_directory_iterator(path)) {
                    if(!std::filesystem::is_directory(p)) {
                        std::string filename = p.path().stem().string();
                        for(auto& ch : filename) {
                            ch = toupper(ch);
                        }

                        std::string extension = p.path().extension().string();
                        for(auto& ch : extension) {
                            ch = toupper(ch);
                        }
                        if(is_extension_excluded(extension)) {
                            if(filename != "MAKEFILE") { // Allow certain files
                                continue;
                            }
                        }

                        std::string fullname = "DSA" + extension + "." + filename;
                        while(std::find(used_fullnames.begin(), used_fullnames.end(), fullname) != used_fullnames.end()) {
                            fullname[2]++;
                        }
                        used_fullnames.push_back(fullname);

                        struct stat st = {};
                        stat(p.path().string().c_str(), &st);
                        int tracks = (st.st_size / reclen) + 1;
                        std::cout << fullname << "\tSEQ\t" << p.path().string() << "\tTRK " << std::to_string(tracks) << " 1 0 PS FB 1 " << std::to_string(reclen) << std::endl;
                    }
                }
            }
            std::cout << "# PATH=" << paths[0] << ",NAME=" << diskname << ",IPL=" << iplname << std::endl;
        }
    }
    return 0;
}
