#include "winner_app.h"
#include <QtWidgets/QApplication>
#include <qtextcodec.h>

//#include <boost/lexical_cast.hpp>

#include <TLib/core/tsystem_core_common.h>
#include <TLib/core/tsystem_core_paths.h>
#include <TLib/tool/tsystem_exe_frame.h>

#include "winner_app.h"

using namespace TSystem;
 
class TheFrame : public TSystem::ExecutableFrame
{
 
public:
    int main(int argc, char* argv[])
    {
    TSystem::utility::ProjectTag("WZF");
#ifdef PUBLISH
#else
	QCoreApplication::addLibraryPath("D:\\Qt\\qt5.2.1_win32\\bin\\plugins");
#endif
    QCoreApplication::addLibraryPath(".");
	QCoreApplication::addLibraryPath("./plugins");
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));

    //QApplication a(argc, argv);
    //-------------test
    
    //---------------end

    WinnerApp  winner_app(argc, argv);
    int ret = 0;
 
    PrintAppInfo(winner_app);
   
    if( winner_app.Init() )
    { 
        PrintLaunchDone();
        ret = winner_app.exec(); 
    }else
        winner_app.Stop();
     
    winner_app.WaitShutdown();
   
    return ret;
    }
};

int main(int argc, char* argv[])
{
   TheFrame frame;
   return frame.LaunchServer(argc, argv);
}