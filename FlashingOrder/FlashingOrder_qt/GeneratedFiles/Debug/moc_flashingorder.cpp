/****************************************************************************
** Meta object code from reading C++ file 'flashingorder.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.2.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../flashingorder.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'flashingorder.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.2.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_FlashingOrder_t {
    QByteArrayData data[11];
    char stringdata[107];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_FlashingOrder_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_FlashingOrder_t qt_meta_stringdata_FlashingOrder = {
    {
QT_MOC_LITERAL(0, 0, 13),
QT_MOC_LITERAL(1, 14, 7),
QT_MOC_LITERAL(2, 22, 0),
QT_MOC_LITERAL(3, 23, 12),
QT_MOC_LITERAL(4, 36, 10),
QT_MOC_LITERAL(5, 47, 9),
QT_MOC_LITERAL(6, 57, 9),
QT_MOC_LITERAL(7, 67, 7),
QT_MOC_LITERAL(8, 75, 8),
QT_MOC_LITERAL(9, 84, 12),
QT_MOC_LITERAL(10, 97, 8)
    },
    "FlashingOrder\0key_sig\0\0show_msg_sig\0"
    "log2ui_sig\0DoReadCfg\0DoSaveCfg\0DoTimer\0"
    "DoKeySig\0DoShowMsgSig\0DoLog2Ui\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_FlashingOrder[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   59,    2, 0x06,
       3,    2,   62,    2, 0x06,
       4,    1,   67,    2, 0x06,

 // slots: name, argc, parameters, tag, flags
       5,    0,   70,    2, 0x08,
       6,    0,   71,    2, 0x08,
       7,    0,   72,    2, 0x08,
       8,    1,   73,    2, 0x08,
       9,    2,   76,    2, 0x08,
      10,    1,   81,    2, 0x08,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    2,    2,
    QMetaType::Void, QMetaType::QString,    2,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    2,    2,
    QMetaType::Void, QMetaType::QString,    2,

       0        // eod
};

void FlashingOrder::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        FlashingOrder *_t = static_cast<FlashingOrder *>(_o);
        switch (_id) {
        case 0: _t->key_sig((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->show_msg_sig((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 2: _t->log2ui_sig((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->DoReadCfg(); break;
        case 4: _t->DoSaveCfg(); break;
        case 5: _t->DoTimer(); break;
        case 6: _t->DoKeySig((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 7: _t->DoShowMsgSig((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 8: _t->DoLog2Ui((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (FlashingOrder::*_t)(QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&FlashingOrder::key_sig)) {
                *result = 0;
            }
        }
        {
            typedef void (FlashingOrder::*_t)(QString , QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&FlashingOrder::show_msg_sig)) {
                *result = 1;
            }
        }
        {
            typedef void (FlashingOrder::*_t)(QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&FlashingOrder::log2ui_sig)) {
                *result = 2;
            }
        }
    }
}

const QMetaObject FlashingOrder::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_FlashingOrder.data,
      qt_meta_data_FlashingOrder,  qt_static_metacall, 0, 0}
};


const QMetaObject *FlashingOrder::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FlashingOrder::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FlashingOrder.stringdata))
        return static_cast<void*>(const_cast< FlashingOrder*>(this));
    return QWidget::qt_metacast(_clname);
}

int FlashingOrder::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void FlashingOrder::key_sig(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void FlashingOrder::show_msg_sig(QString _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void FlashingOrder::log2ui_sig(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE
