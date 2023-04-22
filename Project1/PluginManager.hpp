#include "minizip/zip.h"
#include "minizip/unzip.h"
#include <vector>
#include <string>
#include <iostream>


class PluginManager {
	
public:
	std::unique_ptr<std::vector<std::string>> getClassData(std::string loc) {
        unzFile uf = unzOpen(("./plugins/"+loc).c_str());
        

        if (!uf)
        {
            std::cerr << "Error: Failed to open zip file." << std::endl;
            return;
        }

        if (unzGoToFirstFile(uf) != UNZ_OK)
        {
            std::cerr << "Error: Failed to go to first file in zip archive." << std::endl;
            unzClose(uf);
            return;
        }

        auto files = std::make_unique<std::vector<std::string>>();
        do
        {
            char filename_inzip[256];
            unz_file_info64 file_info;

            if (unzGetCurrentFileInfo64(uf, &file_info, filename_inzip, sizeof(filename_inzip), nullptr, 0, nullptr, 0) != UNZ_OK)
            {
                std::cerr << "Error: Failed to get current file info in zip archive." << std::endl;
                unzClose(uf);
                return;
            }
            std::string buffer(file_info.uncompressed_size, '\0');
            if (std::strstr(filename_inzip, ".class")) {
                if (unzReadCurrentFile(uf, &buffer[0], buffer.size()) != static_cast<int>(buffer.size()))
                {
                    std::cerr << "Error: Failed to read file from zip archive." << std::endl;
                    unzCloseCurrentFile(uf);
                    unzClose(uf);
                    return;
                }
            }
            files->push_back(buffer);
        } while (unzGoToNextFile(uf) == UNZ_OK);

        unzClose(uf);

        return files;
	}
};