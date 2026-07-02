/****************************************************************************
** Meta object code from reading C++ file 'cswidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/views/cswidget.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cswidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.7.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSCSWidgetENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSCSWidgetENDCLASS = QtMocHelpers::stringData(
    "CSWidget",
    "logoutRequested",
    "",
    "fetchData",
    "toggleOnlineStatus",
    "acceptQueue",
    "convId",
    "selectChat",
    "sendMessage",
    "confirmEndSession",
    "promptEndSession",
    "logout"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSCSWidgetENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   68,    2, 0x06,    1 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       3,    0,   69,    2, 0x08,    2 /* Private */,
       4,    0,   70,    2, 0x08,    3 /* Private */,
       5,    1,   71,    2, 0x08,    4 /* Private */,
       7,    1,   74,    2, 0x08,    6 /* Private */,
       8,    0,   77,    2, 0x08,    8 /* Private */,
       9,    0,   78,    2, 0x08,    9 /* Private */,
      10,    0,   79,    2, 0x08,   10 /* Private */,
      11,    0,   80,    2, 0x08,   11 /* Private */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::LongLong,    6,
    QMetaType::Void, QMetaType::LongLong,    6,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject CSWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSCSWidgetENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSCSWidgetENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSCSWidgetENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<CSWidget, std::true_type>,
        // method 'logoutRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'fetchData'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'toggleOnlineStatus'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'acceptQueue'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        // method 'selectChat'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        // method 'sendMessage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'confirmEndSession'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'promptEndSession'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'logout'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void CSWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CSWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->logoutRequested(); break;
        case 1: _t->fetchData(); break;
        case 2: _t->toggleOnlineStatus(); break;
        case 3: _t->acceptQueue((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1]))); break;
        case 4: _t->selectChat((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1]))); break;
        case 5: _t->sendMessage(); break;
        case 6: _t->confirmEndSession(); break;
        case 7: _t->promptEndSession(); break;
        case 8: _t->logout(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CSWidget::*)();
            if (_t _q_method = &CSWidget::logoutRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject *CSWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CSWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSCSWidgetENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CSWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void CSWidget::logoutRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
