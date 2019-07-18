#ifndef LINEINPUTDIALOG_H
#define LINEINPUTDIALOG_H

#include "Gui/Utils/Widgets/Dialog.h"
#include "Utils/Pimpl.h"

UI_FWD(LineInputDialog)

namespace Gui
{
	class Completer;
	class LineInputDialog :
		public Dialog
	{
		Q_OBJECT
		UI_CLASS(LineInputDialog)
		PIMPL(LineInputDialog)

		public:
			enum ReturnValue
			{
				Ok=0,
				Cancelled
			};

			LineInputDialog(const QString& window_title, const QString& info_text, const QString& input_text, QWidget* parent=nullptr);
			LineInputDialog(const QString& window_title, const QString& info_text, QWidget* parent=nullptr);
			~LineInputDialog();

			void set_header_text(const QString& text);
			void set_info_text(const QString& text);
			void set_completer_text(const QStringList& lst);

			ReturnValue return_value() const;
			QString text() const;
			void set_text(const QString& text);

			bool was_accepted() const;

		private slots:
			void ok_clicked();
			void cancel_clicked();

		protected:
			void showEvent(QShowEvent* e) override;
			void closeEvent(QCloseEvent* e) override;
	};
}

#endif // LINEINPUTDIALOG_H
