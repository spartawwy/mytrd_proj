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