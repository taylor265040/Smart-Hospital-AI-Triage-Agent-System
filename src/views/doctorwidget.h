#ifndef DOCTORWIDGET_H
#define DOCTORWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QScrollArea>
#include "src/models/datatypes.h"

class DoctorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DoctorWidget(QWidget *parent = nullptr);
    ~DoctorWidget() override;

signals:
    void logoutRequested();

private slots:
    void fetchData();
    void toggleDuty();
    void confirmAppointment(qint64 id);
    void rejectAppointment(qint64 id);
    void switchTab(const QString &status);
    void logout();

private:
    void setupUi();
    void setupHeader();
    void refreshStats();
    void refreshAppointmentList();
    void updateDutyButtonStyles();
    void showToast(const QString &text, int type = 0);

    bool m_doctorOnDuty = true;
    qint64 m_doctorId = 0;
    Doctor m_doctorRecord;
    QList<Appointment> m_appointments;
    QMap<qint64, QString> m_deptNameMap;
    QMap<qint64, QString> m_userNameMap;
    QTimer *m_pollTimer = nullptr;
    QString m_activeTab = "ALL";

    QLabel *m_nameLabel;
    QLabel *m_titleLabel;
    QLabel *m_deptLabel;
    QPushButton *m_dutyOnBtn;
    QPushButton *m_dutyOffBtn;
    QLabel *m_totalLabel;
    QLabel *m_pendingLabel;
    QLabel *m_confirmedLabel;
    QLabel *m_todayLabel;
    QVBoxLayout *m_apptListLayout;
};

#endif // DOCTORWIDGET_H
