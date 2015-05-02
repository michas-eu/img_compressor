#ifndef IMAGE_HXX
#define IMAGE_HXX

#include <QList>
#include <QVector>
#include <QPair>
#include <QPoint>
#include <QByteArray>
#include <QImage>
#include <QPixmap>

class image_helper: public QObject
{
	Q_OBJECT

public:
	image_helper(QImage img);
	~image_helper();
	void reset();
	QPair<int,int> get_wh();
	QPixmap get_src();
	QPixmap get_gray();
	QPixmap get_visualise();
	QByteArray get_string();
	void proc_colours(bool reverse=false);
	void proc_spatial();

private:
	QImage src;
	QImage visualise;
	bool planes_ready;
	QVector<quint16> plane_1;
	QVector<quint16> plane_2;
	QVector<quint16> plane_3;
	QVector<quint16> plane_0;
	void reset_planes();
	QList<quint16> get_raw();
	QList< QPair<char, int> > analyse();
	void color_to_lopa(int i);
	void color_to_grba(int i);
};

#endif // IMAGE_HXX
