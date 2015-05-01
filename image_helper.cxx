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
