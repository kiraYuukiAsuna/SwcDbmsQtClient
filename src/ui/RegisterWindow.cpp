#include "RegisterWindow.h"

#include <Message/Request.pb.h>
#include <grpcpp/client_context.h>

#include <QMessageBox>

#include "src/framework/defination/ImageDefination.h"
#include "src/framework/service/RpcCall.h"
#include "src/framework/service/WrappedCall.h"
#include "ui_RegisterWindow.h"

RegisterWindow::RegisterWindow(QWidget* parent)
	: QDialog(parent), ui(new Ui::RegisterWindow) {
	ui->setupUi(this);
	this->setWindowIcon(QIcon(Image::ImageCreateUser));
	this->setWindowFlags(this->windowFlags() & ~Qt::WindowMaximizeButtonHint);
	this->setFixedSize(this->size());

	connect(ui->registerBtn, &QPushButton::clicked, this,
			&RegisterWindow::onRegisterBtnClicked);

	ui->label->setPixmap(
		QPixmap::fromImage(QImage(Image::ImageCreateUser).scaled({32, 32})));
	ui->label_2->setPixmap(
		QPixmap::fromImage(QImage(Image::ImagePassword).scaled({32, 32})));
	ui->label_3->setPixmap(
		QPixmap::fromImage(QImage(Image::ImagePassword).scaled({32, 32})));
	ui->label_4->setPixmap(
		QPixmap::fromImage(QImage(Image::ImageDescription).scaled({32, 32})));
}

RegisterWindow::~RegisterWindow() { delete ui; }

void RegisterWindow::onRegisterBtnClicked(bool checked) {
	if (ui->userNameEditor->text().trimmed().isEmpty()) {
		QMessageBox::warning(this, "Error", "User Name cannot be empty!");
		return;
	}
	if (ui->passwordEditor->text().trimmed().isEmpty() ||
		ui->repeatPasswordEditor->text().trimmed().isEmpty()) {
		QMessageBox::warning(this, "Error", "Password cannot be empty!");
		return;
	}
	if (ui->passwordEditor->text().trimmed() !=
		ui->repeatPasswordEditor->text().trimmed()) {
		QMessageBox::warning(this, "Error",
							 "Password and Repeated Password are not equal!");
		return;
	}

	proto::CreateUserResponse response;
	if (WrappedCall::CreateUser(
			ui->userNameEditor->text().trimmed().toStdString(),
			ui->passwordEditor->text().trimmed().toStdString(),
			ui->descriptionEditor->text().trimmed().toStdString(), response,
			this)) {
		if (response.metainfo().status()) {
			QMessageBox::information(this, "Info", "Register Successfully!");
			accept();
		} else {
			QMessageBox::warning(
				this, "Info",
				"Register Failed!" +
					QString::fromStdString(response.metainfo().message()));
		}
	}
}
