#ifndef IMAGE_HXX
#define IMAGE_HXX

#include <QVector>
#include <QImage>
#include <QPixmap>

class image_helper: public QObject
{
	Q_OBJECT

public:
	image_helper(QImage img);
	~image_helper();
	void reset();
	QPixmap get_src();

private:
	QImage src;
};

#endif // IMAGE_HXX
