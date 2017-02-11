#include "BigDiffWindow.h"
#include "ui_BigDiffWindow.h"

BigDiffWindow::BigDiffWindow(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::BigDiffWindow)
{
	ui->setupUi(this);
	Qt::WindowFlags flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint;
	flags |= Qt::WindowMaximizeButtonHint;
	setWindowFlags(flags);

	ui->widget_diff->setMaximizeButtonEnabled(false);
}

BigDiffWindow::~BigDiffWindow()
{
	delete ui;
}

void BigDiffWindow::init(MainWindow *mw, FileDiffWidget::InitParam_ const &param)
{
	ui->widget_diff->bind(mw);

	switch (param.view_style) {
	case FileDiffWidget::ViewStyle::LeftOnly:
		ui->widget_diff->setLeftOnly(param.content_left, param.diff);
		break;
	case FileDiffWidget::ViewStyle::RightOnly:
		ui->widget_diff->setRightOnly(param.content_left, param.diff);
		break;
	case FileDiffWidget::ViewStyle::SideBySide:
		ui->widget_diff->setSideBySide(param.content_left, param.diff, param.uncommited, param.workingdir);
		break;
	}
}