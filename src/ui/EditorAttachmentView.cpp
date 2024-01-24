#include "EditorAttachmentView.h"

#include "EditorAnoAttachment.h"
#include "EditorApoAttachment.h"
#include "ui_EditorAttachmentView.h"


EditorAttachmentView::EditorAttachmentView(const std::string& swcName, QWidget *parent) :
    QDialog(parent), ui(new Ui::EditorAttachmentView) , m_SwcName(swcName){
    ui->setupUi(this);

    connect(ui->AnoBtn,&QPushButton::clicked,this,[&]() {
        EditorAnoAttachment view(m_SwcName,this);
        view.exec();
    });

    connect(ui->ApoBtn,&QPushButton::clicked,this,[&]() {
        EditorApoAttachment view(m_SwcName,this);
        view.exec();
    });
}

EditorAttachmentView::~EditorAttachmentView() {
    delete ui;
}
