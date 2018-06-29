#include "GUI_Controls.h"
#include "GUI/Player/ui_GUI_Controls.h"

#include "GUI_TrayIcon.h"
#include "GUI/Utils/GuiUtils.h"
#include "GUI/Utils/Icons.h"
#include "GUI/Utils/Shortcuts/Shortcut.h"
#include "GUI/Utils/Shortcuts/ShortcutHandler.h"
#include "GUI/Utils/Style.h"
#include "GUI/Utils/PreferenceAction.h"
#include "GUI/Utils/ContextMenu/LibraryContextMenu.h"

#include "Components/PlayManager/PlayManager.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Engine/EngineHandler.h"
#include "Components/Tagging/ChangeNotifier.h"

#include "Utils/Utils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Language.h"


#include <QToolTip>
#include <QPushButton>
#include <QImage>
#include <QPixmap>
#include <QIcon>
#include <QDateTime>

#include <algorithm>

using Cover::Location;

struct GUI_Controls::Private
{
	LibraryContextMenu* context_menu=nullptr;
};

GUI_Controls::GUI_Controls(QWidget* parent) :
	Gui::Widget(parent),
	ShortcutWidget()
{
	m = Pimpl::make<Private>();
	ui = new Ui::GUI_Controls();
	ui->setupUi(this);

	ui->btn_rec->setVisible(false);
//	ui->btn_cover->setVisible(false);

	PlayManager* play_manager = PlayManager::instance();
	int volume = play_manager->volume();
	volume_changed(volume);

	bool muted = play_manager->is_muted();
	mute_changed(muted);

	setup_connections();
	setup_shortcuts();

	playstate_changed(play_manager->playstate());

	if(play_manager->playstate() != PlayState::Stopped)
	{
		track_changed(play_manager->current_track());
		cur_pos_changed(play_manager->initial_position_ms());
	}

/*	connect(ui->btn_cover, &CoverButton::sig_rejected, this, [=](){
		show_edit();
	});*/

	Set::listen<Set::Engine_SR_Active>(this, &GUI_Controls::sr_active_changed);
	Set::listen<Set::Engine_Pitch>(this, &GUI_Controls::file_info_changed);
	Set::listen<Set::Engine_SpeedActive>(this, &GUI_Controls::file_info_changed, false);

	language_changed();
}

GUI_Controls::~GUI_Controls()
{
	if(ui){
		delete ui; ui=nullptr;
	}
}

// new track
void GUI_Controls::track_changed(const MetaData & md)
{
	ui->lab_sayonara->hide();
	ui->lab_title->show();

	ui->lab_version->hide();
	ui->lab_artist->show();

	ui->lab_writtenby->hide();
	ui->lab_album->show();

	ui->lab_copyright->hide();

	ui->lab_bitrate->show();
	ui->lab_filesize->show();
	ui->widget->show();

	set_info_labels(md);
	set_cur_pos_label(0);
	set_total_time_label(md.length_ms);
	file_info_changed();
	set_cover_location(md);
	set_radio_mode( md.radio_mode() );

	ui->sli_progress->setEnabled( (md.length_ms / 1000) > 0 );
}


void GUI_Controls::playstate_changed(PlayState state)
{
	switch(state)
	{
		case PlayState::Playing:
			played();
			break;
		case PlayState::Paused:
			paused();
			break;
		case PlayState::Stopped:
			stopped();
			break;
		default:
			break;
	}

	check_record_button_visible();
}

static QIcon get_icon(Gui::Icons::IconName name)
{
	using namespace Gui;

	Icons::IconMode mode = Icons::Automatic;

	if(Style::is_dark()){
		mode = Icons::ForceSayonaraIcon;
	}

	switch(name)
	{
		case Icons::Play:
		case Icons::Pause:
		case Icons::Stop:
		case Icons::Next:
		case Icons::Previous:
		case Icons::Forward:
		case Icons::Backward:
		case Icons::Record:
			mode = Icons::ForceSayonaraIcon;

		default:
			mode = Icons::Automatic;
	}

	return Icons::icon(name, mode);
}

void GUI_Controls::played()
{
	ui->btn_play->setIcon(get_icon(Gui::Icons::Pause));
}

void GUI_Controls::paused()
{
	ui->btn_play->setIcon(get_icon(Gui::Icons::Play));
}

void GUI_Controls::stopped()
{
	setWindowTitle("Sayonara");

	ui->btn_play->setIcon(get_icon(Gui::Icons::Play));

	ui->progress_widget->setCurrentIndex(0);

	ui->lab_title->hide();
	ui->lab_sayonara->show();

	ui->lab_artist->hide();
	ui->lab_writtenby->show();

	ui->lab_album->hide();
	ui->lab_version->show();

	ui->widget->hide();

	ui->lab_copyright->show();

	ui->sli_progress->setValue(0);
	ui->sli_progress->setEnabled(false);

	ui->lab_cur_time->setText("00:00");
	ui->lab_max_time->clear();

	set_standard_cover();
}

void GUI_Controls::rec_changed(bool b)
{
	ui->btn_rec->setChecked(b);
}

void GUI_Controls::buffering(int progress)
{
	ui->sli_buffer->set_position(Gui::ProgressBar::Position::Middle);

	if(progress > 0 && progress < 100)
	{
		ui->progress_widget->setCurrentIndex(1);
		ui->sli_buffer->setValue(progress);
		ui->lab_cur_time->setText(QString("%1%").arg(progress));
		ui->lab_max_time->hide();
	}

	else
	{
		ui->progress_widget->setCurrentIndex(0);
		ui->lab_max_time->show();
	}
}


void GUI_Controls::progress_moved(int val)
{
	val = std::max(val, 0);

	set_cur_pos_label(val);

	double percent = (val * 1.0) / ui->sli_progress->maximum();
	PlayManager::instance()->seek_rel(percent);
}


void GUI_Controls::cur_pos_changed(MilliSeconds pos_ms)
{
	MilliSeconds duration = PlayManager::instance()->duration_ms();
	int max = ui->sli_progress->maximum();
	int new_val;

	if ( duration > 0 ) {
		new_val = ( pos_ms * max ) / (duration);
	}

	else if(pos_ms > duration) {
		new_val = 0;
	}

	else{
		return;
	}

	if(!ui->sli_progress->is_busy())
	{
		QString cur_pos_string = Util::cvt_ms_to_string(pos_ms);
		ui->lab_cur_time->setText(cur_pos_string);
		ui->sli_progress->setValue(new_val);
	}
}

QString GUI_Controls::get_cur_pos_string(int val) const
{
	MilliSeconds duration = PlayManager::instance()->duration_ms();
	int max = ui->sli_progress->maximum();

	val = std::max(val, 0);
	val = std::min(max, val);

	double percent = (val * 1.0) / max;
	MilliSeconds cur_pos_ms =  (MilliSeconds) (percent * duration);

	return Util::cvt_ms_to_string(cur_pos_ms);
}



void GUI_Controls::set_total_time_label(MilliSeconds total_time)
{
	QString length_str;
	if(total_time > 0){
		length_str = Util::cvt_ms_to_string(total_time, true);
	}

	ui->lab_max_time->setText(length_str);
	ui->sli_progress->setEnabled(total_time > 0);
}

void GUI_Controls::set_cur_pos_label(int val)
{
	ui->lab_cur_time->setText(get_cur_pos_string(val));
}


void GUI_Controls::progress_hovered(int val)
{
	QToolTip::showText( QCursor::pos(), get_cur_pos_string(val) );
}

void GUI_Controls::volume_changed(int val)
{
	setup_volume_button(val);
	ui->sli_volume->setValue(val);
}

void GUI_Controls::setup_volume_button(int percent)
{
	using namespace Gui;

	QIcon icon;

	if (percent <= 1) {
		icon = Icons::icon(Icons::VolMute);
	}

	else if (percent < 40) {
		icon = Icons::icon(Icons::Vol1);
	}

	else if (percent < 80) {
		icon = Icons::icon(Icons::Vol2);
	}

	else {
		icon = Icons::icon(Icons::Vol3);
	}

	ui->btn_mute->setIcon(icon);
}

void GUI_Controls::change_volume_by_tick(int val)
{
	if(val > 0){
		PlayManager::instance()->volume_up();
	}
	else{
		PlayManager::instance()->volume_down();
	}
}

void GUI_Controls::mute_button_clicked()
{
	bool muted = PlayManager::instance()->is_muted();
	PlayManager::instance()->set_muted(!muted);
}


void GUI_Controls::mute_changed(bool muted)
{
	int val;
	ui->sli_volume->setDisabled(muted);

	if(muted){
		val = 0;
	}

	else {
		val = PlayManager::instance()->volume();
	}

	ui->sli_volume->setValue(val);
	setup_volume_button(val);
}


// public slot:
// id3 tags have changed
void GUI_Controls::id3_tags_changed(const MetaDataList& v_md_old, const MetaDataList& v_md_new)
{
	PlayManager* pm = PlayManager::instance();
	const MetaData& md = pm->current_track();

	IdxList idxs = v_md_old.findTracks(md.filepath());
	if(!idxs.empty())
	{
		const MetaData& md = v_md_new[idxs.first()];
		set_info_labels(md);
		set_cover_location(md);

		setWindowTitle(QString("Sayonara - ") + md.title());
	}
}


void GUI_Controls::md_changed(const MetaData& md)
{
	MetaData modified_md(md);

	if(md.radio_mode() == RadioMode::Station){
		modified_md.set_album(md.album() + " (" + md.filepath() + ")");
	}

	set_info_labels(modified_md);
}


void GUI_Controls::dur_changed(const MetaData& md)
{
	set_total_time_label(md.length_ms);
}

void GUI_Controls::br_changed(const MetaData& md)
{
	if(md.bitrate / 1000 > 0){
		QString bitrate = QString::number(md.bitrate / 1000) + " kBit/s";
		ui->lab_bitrate->setText(bitrate);
	}

	if(md.filesize > 0)
	{
		QString filesize = QString::number( (double) (md.filesize / 1024) / 1024.0, 'f', 2) + " MB";
		ui->lab_filesize->setText(filesize);
	}
}


void GUI_Controls::refresh_info_labels()
{
	set_info_labels(PlayManager::instance()->current_track());
}

void GUI_Controls::set_info_labels(const MetaData& md)
{
	// title
	QString text = Gui::Util::elide_text(md.title(), ui->lab_title, 2);
	ui->lab_title->setText(text);

	//album
	QString str_year = QString::number(md.year);
	QString album_name(md.album());

	if(md.year > 1000 && !album_name.contains(str_year)){
		album_name += " (" + str_year + ")";
	}

	QString elided_text;
	QFontMetrics fm_album = ui->lab_album->fontMetrics();
	elided_text = fm_album.elidedText(album_name, Qt::ElideRight, ui->lab_album->width());
	ui->lab_album->setText(elided_text);

	//artist
	QFontMetrics fm_artist = ui->lab_artist->fontMetrics();
	elided_text = fm_artist.elidedText(md.artist(), Qt::ElideRight, ui->lab_artist->width());
	ui->lab_artist->setText(elided_text);
}


void GUI_Controls::file_info_changed()
{
	const MetaData& md = PlayManager::instance()->current_track();

	QString rating_text;
	if( (_settings->get<Set::Engine_Pitch>() != 440) &&
		_settings->get<Set::Engine_SpeedActive>())
	{
		if(!rating_text.isEmpty()){
			rating_text += ", ";
		}

		rating_text += QString::number(_settings->get<Set::Engine_Pitch>()) + "Hz";
	}


	QString sBitrate;
	if(md.bitrate / 1000 > 0){
		sBitrate = QString::number(md.bitrate / 1000) + " kBit/s";
		ui->lab_bitrate->setText(sBitrate);
	}
	ui->lab_bitrate->setVisible(!sBitrate.isEmpty());


	QString sFilesize;
	if(md.filesize > 0)
	{
		sFilesize = QString::number( (double) (md.filesize / 1024) / 1024.0, 'f', 2) + " MB";
		ui->lab_filesize->setText(sFilesize);
	}
	ui->lab_filesize->setVisible(!sFilesize.isEmpty());
}


void GUI_Controls::skin_changed()
{
	using namespace Gui;

	QString stylesheet = Style::current_style();
	this->setStyleSheet(stylesheet);

	ui->btn_fw->setIcon(get_icon(Icons::Forward));
	ui->btn_bw->setIcon(get_icon(Icons::Backward));

	if(PlayManager::instance()->playstate() == PlayState::Playing){
		ui->btn_play->setIcon(get_icon(Icons::Pause));
	}

	else{
		ui->btn_play->setIcon(get_icon(Icons::Play));
	}

	ui->btn_stop->setIcon(get_icon(Icons::Stop));
	ui->btn_rec->setIcon(get_icon(Icons::Record));

	setup_volume_button(ui->sli_volume->value());
}

void GUI_Controls::language_changed()
{
	QString version = _settings->get<Set::Player_Version>();
	ui->lab_sayonara->setText(tr("Sayonara Player"));
	ui->lab_version->setText( version );
	ui->lab_writtenby->setText(tr("Written by") + " Lucio Carreras");
	ui->lab_copyright->setText(
		tr("Copyright") +
		QString(" 2011-%1").arg(QDate::currentDate().year())
	);
}

void GUI_Controls::resizeEvent(QResizeEvent* e)
{
	Widget::resizeEvent(e);
	refresh_info_labels();
}


void GUI_Controls::sr_active_changed()
{
	check_record_button_visible();
	ui->btn_rec->setChecked(false);
}


void GUI_Controls::check_record_button_visible()
{
	PlayManagerPtr play_manager = PlayManager::instance();

	const MetaData& md = play_manager->current_track();
	PlayState playstate = play_manager->playstate();

	bool is_lame_available = _settings->get<SetNoDB::MP3enc_found>();
	bool is_sr_active = _settings->get<Set::Engine_SR_Active>();
	bool is_radio = ((md.radio_mode() != RadioMode::Off));
	bool is_playing = (playstate == PlayState::Playing);

	bool recording_enabled = (is_lame_available &&
							  is_sr_active &&
							  is_radio &&
							  is_playing);

	ui->btn_play->setVisible(!recording_enabled);
	ui->btn_rec->setVisible(recording_enabled);

	if(!recording_enabled){
		ui->btn_rec->setChecked(false);
	}
}


void GUI_Controls::set_cover_location(const MetaData& md)
{
	Location cl = Location::cover_location(md);

	ui->btn_cover->set_cover_location(cl);
}

void GUI_Controls::set_standard_cover()
{
	ui->btn_cover->set_cover_location(Location::invalid_location());
}

void GUI_Controls::force_cover(const QImage& img)
{
	ui->btn_cover->force_cover(img);
}


void GUI_Controls::setup_connections()
{
	PlayManager* play_manager = PlayManager::instance();

	connect(ui->btn_play, &QPushButton::clicked, play_manager, &PlayManager::play_pause);
	connect(ui->btn_fw,	&QPushButton::clicked, play_manager, &PlayManager::next);
	connect(ui->btn_bw,	&QPushButton::clicked, play_manager, &PlayManager::previous);
	connect(ui->btn_stop, &QPushButton::clicked, play_manager, &PlayManager::stop);
	connect(ui->btn_mute, &QPushButton::released, play_manager, &PlayManager::toggle_mute);
	connect(ui->btn_rec, &QPushButton::clicked, play_manager, &PlayManager::record);

	connect(ui->sli_volume, &SearchSlider::sig_slider_moved, play_manager, &PlayManager::set_volume);
	connect(ui->sli_progress, &SearchSlider::sig_slider_moved, this, &GUI_Controls::progress_moved);
	connect(ui->sli_progress, &SearchSlider::sig_slider_hovered, this, &GUI_Controls::progress_hovered);

	connect(play_manager, &PlayManager::sig_playstate_changed, this, &GUI_Controls::playstate_changed);
	connect(play_manager, &PlayManager::sig_track_changed, this, &GUI_Controls::track_changed);
	connect(play_manager, &PlayManager::sig_position_changed_ms, this,	&GUI_Controls::cur_pos_changed);
	connect(play_manager, &PlayManager::sig_buffer, this, &GUI_Controls::buffering);
	connect(play_manager, &PlayManager::sig_volume_changed, this, &GUI_Controls::volume_changed);
	connect(play_manager, &PlayManager::sig_mute_changed, this, &GUI_Controls::mute_changed);
	connect(play_manager, &PlayManager::sig_record, this, &GUI_Controls::rec_changed);

	// engine
	Engine::Handler* engine = Engine::Handler::instance();
	connect(engine, &Engine::Handler::sig_md_changed,	this, &GUI_Controls::md_changed);
	connect(engine, &Engine::Handler::sig_dur_changed, this, &GUI_Controls::dur_changed);
	connect(engine, &Engine::Handler::sig_br_changed,	this, &GUI_Controls::br_changed);
	connect(engine, &Engine::Handler::sig_cover_changed, this, &GUI_Controls::force_cover);

	Tagging::ChangeNotifier* mdcn = Tagging::ChangeNotifier::instance();
	connect(mdcn, &Tagging::ChangeNotifier::sig_metadata_changed, this, &GUI_Controls::id3_tags_changed);
}

void GUI_Controls::setup_shortcuts()
{
	ShortcutHandler* sch = ShortcutHandler::instance();
	PlayManager* play_manager = PlayManager::instance();

	Shortcut sc1 = sch->add(Shortcut(this, "play_pause", Lang::get(Lang::PlayPause), "Space"));
	Shortcut sc2 = sch->add(Shortcut(this, "stop", Lang::get(Lang::Stop), "Ctrl + Space"));
	Shortcut sc3 = sch->add(Shortcut(this, "next", Lang::get(Lang::NextTrack), "Ctrl + Right"));
	Shortcut sc4 = sch->add(Shortcut(this, "prev", Lang::get(Lang::PreviousTrack), "Ctrl + Left"));
	Shortcut sc5 = sch->add(Shortcut(this, "vol_down", Lang::get(Lang::VolumeDown), "Ctrl + -"));
	Shortcut sc6 = sch->add(Shortcut(this, "vol_up", Lang::get(Lang::VolumeUp), "Ctrl++"));
	Shortcut sc7 = sch->add(Shortcut(this, "seek_fwd", Lang::get(Lang::SeekForward), "Alt+Right"));
	Shortcut sc8 = sch->add(Shortcut(this, "seek_bwd", Lang::get(Lang::SeekBackward), "Alt+Left"));
	Shortcut sc9 = sch->add(Shortcut(this, "seek_fwd_fast", Lang::get(Lang::SeekForward).space() + "(" + Lang::get(Lang::Fast) + ")", "Shift+Right"));
	Shortcut sc10 = sch->add(Shortcut(this, "seek_bwd_fast", Lang::get(Lang::SeekBackward).space() + "(" + Lang::get(Lang::Fast) + ")", "Shift+Left"));

	sc1.create_qt_shortcut(this, play_manager, SLOT(play_pause()));
	sc2.create_qt_shortcut(this, play_manager, SLOT(stop()));
	sc3.create_qt_shortcut(this, play_manager, SLOT(next()));
	sc4.create_qt_shortcut(this, play_manager, SLOT(previous()));
	sc5.create_qt_shortcut(this, play_manager, SLOT(volume_down()));
	sc6.create_qt_shortcut(this, play_manager, SLOT(volume_up()));
	sc7.create_qt_shortcut(this, [=]() {
		play_manager->seek_rel_ms(2000);
	});

	sc8.create_qt_shortcut(this, [=](){
		play_manager->seek_rel_ms(-2000);
	});

	sc9.create_qt_shortcut(this, [=]() {
		MilliSeconds ms = play_manager->duration_ms() / 20;
		play_manager->seek_rel_ms(ms);
	});

	sc10.create_qt_shortcut(this, [=]() {
		MilliSeconds ms = play_manager->duration_ms() / 20;
		play_manager->seek_rel_ms(-ms);
	});
}


QString GUI_Controls::get_shortcut_text(const QString &shortcut_identifier) const
{
	if(shortcut_identifier == "play_pause"){
		return Lang::get(Lang::PlayPause);
	}
	if(shortcut_identifier == "stop"){
		return Lang::get(Lang::Stop);
	}
	if(shortcut_identifier == "next"){
		return Lang::get(Lang::NextTrack);
	}
	if(shortcut_identifier == "prev"){
		return Lang::get(Lang::PreviousTrack);
	}
	if(shortcut_identifier == "vol_down"){
		return Lang::get(Lang::VolumeDown);
	}
	if(shortcut_identifier == "vol_up"){
		return Lang::get(Lang::VolumeUp);
	}
	if(shortcut_identifier == "seek_fwd"){
		return Lang::get(Lang::SeekForward);
	}
	if(shortcut_identifier == "seek_bwd"){
		return Lang::get(Lang::SeekBackward);
	}
	if(shortcut_identifier == "seek_fwd_fast"){
		return Lang::get(Lang::SeekForward).space() + "(" + Lang::get(Lang::Fast) + ")";
	}
	if(shortcut_identifier == "seek_bwd_fast"){
		return Lang::get(Lang::SeekBackward).space() + "(" + Lang::get(Lang::Fast) + ")";
	}

	return "";
}


void GUI_Controls::showEvent(QShowEvent* e)
{
	Widget::showEvent(e);
	refresh_info_labels();
}

void GUI_Controls::contextMenuEvent(QContextMenuEvent* e)
{
	if(!m->context_menu)
	{
		m->context_menu = new LibraryContextMenu(this);
		m->context_menu->show_actions( (LibraryContexMenuEntries)
			(LibraryContextMenu::EntryInfo |
			LibraryContextMenu::EntryLyrics |
			LibraryContextMenu::EntryEdit)
		);

		connect(m->context_menu, &LibraryContextMenu::sig_edit_clicked, this, [=](){
			show_edit();
		});

		connect(m->context_menu, &LibraryContextMenu::sig_info_clicked, this, [=](){
			show_info();
		});

		connect(m->context_menu, &LibraryContextMenu::sig_lyrics_clicked, this, [=](){
			show_lyrics();
		});

		m->context_menu->add_preference_action(new PlayerPreferencesAction(m->context_menu));
		m->context_menu->add_preference_action(new CoverPreferenceAction(m->context_menu));
	}

	m->context_menu->exec(e->globalPos());
}

void GUI_Controls::set_radio_mode(RadioMode radio)
{
	check_record_button_visible();

	if(radio != RadioMode::Off){
		buffering(0);
	}
}


MD::Interpretation GUI_Controls::metadata_interpretation() const
{
	return MD::Interpretation::Tracks;
}

MetaDataList GUI_Controls::info_dialog_data() const
{
	PlayState ps = PlayManager::instance()->playstate();
	if(ps == PlayState::Stopped){
		return MetaDataList();
	}

	return MetaDataList(
		PlayManager::instance()->current_track()
	);
}
