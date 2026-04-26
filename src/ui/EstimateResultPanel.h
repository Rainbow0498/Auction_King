#pragma once

#include "core/PriceCalculator.h"

#include <QTableWidget>
#include <QLabel>
#include <QWidget>

#include <vector>

namespace bbae {

class EstimateResultPanel : public QWidget {
    Q_OBJECT

public:
    explicit EstimateResultPanel(QWidget* parent = nullptr);

    void showResult(const PriceEstimate& estimate);
    void clear();

private:
    QLabel* lowestLabel_ = nullptr;
    QLabel* guaranteedLabel_ = nullptr;
    QLabel* referenceLabel_ = nullptr;
    QLabel* aggressiveLabel_ = nullptr;
    QLabel* highestLabel_ = nullptr;
    QLabel* riskLabel_ = nullptr;
    QTableWidget* table_ = nullptr;

    static QString moneyText(double value);
    static QString riskText(const std::vector<std::string>& risks);
};

} // namespace bbae
