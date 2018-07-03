#ifndef GUI_CONTROLSBASE_H
#define GUI_CONTROLSBASE_H

#include "GUI/Utils/Shortcuts/ShortcutWidget.h"
#include "GUI/Utils/Widgets/Widget.h"
#include "GUI/InfoDialog/InfoDialogContainer.h"
#include "GUI/Utils/Icons.h"

#include "Components/PlayManager/PlayState.h"

#include "Utils/Pimpl.h"
#include "Utils/MetaData/RadioMode.h"

class QLabel;
class QSlider;
class QPushButton;
class CoverButton;
class SearchSlider;

namespace Gui
{
	class ProgressBar;
}

class GUI_ControlsBase :
		public Gui::Widget,
		public ShortcutWidget,
		public InfoDialogContainer
{
	Q_OBJECT
	PIMPL(GUI_ControlsBase)

public:
	GUI_ControlsBase(QWidget* parent=nullptr);
	virtual ~GUI_ControlsBase();
	void init();

	virtual QLabel* lab_sayonara() const=0;
	virtual QLabel* lab_title() const=0;
	virtual QLabel* lab_version() const=0;
	virtual QLabel* lab_album() const=0;
	virtual QLabel* lab_artist() const=0;
	virtual QLabel* lab_writtenby() const=0;
	virtual QLabel* lab_bitrate() const=0;
	virtual QLabel* lab_filesize() const=0;
	virtual QLabel* lab_copyright() const=0;
	virtual QLabel* lab_current_time() const=0;
	virtual QLabel* lab_max_time() const=0;
	virtual QWidget* widget_details() const=0;

	virtual SearchSlider* sli_progress() const=0;
	virtual SearchSlider* sli_volume() const=0;
	virtual Gui::ProgressBar* sli_buffer() const=0;
	virtual QPushButton* btn_mute() const=0;
	virtual QPushButton* btn_play() const=0;
	virtual QPushButton* btn_rec() const=0;
	virtual QPushButton* btn_bwd() const=0;
	virtual QPushButton* btn_fwd() const=0;
	virtual QPushButton* btn_stop() const=0;
	virtual CoverButton* btn_cover() const=0;

	virtual void toggle_buffer_mode(bool buffering)=0;
	virtual bool is_resizable() const=0;


private:
	QIcon icon(Gui::Icons::IconName name);

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
	void set_total_time_label(MilliSeconds total_time);
	void set_info_labels(const MetaData &md);
	void refresh_info_labels();

	void setup_shortcuts();
	void setup_connections();


protected:

	void resizeEvent(QResizeEvent* e) override;
	void showEvent(QShowEvent* e) override;
	void contextMenuEvent(QContextMenuEvent* e) override;

public:
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

	void cur_pos_changed(MilliSeconds pos_ms);
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

	// cover changed by engine
	void force_cover(const QImage &img);


	// InfoDialogContainer interface
protected:
	MD::Interpretation metadata_interpretation() const override;
	MetaDataList info_dialog_data() const override;
};

#endif // GUI_CONTROLSBASE_H
