#include "ProgressBar.h"

#include <QVBoxLayout>

ProgressBar::ProgressBar(QWidget* parent) : QDialog(parent) {
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());

    auto layout = new QVBoxLayout(this);

    m_ProgressBar = new QProgressBar(this);
    m_TitleLabel = new QLabel(this);
    m_ProgressLabel = new QLabel(this);
    m_ProgressLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(m_TitleLabel);
    layout->addWidget(m_ProgressBar);
    layout->addWidget(m_ProgressLabel);

    setLayout(layout);

    m_TitleLabel->setText("Processing...Please Waiting...");
    m_ProgressBar->setValue(0);
    m_ProgressBar->setRange(0,100);
    m_ProgressLabel->setText("0%");
}

ProgressBar::~ProgressBar() {

}

void ProgressBar::setText(const std::string& text) {
    m_TitleLabel->setText(QString::fromStdString(text));
}

void ProgressBar::setValue(int value) {
    m_ProgressBar->setValue(value);
    m_ProgressLabel->setText(QString::number(value) + "%");
}

void ProgressBar::finish() {
    accept();
}
