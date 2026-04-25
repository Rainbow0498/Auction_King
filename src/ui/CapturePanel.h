#pragma once

#include <QWidget>

namespace bbae {

class CapturePanel : public QWidget {
    Q_OBJECT

public:
    explicit CapturePanel(QWidget* parent = nullptr);
};

} // namespace bbae

