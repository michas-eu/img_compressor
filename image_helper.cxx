#include "image_helper.hxx"

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

void image_helper::proc_colours()
{
	if (!this->planes_ready) {
		this->reset_planes();
	}

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
	this->predict_spatial(w, h, this->plane_1, &tmp_1);
	this->predict_spatial(w, h, this->plane_2, &tmp_2);
	this->predict_spatial(w, h, this->plane_3, &tmp_3);
	this->predict_spatial(w, h, this->plane_0, &tmp_0);

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
}

void image_helper::reset_planes()
{
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

void image_helper::predict_spatial(int w, int h, QVector<quint16> in, QVector<quint16> *out)
{
	/* Just copy for now top left. */
	(*out)[0] = in[0];
	image_helper::predict_spatial_top(w, in, out);
	image_helper::predict_spatial_left(w, h, in, out);
	image_helper::predict_spatial_main(w, h, in, out);
}

void image_helper::predict_spatial_top(int w, QVector<quint16> in, QVector<quint16> *out)
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

void image_helper::predict_spatial_left(int w, int h, QVector<quint16> in, QVector<quint16> *out)
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

void image_helper::predict_spatial_main(int w, int h, QVector<quint16> in, QVector<quint16> *out)
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
