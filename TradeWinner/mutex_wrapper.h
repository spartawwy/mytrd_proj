#ifndef MUTEX_WRAPPER_H_W32SDFDS_
#define MUTEX_WRAPPER_H_W32SDFDS_

//#include <mutex>
#include <atomic>


namespace TSystem
{
    class LocalLogger;
};
class TimedMutexWrapper
{
public:

    TimedMutexWrapper(int id=0, TSystem::LocalLogger *local_logger=nullptr);
    ~TimedMutexWrapper();

    bool try_lock_for(__int64 milli_sec);
    void unlock();

private:

    /*class TimedMutext
    {
    public:
        TimedMutext();
        ~TimedMutext();
    private: 
        std::atomic_bool is_locked_;
    };*/

private:

     //std::timed_mutex  timed_mutex_;
     std::atomic_bool is_locked_;

     int id_;
     TSystem::LocalLogger *local_logger_;
};
#endif // MUTEX_WRAPPER_H_W32SDFDS_
