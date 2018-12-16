#ifndef TAGGINGCOVER_H
#define TAGGINGCOVER_H

class QPixmap;
class QString;
class QByteArray;

namespace Tagging
{
	namespace Covers
	{
		bool write_cover(const QString& filepath, const QPixmap& image);
		bool write_cover(const QString& filepath, const QString& image_path);
		bool extract_cover(const QString& filepath, QByteArray& cover_data, QString& mime_type);
		QPixmap extract_cover(const QString& filepath);
		bool has_cover(const QString& filepath);
		bool is_cover_supported(const QString& filepath);
	}
}

#endif // TAGGINGCOVER_H
