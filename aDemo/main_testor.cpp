#if 1

#include "app_testor.h"
 

//#include <boost/lexical_cast.hpp>

#include <TLib/core/tsystem_core_common.h>
#include <TLib/core/tsystem_core_paths.h>
#include <TLib/tool/tsystem_exe_frame.h>
 
class TheFrame : public TSystem::ExecutableFrame
{
public:

    int main(int argc, char* argv[])
    {
#if 1
    std::unordered_map<int, int>  ord_map;
    ord_map.insert(std::make_pair(1, 10));

    ord_map.insert(std::make_pair(1, 20));

    printf("val:%d \n", ord_map[1]); // out 10
    printf("val:%d \n", ord_map.find(1)->second); // out 10

    ord_map[1] = 20;
    printf("val:%d \n", ord_map[1]); // out 20

    ord_map[2] = 30;
    printf("val:%d \n", ord_map[2]); // out 30
    getchar();
#endif
        TSystem::utility::ProjectTag("WZF");

        AppTestor  app_testor;

        app_testor.Initiate();

        PrintLaunchDone();
        PrintAppInfo(app_testor);
        app_testor.WaitShutdown();
        return 0;
    }
};


int main(int argc, char* argv[])
{
    TheFrame frame;
    return frame.LaunchServer(argc, argv);
}

#endif