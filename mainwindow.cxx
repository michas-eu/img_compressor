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
    if (!is_mono) {
        return;
    }
    ui->img_src->clear();
    int w, h;
    w = img.width();
    h = img.height();
    marks = QImage(w, h, QImage::Format_RGB32);
    int history  = 0;
    int y, x;
    for (y=0; y<h; y++) {
        for (x=0; x<w; x++) {
            QRgb npx;
            int cur = qBlue(img.pixel(x, y));
            if (cur != 128) {
		if (cur > 148 || cur < 108) {
		    npx = qRgb(255, 255, 0);
		} else if (cur > 132 || cur < 124) {
		    npx = qRgb(96, 0, 0);
		} else {
                    npx = qRgb(0, 0, 0);
		}
                history = 0;
            } else {
                if (history) {
                    npx = qRgb(0, 255, 0);
                } else {
                    npx = qRgb(0, 0, 255);
                }
	        history++;
           }
           marks.setPixel(x, y, npx);
        }
    }
    ui->img_src->setPixmap(QPixmap::fromImage(marks));
}

void MainWindow::on_save_btn_clicked()
{
    if (!is_mono) {
        return;
    }
    QString fname;
    fname = QFileDialog::getSaveFileName();
    if (fname.isEmpty()) {
        return;
    }
    QFile file(fname);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    int w, h;
    w = img.width();
    h = img.height();
    out.writeRawData(QString("Trip_01b").toLatin1(), 8);
    out << (quint16)w;
    out << (quint16)h;
    int y, x;
    for (y=0; y<h; y++) {
        for (x=0; x<w; x++) {
            unsigned char byte, b1, b2;
            int pre;
            pre = qBlue(img.pixel(x, y));
            byte = (unsigned char)(255 & pre);
	    if (byte == 0x80) {
		    byte = 0 + 'a';
		    out << byte;
	    } else if (byte > 0x80 && byte <= 0x84) {
		    byte = byte - 0x81 + 4 + 'a';
		    out << byte;
	    } else if (byte < 0x80 && byte >= 0x7c) {
		    byte = 0x7f - byte + 8 +'a';
		    out << byte;
	    } else if (byte > 0x80 && byte <= 0x94) {
		    byte = byte - 0x85;
		    b1 = (byte & 0xf) + 'a';
		    byte = 12 + 'a';
		    out << byte << b1;
	    } else if (byte < 0x80 && byte >= 0x6c) {
		    byte = 0x7b - byte;
		    b1 = (byte & 0xf) + 'a';
		    byte = 13 + 'a';
		    out << byte << b1;
	    } else if (byte > 0x80) {
		    byte = byte - 0x94;
		    b1 = ((byte >> 4) & 0xf) + 'a';
		    b2 = (byte & 0xf) + 'a';
		    byte = 14 + 'a';
		    out << byte << b1 << b2;
	    } else if (byte < 0x80) {
		    byte = 0x6c - byte;
		    b1 = ((byte >> 4) & 0xf) + 'a';
		    b2 = (byte & 0xf) + 'a';
		    byte = 15 + 'a';
		    out << byte << b1 << b2;
	    }else {
                out << byte;
	    }
        }
    }
    file.close();
}
