#include "GUI_ControlsNew.h"
#include "GUI/Player/ui_GUI_ControlsNew.h"

GUI_ControlsNew::GUI_ControlsNew(QWidget* parent) :
	GUI_ControlsBase(parent)
{
	ui = new Ui::GUI_ControlsNew();
	ui->setupUi(this);
}

GUI_ControlsNew::~GUI_ControlsNew()
{
	delete ui; ui=nullptr;
}

QLabel* GUI_ControlsNew::lab_sayonara() const {	return ui->lab_sayonara; }
QLabel* GUI_ControlsNew::lab_title() const { return ui->lab_title; }
QLabel* GUI_ControlsNew::lab_version() const { return ui->lab_version; }
QLabel* GUI_ControlsNew::lab_album() const { return ui->lab_album; }
QLabel* GUI_ControlsNew::lab_artist() const { return ui->lab_artist; }
QLabel* GUI_ControlsNew::lab_writtenby() const { return ui->lab_writtenby; }
QLabel* GUI_ControlsNew::lab_bitrate() const { return ui->lab_bitrate; }
QLabel* GUI_ControlsNew::lab_copyright() const { return ui->lab_copyright; }
QLabel* GUI_ControlsNew::lab_filesize() const { return ui->lab_filesize; }
QLabel* GUI_ControlsNew::lab_current_time() const { return ui->lab_cur_time; }
QLabel* GUI_ControlsNew::lab_max_time() const { return ui->lab_max_time; }
QWidget* GUI_ControlsNew::widget_details() const { return ui->widget_details; }
SearchSlider* GUI_ControlsNew::sli_progress() const { return ui->sli_progress; }
SearchSlider* GUI_ControlsNew::sli_volume() const { return ui->sli_volume; }
Gui::ProgressBar* GUI_ControlsNew::sli_buffer() const { return ui->sli_buffer; }
QPushButton* GUI_ControlsNew::btn_mute() const { return ui->btn_mute; }
QPushButton* GUI_ControlsNew::btn_play() const { return ui->btn_play; }
QPushButton* GUI_ControlsNew::btn_rec() const { return ui->btn_rec; }
QPushButton* GUI_ControlsNew::btn_bwd() const { return ui->btn_bw; }
QPushButton* GUI_ControlsNew::btn_fwd() const { return ui->btn_fw; }
QPushButton* GUI_ControlsNew::btn_stop() const { return ui->btn_stop; }
CoverButton* GUI_ControlsNew::btn_cover() const { return ui->btn_cover; }


void GUI_ControlsNew::toggle_buffer_mode(bool buffering)
{
	if(!buffering){
		ui->progress_widget->setCurrentIndex(0);
	}

	else {
		ui->progress_widget->setCurrentIndex(1);
	}
}

bool GUI_ControlsNew::is_resizable() const
{
	return true;
}
