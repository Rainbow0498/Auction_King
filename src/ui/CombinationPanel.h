#pragma once

#include "core/CombinationSolver.h"

#include <QLabel>
#include <QTableWidget>
#include <QWidget>

namespace bbae {

class CombinationPanel : public QWidget {
    Q_OBJECT

public:
    explicit CombinationPanel(QWidget* parent = nullptr);

    void showResult(const CombinationResult& result);
    void clear();

private:
    QLabel* summaryLabel_ = nullptr;
    QTableWidget* countTable_ = nullptr;
};

} // namespace bbae

