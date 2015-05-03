#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QFileDialog>
#include <QImage>
#include <QPixmap>

#include "image_helper.hxx"

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
    void on_reload_btn_clicked();
    void on_split_btn_clicked();
    void on_colours_btn_clicked();
    void on_uncolours_btn_clicked();
    void on_spatial_btn_clicked();
    void on_unspatial_btn_clicked();
    void on_mark_btn_clicked();
    void on_save_btn_clicked();

private:
    Ui::MainWindow *ui;
    image_helper *img_hlp;
};

#endif // MAINWINDOW_H
