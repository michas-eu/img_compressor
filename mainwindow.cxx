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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_load_btn_clicked()
{
    QString str = QFileDialog::getOpenFileName();
    img = QImage(str);
    ui->img_src->setPixmap(QPixmap::fromImage(img));
    is_split = false;
    is_mono = false;
    is_loaded = true;
}

void MainWindow::on_split_btn_clicked()
{
    if (!is_loaded) {
        return;
    }
    ui->img_src->clear();
    int w, h;
    w = img.width();
    h = img.height();
    QImage new_img = QImage(w, h * 4, QImage::Format_RGB32);
    int x, y;
    for (y=0; y<h; y++) {
        for (x=0; x<w; x++) {
            QRgb opx = img.pixel(x, y);
            QRgb px_a, px_g, px_r, px_b;
            px_a = qAlpha(opx);
            px_g = qGreen(opx);
            px_r = qRed(opx);
            px_b = qBlue(opx);
            px_a = qRgb(px_a, px_a, px_a);
            px_g = qRgb(px_g, px_g, px_g);
            px_r = qRgb(px_r, px_r, px_r);
            px_b = qRgb(px_b, px_b, px_b);
            new_img.setPixel(x, y + h * 0, px_g);
            new_img.setPixel(x, y + h * 1, px_r);
            new_img.setPixel(x, y + h * 2, px_b);
            new_img.setPixel(x, y + h * 3, px_a);
        }
    }
    img = new_img;
    is_split = true;
    is_mono = true;
    ui->img_src->setPixmap(QPixmap::fromImage(img));
}

void MainWindow::on_predict_btn_clicked()
{
    if (!is_split || !is_mono) {
        return;
    }
    ui->img_src->clear();
    int w, h;
    w = img.width();
    h = img.height() / 4;
    int x, y;
    for (y=0; y<h; y++) {
        for (x=0; x<w; x++) {
            int g = qBlue(img.pixel(x, y + h * 0));
            int r = qBlue(img.pixel(x, y + h * 1));
            int b = qBlue(img.pixel(x, y + h * 2));
            int tmp;
            tmp = (5 * g + 3 * r) / 8;
            b = 255 & (b - tmp + 127);
            tmp = g;
            r = 255 & (r - tmp + 127);
            QRgb px_r, px_b;
            px_r = qRgb(r, r, r);
            px_b = qRgb(b, b, b);
            img.setPixel(x, y + h * 1, px_r);
            img.setPixel(x, y + h * 2, px_b);
        }
    }
    ui->img_src->setPixmap(QPixmap::fromImage(img));
}

void MainWindow::on_delta_btn_clicked()
{
    if (!is_split || !is_mono) {
        return;
    }
    ui->img_src->clear();
    int w, h;
    w = img.width();
    h = img.height() / 4;
    QImage new_img = QImage(w, h * 4, QImage::Format_RGB32);
    int x, y, z;
    for (z=0; z<4; z++) {
        int src = qBlue(img.pixel(0, h * z));
        QRgb dst = qRgb(src, src, src);
        new_img.setPixel(0, h * z, dst);
    }
    for (x=1; x<w; x++) {
        for (z=0; z<4; z++) {
            int px_l = qBlue(img.pixel(x - 1, h * z));
            int px_0 = qBlue(img.pixel(x    , h * z));
            int px_p = 255 & (px_0 - px_l + 127);
            QRgb px = qRgb(px_p, px_p, px_p);
            new_img.setPixel(x, h * z, px);
        }
    }
    for (y=1; y<h; y++) {
        for (z=0; z<4; z++) {
            int px_t = qBlue(img.pixel(0, y + h * z - 1));
            int px_0 = qBlue(img.pixel(0, y + h * z));
            int px_p = 255 & (px_0 - px_t + 127);
            QRgb px = qRgb(px_p, px_p, px_p);
            new_img.setPixel(0, y + h * z, px);
        }
    }
    for (y=1; y<h; y++) {
        for (x=1; x<w; x++) {
            for (z=0; z<4; z++) {
                int px_l = qBlue(img.pixel(x - 1, y + h * z));
                int px_t = qBlue(img.pixel(x    , y + h * z - 1));
                int px_x = qBlue(img.pixel(x - 1, y + h * z - 1));
                int px_0 = qBlue(img.pixel(x    , y + h * z));
                int px_p = 255 & (px_0 - (px_x * 2 + px_t * 3 + px_l * 3) / 8 + 127);
                QRgb px = qRgb(px_p, px_p, px_p);
                new_img.setPixel(x, y + h * z, px);
            }
        }
    }
    img = new_img;
    ui->img_src->setPixmap(QPixmap::fromImage(img));
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
    int old   = -1;
    int older = -2;
    int y, x;
    for (y=0; y<h; y++) {
        for (x=0; x<w; x++) {
           QRgb npx;
           int cur = qBlue(img.pixel(x, y));
           if (cur == old) {
               if (cur == older) {
                   npx = qRgb(255, 255, 0);
               } else {
                   npx = qRgb(0, 0, 255);
               }
           } else {
               npx = qRgb(0, 0, 0);
           }
           marks.setPixel(x, y, npx);
           older = old;
           old = cur;
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
            unsigned char byte;
            int pre;
            pre = qBlue(img.pixel(x, y));
            byte = (unsigned char)(255 & pre);
            out << byte;
        }
    }
    file.close();
}
