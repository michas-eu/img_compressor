#include "mainwindow.hxx"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    phase = 0;
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
    phase = 1;
}

void MainWindow::on_split_btn_clicked()
{
    if (phase < 1) {
        return;
    }
    ui->img_src->clear();
    int w, h;
    w = img.width();
    h = img.height();
    QImage new_img = QImage(w, h * 5, QImage::Format_RGB32);
    int x, y;
    for (y=0; y<h; y++) {
        for (x=0; x<w; x++) {
            QRgb opx = img.pixel(x, y);
            QRgb px_a, px_k, px_g, px_r, px_b;
            px_a = qAlpha(opx);
            px_k = qGray(opx);
            px_g = qGreen(opx);
            px_r = qRed(opx);
            px_b = qBlue(opx);
            px_a = qRgb(px_a, px_a, px_a);
            px_k = qRgb(px_k, px_k, px_k);
            px_g = qRgb(px_g, px_g, px_g);
            px_r = qRgb(px_r, px_r, px_r);
            px_b = qRgb(px_b, px_b, px_b);
            new_img.setPixel(x, y + h * 0, px_k);
            new_img.setPixel(x, y + h * 1, px_g);
            new_img.setPixel(x, y + h * 2, px_r);
            new_img.setPixel(x, y + h * 3, px_b);
            new_img.setPixel(x, y + h * 4, px_a);
        }
    }
    img = new_img;
    phase = 2;
    ui->img_src->setPixmap(QPixmap::fromImage(img));
}

void MainWindow::on_predict_btn_clicked()
{
    if (phase < 2) {
        return;
    }
    ui->img_src->clear();
    int w, h;
    w = img.width();
    h = img.height() / 5;
    int x, y;
    for (y=0; y<h; y++) {
        for (x=0; x<w; x++) {
            int k = qBlue(img.pixel(x, y + h * 0));
            int g = qBlue(img.pixel(x, y + h * 1));
            int r = qBlue(img.pixel(x, y + h * 2));
            int b = qBlue(img.pixel(x, y + h * 3));
            int tmp;
            tmp = (k * 32 - g * 16 - r * 11 + 3) / 5;
            b = 255 & (tmp - b + 127);
            tmp = (k * 2 - g);
            r = 255 & (r - tmp + 127);
            tmp = k;
            g = 255 & (g - tmp + 127);
            QRgb px_g, px_r, px_b;
            px_g = qRgb(g, g, g);
            px_r = qRgb(r, r, r);
            px_b = qRgb(b, b, b);
            img.setPixel(x, y + h * 1, px_g);
            img.setPixel(x, y + h * 2, px_r);
            img.setPixel(x, y + h * 3, px_b);
        }
    }
    phase = 3;
    ui->img_src->setPixmap(QPixmap::fromImage(img));
}

void MainWindow::on_delta_btn_clicked()
{
    if (phase < 2) {
        return;
    }
    ui->img_src->clear();
    int w, h;
    w = img.width();
    h = img.height() / 5;
    QImage new_img = QImage(w, h * 5, QImage::Format_RGB32);
    int x, y, z;
    for (z=0; z<5; z++) {
        int src = qBlue(img.pixel(0, h * z));
        QRgb dst = qRgb(src, src, src);
        new_img.setPixel(0, h * z, dst);
    }
    for (x=1; x<w; x++) {
        for (z=0; z<5; z++) {
            int px_l = qBlue(img.pixel(x - 1, h * z));
            int px_0 = qBlue(img.pixel(x    , h * z));
            int px_p = 255 & (px_0 - px_l + 127);
            QRgb px = qRgb(px_p, px_p, px_p);
            new_img.setPixel(x, h * z, px);
        }
    }
    for (y=1; y<h; y++) {
        for (z=0; z<5; z++) {
            int px_t = qBlue(img.pixel(0, y + h * z - 1));
            int px_0 = qBlue(img.pixel(0, y + h * z));
            int px_p = 255 & (px_0 - px_t + 127);
            QRgb px = qRgb(px_p, px_p, px_p);
            new_img.setPixel(0, y + h * z, px);
        }
    }
    for (y=1; y<h; y++) {
        for (x=1; x<w; x++) {
            for (z=0; z<5; z++) {
                int px_l = qBlue(img.pixel(x - 1, y + h * z));
                int px_t = qBlue(img.pixel(x    , y + h * z - 1));
                int px_x = qBlue(img.pixel(x - 1, y + h * z - 1));
                int px_0 = qBlue(img.pixel(x    , y + h * z));
                int px_p = 255 & (px_0 - (px_x + px_t * 2 + px_l * 2 + 3) / 5 + 127);
                QRgb px = qRgb(px_p, px_p, px_p);
                new_img.setPixel(x, y + h * z, px);
            }
        }
    }
    phase = 4;
    img = new_img;
    ui->img_src->setPixmap(QPixmap::fromImage(img));
}

void MainWindow::on_mark_btn_clicked()
{
    if (phase < 2) {
        return;
    }
    ui->img_src->clear();
    int w, h;
    w = img.width();
    h = img.height();
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
           img.setPixel(x, y, npx);
           older = old;
           old = cur;
        }
    }
    phase = 1;
    ui->img_src->setPixmap(QPixmap::fromImage(img));
}

void MainWindow::on_save_btn_clicked()
{
    if (phase < 2) {
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
