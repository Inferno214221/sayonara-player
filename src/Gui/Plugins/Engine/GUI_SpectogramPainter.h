#ifndef GUI_SPECTOGRAMPAINTER_H
#define GUI_SPECTOGRAMPAINTER_H

#include "Components/PlayManager/PlayState.h"
#include "Gui/Plugins/PlayerPluginBase.h"

#include <QList>
#include <QWidget>

class GUI_SpectogramPainter :
	public PlayerPlugin::Base
{
	Q_OBJECT
	PIMPL(GUI_SpectogramPainter)

public:
	explicit GUI_SpectogramPainter(QWidget* parent = nullptr);
	~GUI_SpectogramPainter() override;

	QString get_name() const override;
	QString get_display_name() const override;
	bool is_ui_initialized() const override;

private slots:
	void reset();
	void spectrum_changed(const QList<float>& spectrum, MilliSeconds ms);
	void finished();

	void playstate_changed(PlayState state);
	void track_changed(const MetaData& md);

protected:
	void retranslate_ui() override;
	void init_ui() override;

	void paintEvent(QPaintEvent* e) override;
	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;

private:
	void draw_buffer(int percent_step);
	QString calc_tooltip(float yPercent);

	void show_fullsize();
	void position_clicked(QPoint position);

	QSize minimumSizeHint() const override;

};

#endif // GUI_SPECTOGRAMPAINTER_H
