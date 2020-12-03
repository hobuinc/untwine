#ifndef _WIN32
#include <unistd.h>
#endif

#include <iostream>

#include "QgisUntwine.hpp"

int main()
{
    untwine::QgisUntwine::StringList files;
    untwine::QgisUntwine::Options options;
    untwine::QgisUntwine api("C:\\Users\\andre\\untwine\\build\\untwine.exe");
    
    files.push_back("C:\\Users\\andre\\nyc2");
//    files.push_back("C:\\Users\\andre\\nyc2\\18TXL075075.las.laz");
    api.start(files, ".\\out", options);

    bool stopped = false;
    while (true)
    {
#ifdef _WIN32
    	Sleep(1000);
#else
        ::sleep(1);
#endif
        int percent = api.progressPercent();
        std::string s = api.progressMessage();
        std::cerr << "Percent/Msg = " << percent << " / " << s << "!\n";
        /**
        if (!stopped && percent >= 50)
        {
            stopped = true;
            api.stop();
        }
        **/
        if (!api.running())
            break;
    }
}
