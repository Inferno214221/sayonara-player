#ifndef IMAGE_H
#define IMAGE_H

class QPixmap;
class QImage;
class QSize;

namespace Util
{
	class Image
	{

	private:
		struct Private;
		Private* m=nullptr;

	public:
		Image();
		Image(const QPixmap& pm);
		Image(const QPixmap& pm, const QSize& max_size);

		Image(const Image& other);
		~Image();

		Image& operator=(const Image& other);

		QPixmap pixmap() const;
	};
}

#endif // IMAGE_H
