#include "image_helper.hxx"

image_helper::image_helper(QImage img)
{
	this->src = img;
}

image_helper::~image_helper()
{
	/* No public alocations. */
}

QPixmap image_helper::get_src()
{
	return QPixmap::fromImage(this->src);
}
