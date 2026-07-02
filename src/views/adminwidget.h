#ifndef ADMINWIDGET_H
#define ADMINWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QLabel>
#include <QLineEdit>
#include <QDialog>
#include "src/models/datatypes.h"

class AdminWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AdminWidget(QWidget *parent = nullptr);

signals:
    void logoutRequested();

private slots:
    void fetchAll();
    void switchTab(const QString &key);
    void openAddDialog();
    void openEditDialog(const QJsonObject &item);
    void saveForm();
    void confirmDelete();
    void promptDelete(qint64 id, const QString &label);
    void refreshTable();
    void logout();

private:
    void setupUi();
    void setupHeader();
    void setupTable();
    QDialog* buildFormDialog(bool isEdit);
    void showToast(const QString &text, int type = 0);

    // Data
    QList<User>          m_users;
    QList<Department>    m_depts;
    QList<Doctor>        m_doctors;
    QList<KnowledgeBase> m_kb;
    QList<Announcement>  m_announcements;
    QList<Rating>        m_ratings;

    QString m_activeTab = "users";
    QString m_editingType;
    QJsonObject m_editingPayload;
    QDialog *m_formDialog = nullptr;
    QDialog *m_deleteDialog = nullptr;
    QString m_searchQuery;

    // Stats
    QLabel *m_userCount;
    QLabel *m_deptCount;
    QLabel *m_docCount;
    QLabel *m_kbCount;
    QLabel *m_annCount;
    QLabel *m_ratingCount;

    // Table
    QTableWidget *m_table;
    QLineEdit    *m_searchEdit;
};

#endif // ADMINWIDGET_H
