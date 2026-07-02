/****************************************************************************
** Meta object code from reading C++ file 'adminwidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/views/adminwidget.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'adminwidget.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSAdminWidgetENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSAdminWidgetENDCLASS = QtMocHelpers::stringData(
    "AdminWidget",
    "logoutRequested",
    "",
    "fetchAll",
    "switchTab",
    "key",
    "openAddDialog",
    "openEditDialog",
    "item",
    "saveForm",
    "confirmDelete",
    "promptDelete",
    "id",
    "label",
    "refreshTable",
    "logout"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSAdminWidgetENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   74,    2, 0x06,    1 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       3,    0,   75,    2, 0x08,    2 /* Private */,
       4,    1,   76,    2, 0x08,    3 /* Private */,
       6,    0,   79,    2, 0x08,    5 /* Private */,
       7,    1,   80,    2, 0x08,    6 /* Private */,
       9,    0,   83,    2, 0x08,    8 /* Private */,
      10,    0,   84,    2, 0x08,    9 /* Private */,
      11,    2,   85,    2, 0x08,   10 /* Private */,
      14,    0,   90,    2, 0x08,   13 /* Private */,
      15,    0,   91,    2, 0x08,   14 /* Private */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QJsonObject,    8,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::LongLong, QMetaType::QString,   12,   13,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject AdminWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSAdminWidgetENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSAdminWidgetENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSAdminWidgetENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<AdminWidget, std::true_type>,
        // method 'logoutRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'fetchAll'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'switchTab'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'openAddDialog'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'openEditDialog'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QJsonObject &, std::false_type>,
        // method 'saveForm'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'confirmDelete'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'promptDelete'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'refreshTable'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'logout'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void AdminWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<AdminWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->logoutRequested(); break;
        case 1: _t->fetchAll(); break;
        case 2: _t->switchTab((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->openAddDialog(); break;
        case 4: _t->openEditDialog((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 5: _t->saveForm(); break;
        case 6: _t->confirmDelete(); break;
        case 7: _t->promptDelete((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 8: _t->refreshTable(); break;
        case 9: _t->logout(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (AdminWidget::*)();
            if (_t _q_method = &AdminWidget::logoutRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject *AdminWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AdminWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSAdminWidgetENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int AdminWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void AdminWidget::logoutRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
