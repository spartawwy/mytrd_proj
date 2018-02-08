#include "dbmoudle.h"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include <qdebug.h>

DBMoudle::DBMoudle()
    : p_database_(nullptr)
{
    p_database_ = std::make_shared<QSqlDatabase>(); //database;
   /* if (QSqlDatabase::contains("qt_sql_default_connection"))
    {
        database = QSqlDatabase::database("qt_sql_default_connection");
    }
    else
    {*/
        *p_database_ = QSqlDatabase::addDatabase("QSQLITE");
        p_database_->setDatabaseName("pzwj.kd");
        //database.setUserName("XingYeZhiXia");
        //database.setPassword("123456");
    //}
}

#include <qvariant.h>

bool DBMoudle::Init()
{
    if( !p_database_->open() )
    {
        qDebug() << "Error: Failed to connect database." << p_database_->lastError();
        return false;
    }

    QSqlQuery sql_query;
    QString sql_str = "SELECT name, code FROM stock order by name ";
    sql_query.prepare(sql_str);
    if(!sql_query.exec())
    {
        qDebug() << "Error: Fail to create table." << sql_query.lastError();
    }
    else
    {
        while(sql_query.next())
        {
            auto name0 = sql_query.value(0).toString();
            
           /* auto name1 = QString::fromLocal8Bit(sql_query.value(0).toString().toUtf8().constData());
            auto name2 = QString::fromLocal8Bit(sql_query.value(0).toByteArray().constData());
            QString name3 = sql_query.value(0).toString().toLatin1().constData();*/
            auto code = sql_query.value(1).toString();
            qDebug() << QString("name:%1 code:%2 ").arg(name0).arg(code) << "\n";
        }
    }
}