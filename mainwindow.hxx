#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QFileDialog>
#include <QImage>
#include <QPixmap>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_load_btn_clicked();
    void on_predict_btn_clicked();
    void on_split_btn_clicked();
    void on_delta_btn_clicked();
    void on_mark_btn_clicked();
    void on_save_btn_clicked();

private:
    Ui::MainWindow *ui;
    QImage img;
    QImage marks;
    bool is_loaded;
    bool is_split;
    bool is_mono;
};

#endif // MAINWINDOW_H
