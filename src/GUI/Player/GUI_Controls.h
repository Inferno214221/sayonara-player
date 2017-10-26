#ifndef GUI_CONTROLS_H
#define GUI_CONTROLS_H

#include <QWidget>

#include "Components/PlayManager/PlayState.h"

#include "Utils/Pimpl.h"
#include "Utils/MetaData/RadioMode.h"
#include "GUI/Utils/GUIClass.h"
#include "GUI/Utils/Shortcuts/ShortcutWidget.h"
#include "GUI/Utils/Widgets/Widget.h"

UI_FWD(GUI_Controls)

class MetaData;
class MetaDataList;

class GUI_Controls :
		public Gui::Widget,
		public ShortcutWidget
{
	Q_OBJECT
	PIMPL(GUI_Controls)
	UI_CLASS(GUI_Controls)

private:
	void played();
	void paused();
	void stopped();

	void check_record_button_visible();

	void set_cover_location(const MetaData &md);
	void set_standard_cover();
	void set_radio_mode(RadioMode radio);

	void setup_volume_button(int percent);
	void increase_volume();
	void decrease_volume();

	void set_cur_pos_label(int val);
	void set_total_time_label(int64_t total_time);
	void set_info_labels(const MetaData &md);
	void refresh_info_labels();

	void setup_shortcuts();
	void setup_connections();


protected:
	void resizeEvent(QResizeEvent* e) override;
	void showEvent(QShowEvent* e) override;

public:
	explicit GUI_Controls(QWidget *parent = 0);
	~GUI_Controls();

	QString get_shortcut_text(const QString &shortcut_identifier) const override;
	void language_changed() override;
	void skin_changed() override;

	void file_info_changed();
	void sr_active_changed();

public slots:
	void change_volume_by_tick(int val);

private slots:
	void playstate_changed(PlayState state);

	void play_clicked();
	void stop_clicked();

	void prev_clicked();
	void next_clicked();

	void rec_clicked(bool b);
	void rec_changed(bool b);

	void buffering(int progress);

	void cur_pos_changed(uint64_t pos_ms);
	void progress_moved(int val);
	void progress_hovered(int val);

	void volume_slider_moved(int val);
	void volume_changed(int val);

	void mute_button_clicked();
	void mute_changed(bool muted);

	void track_changed(const MetaData& md);
	void id3_tags_changed(const MetaDataList &v_md_old, const MetaDataList &v_md_new);
	void md_changed(const MetaData &md);
	void dur_changed(const MetaData &md);
	void br_changed(const MetaData &md);

	void cover_changed(const QImage &img);

};

#endif // GUI_CONTROLS_H
