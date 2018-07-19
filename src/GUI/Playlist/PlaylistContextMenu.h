#ifndef PLAYLISTCONTEXTMENU_H
#define PLAYLISTCONTEXTMENU_H

#include "GUI/Utils/ContextMenu/LibraryContextMenu.h"

class PlaylistContextMenu :
		public LibraryContextMenu
{
	Q_OBJECT
	PIMPL(PlaylistContextMenu)

signals:
	void sig_rating_changed(Rating rating);

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

private:
	QAction* init_rating_action(Rating rating, QObject* parent);
	void language_changed() override;
	void skin_changed() override;
};


#endif // PLAYLISTCONTEXTMENU_H
