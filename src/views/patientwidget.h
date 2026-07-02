#ifndef PATIENTWIDGET_H
#define PATIENTWIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QTimer>
#include <QComboBox>
#include <QDateEdit>
#include "src/models/datatypes.h"

class PatientWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PatientWidget(QWidget *parent = nullptr);
    ~PatientWidget() override;

signals:
    void logoutRequested();

private slots:
    void switchTab(int index);
    void sendMessage();
    void startChat();
    void syncMessages();
    void checkQueueStatus();
    void requestHumanService();
    void endManualService();
    void submitRating();

    // Booking
    void onDeptSelected(int deptId);
    void onDoctorExpand(qint64 docId);
    void onDateChanged(const QDate &date);
    void bookAppointment(qint64 docId);

    // Appointments
    void fetchAppointments();
    void cancelAppointment(qint64 id);
    void logout();

private:
    void setupUi();
    void setupHeader();
    void setupNav();
    void setupChatTab();
    void setupBookingTab();
    void setupAppointmentsTab();
    void addChatMessage(const QString &content, const QString &senderType, const QString &senderName = QString());
    void showToast(const QString &text, int type = 0);
    void setLoading(bool loading);
    void fetchBaseData();
    void refreshAppointmentPanel();
    void refreshBookingSlots(qint64 docId);

    // Data
    QList<Announcement>     m_announcements;
    QList<AppointmentDept>  m_depts;
    QMap<qint64, QString>   m_deptNameMap;
    QMap<qint64, QString>   m_docNameMap;
    QMap<qint64, Doctor>    m_doctorMap;
    QList<Message>          m_messages;
    QList<Appointment>      m_appointments;
    AppointmentDept         m_selectedDept;
    QList<Doctor>           m_deptDoctors;
    QMap<qint64, QList<TimeSlot>> m_bookingSlots;
    qint64                  m_expandedDocId = 0;
    QString                 m_selectedSlot;
    QDate                   m_selectedDate;

    qint64   m_activeConvId  = 0;
    bool     m_isManualMode  = false;
    QueueStatus m_queueStatus;
    int      m_activeApptCount = 0;
    QTimer  *m_pollTimer     = nullptr;
    QTimer  *m_apptPollTimer = nullptr;

    // Chat sub-tab
    QWidget      *m_chatTab;
    QWidget      *m_chatMessagesContainer;
    QVBoxLayout  *m_chatMessagesLayout;
    QScrollArea  *m_chatScrollArea;
    QLineEdit    *m_chatInput;
    QWidget      *m_recommendBar;

    // Booking sub-tab
    QWidget      *m_bookingTab;
    QWidget      *m_bookingDeptGrid;
    QWidget      *m_bookingDocArea;
    QDateEdit    *m_dateEdit;

    // Appointments sub-tab
    QWidget      *m_appointmentsTab;
    QVBoxLayout  *m_apptsLayout;
    QStackedWidget *m_contentStack;
    QWidget      *m_navBar;
    QPushButton  *m_chatNavBtn;
    QPushButton  *m_bookingNavBtn;
    QPushButton  *m_apptNavBtn;
};

#endif // PATIENTWIDGET_H
