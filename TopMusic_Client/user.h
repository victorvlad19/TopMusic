#ifndef USER_H
#define USER_H

#include <QDialog>

namespace Ui {
class User;
}

class User : public QDialog
{
    Q_OBJECT

public:
    explicit User(QWidget *parent = 0);
    void list_comments();
    void list_music_table();
    ~User();

private slots:
    void on_pushButton_clicked();
    void closeEvent(QCloseEvent *bar);

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_f_ex_clicked();

    void on_pushButton_voteaza();

    void on_comm_push_clicked();

    void on_channel_button_clicked();

private:
    Ui::User *ui;
};

#endif // USER_H
