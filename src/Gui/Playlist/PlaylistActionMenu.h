#ifndef PLAYLISTACTIONMENU_H
#define PLAYLISTACTIONMENU_H

#include <QMenu>
#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Utils/Pimpl.h"

class PlaylistActionMenu :
	public Gui::WidgetTemplate<QMenu>
{
	Q_OBJECT
	PIMPL(PlaylistActionMenu)

public:
	PlaylistActionMenu(QWidget* parent=nullptr);
	~PlaylistActionMenu();

	void check_dynamic_play_button();

private slots:
	void rep1_checked(bool checked);
	void rep_all_checked(bool checked);
	void shuffle_checked(bool checked);
	void playlist_mode_changed();
	void gapless_clicked();

	void language_changed() override;

	void s_playlist_mode_changed();
};

#endif // PLAYLISTACTIONMENU_H
