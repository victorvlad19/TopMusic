#ifndef ADMIN_H
#define ADMIN_H

#include <QDialog>

namespace Ui {
class Admin;
}

class Admin : public QDialog
{
    Q_OBJECT

public:
    explicit Admin(QWidget *parent = 0);
    ~Admin();

private:
    Ui::Admin *ui;


private slots:
    void closeEvent(QCloseEvent *bar);

    void on_adm_sterge_clicked();
    void on_adm_res_clicked();
    void on_adm_res_2_clicked();
};
#endif // ADMIN_H
