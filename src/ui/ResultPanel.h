#pragma once

#include "core/AuctionAdvice.h"
#include "core/EstimateResult.h"

#include <QLabel>
#include <QTableWidget>
#include <QWidget>

namespace bbae {

class ResultPanel : public QWidget {
    Q_OBJECT

public:
    explicit ResultPanel(QWidget* parent = nullptr);

    void showEmptyMessage(const QString& message);
    void showResult(const EstimateResult& estimate, const AuctionAdvice& advice);

private:
    QLabel* expectedValueLabel_ = nullptr;
    QLabel* medianValueLabel_ = nullptr;
    QLabel* rangeLabel_ = nullptr;
    QLabel* confidenceLabel_ = nullptr;
    QLabel* candidateCountLabel_ = nullptr;
    QLabel* conservativeBidLabel_ = nullptr;
    QLabel* balancedBidLabel_ = nullptr;
    QLabel* aggressiveBidLabel_ = nullptr;
    QLabel* maxSecondPriceLabel_ = nullptr;
    QTableWidget* breakdownTable_ = nullptr;

    static QString moneyWan(double value);
    static QString countText(unsigned long long value);
};

} // namespace bbae

