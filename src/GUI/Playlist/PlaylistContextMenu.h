#ifndef PLAYLISTCONTEXTMENU_H
#define PLAYLISTCONTEXTMENU_H

#include "GUI/Utils/ContextMenu/LibraryContextMenu.h"

class MetaData;
class PlaylistContextMenu :
		public LibraryContextMenu
{
	Q_OBJECT
	PIMPL(PlaylistContextMenu)

signals:
	void sig_rating_changed(Rating rating);
	void sig_bookmark_pressed(Seconds timestamp);

public:
	enum Entry
	{
		EntryRating=(LibraryContextMenu::EntryLast << 1),
		EntryBookmarks=(EntryRating << 1)
	};

	using Entries=LibraryContextMenu::Entries;

	explicit PlaylistContextMenu(QWidget* parent);
	~PlaylistContextMenu();

	PlaylistContextMenu::Entries get_entries() const override;
	void show_actions(PlaylistContextMenu::Entries entries) override;

	/**
	 * @brief set rating for the rating entry
	 * @param rating from 0 to 5
	 */
	void set_rating(Rating rating);
	void set_metadata(const MetaData& md);

private:
	QAction* init_rating_action(Rating rating, QObject* parent);
	void language_changed() override;
	void skin_changed() override;

private slots:
	void bookmark_pressed(Seconds timestamp);
};


#endif // PLAYLISTCONTEXTMENU_H
