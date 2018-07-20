#ifndef BOOKMARKSBASE_H
#define BOOKMARKSBASE_H

#include <QObject>
#include <QList>
#include "Utils/Pimpl.h"
#include "Bookmark.h"

class MetaData;
class BookmarksBase :
		public QObject
{
	Q_OBJECT
	PIMPL(BookmarksBase)

public:
	enum class CreationStatus : unsigned char
	{
		Success,
		AlreadyThere,
		NoDBTrack,
		DBError,
		OtherError
	};

	explicit BookmarksBase(QObject* parent);
	virtual ~BookmarksBase();

	/**
	 * @brief create a new bookmark for current track and current position
	 * @return true if successful, else false
	 */
	virtual CreationStatus create(Seconds timestamp);

	virtual bool load();

	/**
	 * @brief remove single bookmark from database for current track
	 * @param idx index
	 * @return
	 */
	virtual bool remove(int idx);

	/**
	 * @brief get the current track
	 * @return
	 */
	MetaData metadata() const;
	void set_metadata(const MetaData& md);

	const QList<Bookmark> bookmarks() const;
	void set_bookmarks(const QList<Bookmark> bookmarks);

	int count();
	void add(const Bookmark& bookmark);
	void clear();

	const Bookmark& bookmark(int idx) const;
	Bookmark& bookmark(int idx);

	void sort();
};

#endif // BOOKMARKSBASE_H
