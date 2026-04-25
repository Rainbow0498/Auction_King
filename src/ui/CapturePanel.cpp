#include "ui/CapturePanel.h"

#include <QLabel>
#include <QVBoxLayout>

namespace bbae {

CapturePanel::CapturePanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    auto* label = new QLabel(QString::fromUtf8("图像识别将在第二阶段接入。"), this);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
}

} // namespace bbae

