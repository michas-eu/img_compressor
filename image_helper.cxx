#include "image_helper.hxx"

static void predict_spatial(int w, int h, QVector<quint16> in, QVector<quint16> *out);
static void predict_spatial_top(int w, QVector<quint16> in, QVector<quint16> *out);
static void predict_spatial_left(int w, int h, QVector<quint16> in, QVector<quint16> *out);
static void predict_spatial_main(int w, int h, QVector<quint16> in, QVector<quint16> *out);
static void adv_point(QPoint *src, int w);

image_helper::image_helper(QImage img)
{
	this->src = img;
	this->reset();
}

image_helper::~image_helper()
{
	/* No public alocations. */
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
				this->visualise.setPixel(pt,black);
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

void image_helper::proc_colours()
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

			qint16 l, o, p;
			qint16 g, r, b;

			g = this->plane_1[i] >> 1;
			r = this->plane_2[i] >> 1;
			b = this->plane_3[i] >> 1;

			l = (g / 2 + r / 4 + b / 4);
			o = (r - b);
			p = (r / 2 + b / 2 - g);

			l <<= 1;
			o += 0xff;
			p += 0xff;

			this->plane_1[i] = l;
			this->plane_2[i] = o;
			this->plane_3[i] = p;
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
	int size = this->plane_0.size();
	int w = src.width();
	int h = src.height();

	/* Set temporary tables. */
	QVector<quint16> tmp_1;
	QVector<quint16> tmp_2;
	QVector<quint16> tmp_3;
	QVector<quint16> tmp_0;
	tmp_1.resize(size);
	tmp_2.resize(size);
	tmp_3.resize(size);
	tmp_0.resize(size);

	/* Predict pixels. */
	predict_spatial(w, h, this->plane_1, &tmp_1);
	predict_spatial(w, h, this->plane_2, &tmp_2);
	predict_spatial(w, h, this->plane_3, &tmp_3);
	predict_spatial(w, h, this->plane_0, &tmp_0);

	this->plane_1.clear();
	this->plane_2.clear();
	this->plane_3.clear();
	this->plane_0.clear();

	this->plane_1 = tmp_1;
	this->plane_2 = tmp_2;
	this->plane_3 = tmp_3;
	this->plane_0 = tmp_0;
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
			if (e >= 0x101 && e <= 0x110) {
				ret += qMakePair('+', e - 0x101);
			} else if (e <= 0xff && e >= 0xf0) {
				ret += qMakePair('-', 0xff - e);
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

static void predict_spatial(int w, int h, QVector<quint16> in, QVector<quint16> *out)
{
	/* Just copy for now top left. */
	(*out)[0] = in[0];
	predict_spatial_top(w, in, out);
	predict_spatial_left(w, h, in, out);
	predict_spatial_main(w, h, in, out);
}

static void predict_spatial_top(int w, QVector<quint16> in, QVector<quint16> *out)
{
	int x;
	for (x=1; x<w; x++) {
		/* Predict from left pixel. */
		int px_l, px_0, px_p;

		px_l = in[x-1];
		px_0 = in[x];
		px_p = 0x1ff & (px_0 - px_l + 0x100);
		(*out)[x] = px_p;
	}
}

static void predict_spatial_left(int w, int h, QVector<quint16> in, QVector<quint16> *out)
{
	int y;
	for (y=1; y<h; y++) {
		/* Predict from top pixel. */
		int i = y * w;
		int px_t, px_0, px_p;

		px_t = in[i - w];
		px_0 = in[i];
		px_p = 0x1ff & (px_0 - px_t + 0x100);
		(*out)[i] = px_p;
	}
}

static void predict_spatial_main(int w, int h, QVector<quint16> in, QVector<quint16> *out)
{
	int x, y;
	/* Predict rest of pixels. */
	for (y=1; y<h; y++) {
		for (x=1; x<w; x++) {
			int i = y * w + x;
			int px_t, px_l, px_x, px_0, px_p;
			px_t = in[i - w];
			px_l = in[i - 1];
			px_x = in[i - w - 1];
			px_0 = in[i];

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
			(*out)[i] = px_p;
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
