#include "mainwindow.hxx"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    is_split = false;
    is_mono = false;
    is_loaded = false;
    img_hlp = NULL;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete img_hlp;
}

void MainWindow::on_load_btn_clicked()
{
    QString str = QFileDialog::getOpenFileName();
    img = QImage(str);
    this->img_hlp = new image_helper(img);
    ui->img_src->setPixmap(this->img_hlp->get_src());
    is_split = false;
    is_mono = false;
    is_loaded = true;
}

void MainWindow::on_split_btn_clicked()
{
    if (!this->img_hlp) {
        return;
    }
    ui->img_src->setPixmap(this->img_hlp->get_gray());
}


void MainWindow::on_colours_btn_clicked()
{
	this->img_hlp->proc_colours();
	ui->img_src->setPixmap(this->img_hlp->get_gray());
}

void MainWindow::on_spatial_btn_clicked()
{
	this->img_hlp->proc_spatial();
	ui->img_src->setPixmap(this->img_hlp->get_gray());
}

void MainWindow::on_mark_btn_clicked()
{
	ui->img_src->setPixmap(this->img_hlp->get_visualise());
}

void MainWindow::on_save_btn_clicked()
{
    QString fname;
    fname = QFileDialog::getSaveFileName();
    if (fname.isEmpty()) {
        return;
    }
    QFile file(fname);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    QPair<int, int> wh = this->img_hlp->get_wh();
    out.writeRawData(QString("Trip_01").toLatin1(), 8);
    out << (quint16)wh.first;
    out << (quint16)wh.second;
    out << img_hlp->get_string();
    file.close();
}
