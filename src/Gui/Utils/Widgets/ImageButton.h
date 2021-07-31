#ifndef IMAGEBUTTON_H
#define IMAGEBUTTON_H

#include <QPushButton>
#include "Utils/Pimpl.h"

class QPixmap;

namespace Gui
{
	class ByteArrayConverter :
		public QObject
	{
		Q_OBJECT
		PIMPL(ByteArrayConverter)

		signals:
			void sigFinished();

		public:
			ByteArrayConverter(const QByteArray& data, const QString& mime);
			~ByteArrayConverter();

			QPixmap pixmap() const;

		public slots:
			void start();
	};


	class ImageButton : public QPushButton
	{
		Q_OBJECT
		PIMPL(ImageButton)

		signals:
			void sigPixmapChanged();
			void sigTriggered();

		public:
			explicit ImageButton(QWidget* parent=nullptr);
			~ImageButton() override;

			QPixmap pixmap() const;
			int verticalPadding() const;
			void setFadingEnabled(bool b);

		public slots:
			void showDefaultPixmap();
			void setPixmap(const QPixmap& pm);
			void setPixmapPath(const QString& path);
			void setCoverData(const QByteArray& data, const QString& mimetype);

		private slots:
			void timerTimedOut();
			void byteconverterFinished();

		private:
			using QPushButton::setIcon;
			using QPushButton::icon;

		protected:
			void paintEvent(QPaintEvent* e) override;
			void resizeEvent(QResizeEvent* e) override;
			void mouseMoveEvent(QMouseEvent* e) override;
			void mouseReleaseEvent(QMouseEvent* event) override;
	};
}

#endif // IMAGEBUTTON_H
