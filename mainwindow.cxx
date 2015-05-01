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
    QImage new_img = QImage(w, h * 4, QImage::Format_RGB32);
    int x, y;

    for (y=0; y<h; y++) {
        for (x=0; x<w; x++) {
            int g = qBlue(img.pixel(x, y + h * 0));
            int r = qBlue(img.pixel(x, y + h * 1));
            int b = qBlue(img.pixel(x, y + h * 2));
            int o = (r - b + 128) & 0xff;
            int p = (r - o / 2 - g + 192) & 0xff;
            int l = (g + p / 2 - 64) & 0xff;
            QRgb px_g, px_r, px_b, px_a;
            px_g = qRgb(l, l, l);
            px_r = qRgb(o, o, o);
            px_b = qRgb(p, p, p);
            px_a = img.pixel(x, y + h * 3);
            new_img.setPixel(x, y + h * 0, px_g);
            new_img.setPixel(x, y + h * 1, px_r);
            new_img.setPixel(x, y + h * 2, px_b);
            new_img.setPixel(x, y + h * 3, px_a);
        }
    }
    img = new_img;
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

    /* Top left pixel. */
    for (z=0; z<4; z++) {
	/* No prediction. */
        int src = qBlue(img.pixel(0, h * z));
        QRgb dst = qRgb(src, src, src);
        new_img.setPixel(0, h * z, dst);
    }

    /* Top row of pixels. */
    for (x=1; x<w; x++) {
        for (z=0; z<4; z++) {
	    /* Predict from left pixel. */
            int px_l = qBlue(img.pixel(x - 1, h * z));
            int px_0 = qBlue(img.pixel(x    , h * z));
            int px_p = 0xff & (px_0 - px_l + 128);
            QRgb px = qRgb(px_p, px_p, px_p);
            new_img.setPixel(x, h * z, px);
        }
    }

    /* Left column of pixels. */
    for (y=1; y<h; y++) {
        for (z=0; z<4; z++) {
	    /* Predict from top pixel. */
            int px_t = qBlue(img.pixel(0, y + h * z - 1));
            int px_0 = qBlue(img.pixel(0, y + h * z));
            int px_p = 0xff & (px_0 - px_t + 128);
            QRgb px = qRgb(px_p, px_p, px_p);
            new_img.setPixel(0, y + h * z, px);
        }
    }

    /* The rest of pixels. */
    for (y=1; y<h; y++) {
        for (x=1; x<w; x++) {
            for (z=0; z<4; z++) {
                int px_l = qBlue(img.pixel(x - 1, y + h * z));
                int px_t = qBlue(img.pixel(x    , y + h * z - 1));
                int px_x = qBlue(img.pixel(x - 1, y + h * z - 1));
                int px_0 = qBlue(img.pixel(x    , y + h * z));
		int px_p;
		if (px_x >= px_l && px_x >= px_t) {
		    /* Expecting light line on top or left. */
		    px_p = px_l <= px_t ? px_l : px_t;
		} else if (px_x <= px_l && px_x <= px_t) {
		    /* Expecting dark line on top or left. */
		    px_p = px_l >= px_t ? px_l : px_t;
		} else {
		    /* Expecting gradient */
                    /* 2 * gradient + 3 * mean */
		    px_p = (px_t * 5 + px_l * 5 - px_x * 2) / 8;
		}
		px_p = 0xff & (px_0 - px_p + 128);
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
