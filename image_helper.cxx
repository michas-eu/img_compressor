#include "image_helper.hxx"

static void predict_spatial(int w, int h, QVector<quint16> *plane);
static void predict_spatial_top(int w, QVector<quint16> *plane);
static void predict_spatial_left(int w, int h, QVector<quint16> *plane);
static void predict_spatial_main(int w, int h, QVector<quint16> *plane);
static void adv_point(QPoint *src, int w);
static char from_safe_int(int i);

image_helper::image_helper(QImage img)
{
	this->src = img;
	this->reset();
}

image_helper::~image_helper()
{
	/* No public alocations. */
}

QPair<int, int> image_helper::get_wh()
{
	return qMakePair(this->src.width(), this->src.height());
}

QPixmap image_helper::get_src()
{
	return QPixmap::fromImage(this->src);
}

QPixmap image_helper::get_gray()
{
	if (!this->planes_ready) {
		this->reset_planes();
	}

	/* Get size. */
	int w = src.width();
	int h = src.height();

	QImage tmp = QImage(w, h * 4, QImage::Format_RGB32);
	/* Iterate and set planes. */
	int x, y;
	for (y=0; y<h; y++) {
		for (x=0; x<w; x++) {
			quint16 val;
			QRgb px;
			int i = x + y * w;

			/* plane_1 */
			val = this->plane_1[i] >> 1;
			px = qRgb(val, val, val);
			tmp.setPixel(x, y, px);

			/* plane_2 */
			val = this->plane_2[i] >> 1;
			px = qRgb(val, val, val);
			tmp.setPixel(x, y + h, px);

			/* plane_3 */
			val = this->plane_3[i] >> 1;
			px = qRgb(val, val, val);
			tmp.setPixel(x, y + h * 2, px);

			/* plane_0 */
			val = this->plane_0[i] >> 1;
			px = qRgb(val, val, val);
			tmp.setPixel(x, y + h * 3, px);
		}
	}

	return QPixmap::fromImage(tmp);
}

QPixmap image_helper::get_visualise()
{
	if (this->visualise.isNull()) {
		QList< QPair<char, int> > tmp = this->analyse();

		/* Get size. */
		int w = src.width();
		int h = src.height();

		this->visualise = QImage(w, h*4, QImage::Format_RGB32);

		QPair<char, int> e;
		QPoint pt = QPoint(0, 0);
		QRgb black = qRgb(0,0,0);
		QRgb blue = qRgb(0,0,255);
		QRgb green = qRgb(0,128,0);
		QRgb yellow = qRgb(255,255,0);
		QRgb red = qRgb(255,0,0);
		while (!tmp.isEmpty()) {
			e = tmp.takeFirst();
			if (e.first == '+' || e.first == '-') {
				this->visualise.setPixel(pt,black);
				adv_point(&pt, w);
			} else if (e.first == 'r') {
				this->visualise.setPixel(pt,yellow);
				adv_point(&pt, w);
			} else if (e.first == '0' && e.second == 1) {
				this->visualise.setPixel(pt,blue);
				adv_point(&pt, w);
			} else if (e.first == '0' && e.second > 1) {
				int i;
				for (i = 0; i < e.second; i++) {
					this->visualise.setPixel(pt,green);
					adv_point(&pt, w);
				}
			} else {
				/* This is an error. */
				this->visualise.setPixel(pt,red);
				adv_point(&pt, w);
			}
		}
	}
	return QPixmap::fromImage(this->visualise);
}

QByteArray image_helper::get_string()
{
	QList< QPair<char, int> > tmp = this->analyse();
	QPair<char, int> e;
	QByteArray ret("");

	int t1, t2, t3;
	while (!tmp.isEmpty()) {
		e = tmp.takeFirst();
		if (e.first == '+' && e.second <= 5) {
			t1 = e.second;
			ret += from_safe_int(t1);
		} else if (e.first == '+' && e.second <= 0x15) {
			t1 = 6;
			t2 = e.second - 6;
			ret += from_safe_int(t1);
			ret += from_safe_int(t2);
		} else if (e.first == '-' && e.second <=5) {
			t1 = 0x10 - e.second;
			ret += from_safe_int(t1);
		} else if (e.first == '-' && e.second <=0x15) {
			t1 = 0xa;
			t2 = e.second - 6;
			ret += from_safe_int(t1);
			ret += from_safe_int(t2);
		} else if (e.first == 'r') {
			if (e.second <= 0xff) {
				t1 = 8;
			} else {
				t1 = 9;
			}
			t2 = (e.second & 0xf0) >> 4;
			t3 = e.second & 0xf;
			ret += from_safe_int(t1);
			ret += from_safe_int(t2);
			ret += from_safe_int(t3);
		} else if (e.first == '0' && e.second == 1) {
			t1 = 0;
			ret += from_safe_int(t1);
		} else if (e.first == '0' && e.second < 0x10) {
			t1 = 7;
			t2 = e.second;
			ret += from_safe_int(t1);
			ret += from_safe_int(t2);
		} else if (e.first == '0') {
			tmp.prepend(qMakePair('0',e.second & 0xf));
			tmp.prepend(qMakePair('0',e.second >> 4));
		} else {
			/* This is an error. */
			ret += from_safe_int(0xff);
		}
	}
	return ret;
}

void image_helper::proc_colours(bool reverse)
{
	if (!this->planes_ready) {
		this->reset_planes();
	}

	this->visualise = QImage();

	/* Get size. */
	int w = src.width();
	int h = src.height();
	int x, y;

	for (y=0; y<h; y++) {
		for (x=0; x<w; x++) {
			int i = x + y * w;
			if (reverse) {
				this->color_to_grba(i);
			} else {
				this->color_to_lopa(i);
			}
		}
	}
}

void image_helper::proc_spatial()
{
	if (!this->planes_ready) {
		this->reset_planes();
	}

	this->visualise = QImage();

	/* Get size. */
	int w = src.width();
	int h = src.height();

	/* Predict pixels. */
	predict_spatial(w, h, &this->plane_1);
	predict_spatial(w, h, &this->plane_2);
	predict_spatial(w, h, &this->plane_3);
	predict_spatial(w, h, &this->plane_0);
}

void image_helper::reset()
{
	this->planes_ready = false;
	this->visualise = QImage();
}

void image_helper::reset_planes()
{
	this->visualise = QImage();

	/* Release memory and drop old data. */
	this->plane_1.clear();
	this->plane_2.clear();
	this->plane_3.clear();
	this->plane_0.clear();

	/* Get size. */
	int w = src.width();
	int h = src.height();

	/* Allocate memory for planes. */
	this->plane_1.reserve(w*h);
	this->plane_2.reserve(w*h);
	this->plane_3.reserve(w*h);
	this->plane_0.reserve(w*h);

	/* Iterate and set planes. */
	int x, y;
	for (y=0; y<h; y++) {
		for (x=0; x<w; x++) {
			QRgb opx = src.pixel(x, y);
			int px_a, px_g, px_r, px_b;
			px_a = qAlpha(opx) << 1;
			px_g = qGreen(opx) << 1;
			px_r = qRed(opx) << 1;
			px_b = qBlue(opx) << 1;
			this->plane_1 += px_g;
			this->plane_2 += px_r;
			this->plane_3 += px_b;
			this->plane_0 += px_a;
		}
	}

	this->planes_ready = true;
}

QList<quint16> image_helper::get_raw()
{
	if (!this->planes_ready) {
		this->reset_planes();
	}

	QList<quint16> ret;
	ret = QList<quint16>::fromVector(this->plane_1);
	ret+= QList<quint16>::fromVector(this->plane_2);
	ret+= QList<quint16>::fromVector(this->plane_3);
	ret+= QList<quint16>::fromVector(this->plane_0);
	return ret;
}

QList< QPair<char, int> > image_helper::analyse()
{
	QList<quint16> tmp = this->get_raw();
	QList< QPair<char, int> > ret;
	int zeros = 0;

	while (!tmp.isEmpty()) {
		quint16 e = tmp.takeFirst();
		if (e == 0x100) {
			zeros++;
		} else {
			if (zeros) {
				ret += qMakePair('0', zeros);
				zeros = 0;
			}
			if (e > 0x100 && e <= (0x100 + 0x15)) {
				ret += qMakePair('+', e - 0x100);
			} else if (e < 0x100 && e >= (0x100 - 0x15)) {
				ret += qMakePair('-', 0x100 - e);
			} else {
				ret += qMakePair('r', e);
			}
		}
	}
	if (zeros) {
		ret += qMakePair('0', zeros);
	}

	return ret;
}

void image_helper::color_to_lopa(int i)
{
	qint16 l, o, p;
	qint16 g, r, b;
	qint16 t;

	g = this->plane_1[i] >> 1;
	r = this->plane_2[i] >> 1;
	b = this->plane_3[i] >> 1;

	o = r - b;
	t = b + o / 2;
	p = t - g;
	l = t - p / 2;

	l <<= 1;
	o += 0xff;
	p += 0xff;

	this->plane_1[i] = l;
	this->plane_2[i] = o;
	this->plane_3[i] = p;
}

void image_helper::color_to_grba(int i)
{
	qint16 g, r, b;
	qint16 l, o, p;
	qint16 t;

	l = this->plane_1[i] >> 1;
	o = this->plane_2[i] - 0xff;
	p = this->plane_3[i] - 0xff;

	t = l + p / 2;
	g = t - p;
	b = t - o / 2;
	r = o + b;

	g <<= 1;
	r <<= 1;
	b <<= 1;

	this->plane_1[i] = g;
	this->plane_2[i] = r;
	this->plane_3[i] = b;
}

static void predict_spatial(int w, int h, QVector<quint16> *plane)
{
	predict_spatial_main(w, h, plane);
	predict_spatial_left(w, h, plane);
	predict_spatial_top(w, plane);
}

static void predict_spatial_top(int w, QVector<quint16> *plane)
{
	int x;
	for (x=w-1; x>0; x--) {
		/* Predict from left pixel. */
		int px_l, px_0, px_p;

		px_l = (*plane)[x-1];
		px_0 = (*plane)[x];
		px_p = 0x1ff & (px_0 - px_l + 0x100);
		(*plane)[x] = px_p;
	}
}

static void predict_spatial_left(int w, int h, QVector<quint16> *plane)
{
	int y;
	for (y=h-1; y>0; y--) {
		/* Predict from top pixel. */
		int i = y * w;
		int px_t, px_0, px_p;

		px_t = (*plane)[i - w];
		px_0 = (*plane)[i];
		px_p = 0x1ff & (px_0 - px_t + 0x100);
		(*plane)[i] = px_p;
	}
}

static void predict_spatial_main(int w, int h, QVector<quint16> *plane)
{
	int x, y;
	/* Predict rest of pixels. */
	for (y=h-1; y>0; y--) {
		for (x=w-1; x>0; x--) {
			int i = y * w + x;
			int px_t, px_l, px_x, px_0, px_p;
			px_t = (*plane)[i - w];
			px_l = (*plane)[i - 1];
			px_x = (*plane)[i - w - 1];
			px_0 = (*plane)[i];

			/* Pixel art friendly. */
			if (px_x == px_t) {
				px_p = px_l;
			} else if (px_x == px_l) {
				px_p = px_t;
			} else if (px_t == px_l) {
				px_p = px_x;
			} else {
				px_p = (px_t * 3 + px_l * 3 + px_x * 2) / 8;
			}
			px_p = 0x1ff & (px_0 - px_p + 0x100);
			(*plane)[i] = px_p;
		}
	}
}

static void adv_point(QPoint *src, int w) {
	src->rx()++;
	if (src->rx() == w) {
		src->ry() ++;
		src->rx() = 0;
	}
}

static char from_safe_int(int i) {
	if (i >= 0 && i < 16) {
		return i + 'a';
	} else {
		return 126;
	}
/* May be usefull later. */
#if 0
	QVector<char> map(64);
	map[0]  = 'A';
	map[1]  = 'B';
	map[2]  = 'C';
	map[3]  = 'D';
	map[4]  = 'E';
	map[5]  = 'F';
	map[6]  = 'G';
	map[7]  = 'H';
	map[8]  = 'I';
	map[9]  = 'J';
	map[10] = 'K';
	map[11] = 'L';
	map[12] = 'M';
	map[13] = 'N';
	map[14] = 'O';
	map[15] = 'P';
	map[16] = 'Q';
	map[17] = 'R';
	map[18] = 'S';
	map[19] = 'T';
	map[20] = 'U';
	map[21] = 'V';
	map[22] = 'W';
	map[23] = 'X';
	map[24] = 'Y';
	map[25] = 'Z';
	map[26] = 'a';
	map[27] = 'b';
	map[28] = 'c';
	map[29] = 'd';
	map[30] = 'e';
	map[31] = 'f';
	map[32] = 'g';
	map[33] = 'h';
	map[34] = 'i';
	map[35] = 'j';
	map[36] = 'k';
	map[37] = 'l';
	map[38] = 'm';
	map[39] = 'n';
	map[40] = 'o';
	map[41] = 'p';
	map[42] = 'q';
	map[43] = 'r';
	map[44] = 's';
	map[45] = 't';
	map[46] = 'u';
	map[47] = 'v';
	map[48] = 'w';
	map[49] = 'x';
	map[50] = 'y';
	map[51] = 'z';
	map[52] = '0';
	map[53] = '1';
	map[54] = '2';
	map[55] = '3';
	map[56] = '4';
	map[57] = '5';
	map[58] = '6';
	map[59] = '7';
	map[60] = '8';
	map[61] = '9';
	map[62] = '+';
	map[63] = '/';
	return map.value(i, '*');
#endif
}
