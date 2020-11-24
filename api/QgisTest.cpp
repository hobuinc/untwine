#include <iostream>
#include <unistd.h>

#include "QgisUntwine.hpp"

int main()
{
    untwine::QgisUntwine::StringList files;
    untwine::QgisUntwine::Options options;
    untwine::QgisUntwine api("/Users/acbell/untwine/build/untwine");
    
    files.push_back("/Users/acbell/nyc2");
    api.start(files, "./out", options);

    while (true)
    {
        sleep(1);
        std::cerr << "Reading!\n";
        int percent = api.progressPercent();
        std::string s = api.progressMessage();
        std::cerr << "Done Reading!\n\n";
        if (!api.running())
        {
            std::cerr << "Exited!\n";
            break;
        }
    }
}
