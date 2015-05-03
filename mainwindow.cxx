#include "mainwindow.hxx"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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
    QImage img(str);
    this->img_hlp = new image_helper(img);
    ui->img_src->setPixmap(this->img_hlp->get_src());
}

void MainWindow::on_reload_btn_clicked()
{
	if (!this->img_hlp) {
		return;
	}
	ui->img_src->setPixmap(this->img_hlp->get_src());
}

void MainWindow::on_split_btn_clicked()
{
    if (!this->img_hlp) {
        return;
    }
    ui->img_src->setPixmap(this->img_hlp->get_gray());
}

void MainWindow::on_join_btn_clicked()
{
	if (!this->img_hlp) {
		return;
	}
	ui->img_src->setPixmap(this->img_hlp->get_joined());
}

void MainWindow::on_colours_btn_clicked()
{
	this->img_hlp->proc_colours();
	ui->img_src->setPixmap(this->img_hlp->get_gray());
}


void MainWindow::on_uncolours_btn_clicked()
{
	bool reverse = true;
	this->img_hlp->proc_colours(reverse);
	ui->img_src->setPixmap(this->img_hlp->get_gray());
}

void MainWindow::on_spatial_btn_clicked()
{
	this->img_hlp->proc_spatial();
	ui->img_src->setPixmap(this->img_hlp->get_gray());
}

void MainWindow::on_unspatial_btn_clicked()
{
	bool reverse = true;
	this->img_hlp->proc_spatial(reverse);
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
