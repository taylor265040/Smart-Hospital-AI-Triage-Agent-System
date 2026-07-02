/****************************************************************************
** Meta object code from reading C++ file 'patientwidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/views/patientwidget.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'patientwidget.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSPatientWidgetENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSPatientWidgetENDCLASS = QtMocHelpers::stringData(
    "PatientWidget",
    "logoutRequested",
    "",
    "switchTab",
    "index",
    "sendMessage",
    "startChat",
    "syncMessages",
    "checkQueueStatus",
    "requestHumanService",
    "endManualService",
    "submitRating",
    "onDeptSelected",
    "deptId",
    "onDoctorExpand",
    "docId",
    "onDateChanged",
    "date",
    "bookAppointment",
    "fetchAppointments",
    "cancelAppointment",
    "id",
    "logout"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSPatientWidgetENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  110,    2, 0x06,    1 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       3,    1,  111,    2, 0x08,    2 /* Private */,
       5,    0,  114,    2, 0x08,    4 /* Private */,
       6,    0,  115,    2, 0x08,    5 /* Private */,
       7,    0,  116,    2, 0x08,    6 /* Private */,
       8,    0,  117,    2, 0x08,    7 /* Private */,
       9,    0,  118,    2, 0x08,    8 /* Private */,
      10,    0,  119,    2, 0x08,    9 /* Private */,
      11,    0,  120,    2, 0x08,   10 /* Private */,
      12,    1,  121,    2, 0x08,   11 /* Private */,
      14,    1,  124,    2, 0x08,   13 /* Private */,
      16,    1,  127,    2, 0x08,   15 /* Private */,
      18,    1,  130,    2, 0x08,   17 /* Private */,
      19,    0,  133,    2, 0x08,   19 /* Private */,
      20,    1,  134,    2, 0x08,   20 /* Private */,
      22,    0,  137,    2, 0x08,   22 /* Private */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   13,
    QMetaType::Void, QMetaType::LongLong,   15,
    QMetaType::Void, QMetaType::QDate,   17,
    QMetaType::Void, QMetaType::LongLong,   15,
    QMetaType::Void,
    QMetaType::Void, QMetaType::LongLong,   21,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject PatientWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSPatientWidgetENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSPatientWidgetENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSPatientWidgetENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<PatientWidget, std::true_type>,
        // method 'logoutRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'switchTab'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'sendMessage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'startChat'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'syncMessages'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'checkQueueStatus'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'requestHumanService'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'endManualService'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'submitRating'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDeptSelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onDoctorExpand'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        // method 'onDateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QDate &, std::false_type>,
        // method 'bookAppointment'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        // method 'fetchAppointments'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'cancelAppointment'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        // method 'logout'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void PatientWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PatientWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->logoutRequested(); break;
        case 1: _t->switchTab((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->sendMessage(); break;
        case 3: _t->startChat(); break;
        case 4: _t->syncMessages(); break;
        case 5: _t->checkQueueStatus(); break;
        case 6: _t->requestHumanService(); break;
        case 7: _t->endManualService(); break;
        case 8: _t->submitRating(); break;
        case 9: _t->onDeptSelected((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 10: _t->onDoctorExpand((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1]))); break;
        case 11: _t->onDateChanged((*reinterpret_cast< std::add_pointer_t<QDate>>(_a[1]))); break;
        case 12: _t->bookAppointment((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1]))); break;
        case 13: _t->fetchAppointments(); break;
        case 14: _t->cancelAppointment((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1]))); break;
        case 15: _t->logout(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PatientWidget::*)();
            if (_t _q_method = &PatientWidget::logoutRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject *PatientWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PatientWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSPatientWidgetENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int PatientWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 16;
    }
    return _id;
}

// SIGNAL 0
void PatientWidget::logoutRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
