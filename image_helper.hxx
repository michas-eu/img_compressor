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
	QPixmap get_gray();
	void proc_colours();
	void proc_spatial();

private:
	QImage src;
	bool planes_ready;
	QVector<quint16> plane_1;
	QVector<quint16> plane_2;
	QVector<quint16> plane_3;
	QVector<quint16> plane_0;
	void reset_planes();
};

#endif // IMAGE_HXX
