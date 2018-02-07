#ifndef DB_MOUDLE_H_SDF32DSFD_
#define DB_MOUDLE_H_SDF32DSFD_
 
#include <memory>

class QSqlDatabase;
class DBMoudle
{
public:
    DBMoudle();
    ~DBMoudle(){}

    bool Init();

private:
    std::shared_ptr<QSqlDatabase> p_database_;
};

#endif //DB_MOUDLE_H_SDF32DSFD_

