#include "ui/ResultPanel.h"

#include <QAbstractItemView>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLocale>
#include <QVBoxLayout>

namespace bbae {

ResultPanel::ResultPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(14);

    auto* summaryBox = new QGroupBox(QString::fromUtf8("推理结果与估值"), this);
    auto* summaryLayout = new QGridLayout(summaryBox);
    summaryLayout->setSpacing(10);

    expectedValueLabel_ = new QLabel("-", this);
    medianValueLabel_ = new QLabel("-", this);
    rangeLabel_ = new QLabel("-", this);
    confidenceLabel_ = new QLabel("-", this);
    candidateCountLabel_ = new QLabel("-", this);

    summaryLayout->addWidget(new QLabel(QString::fromUtf8("期望估值"), this), 0, 0);
    summaryLayout->addWidget(expectedValueLabel_, 0, 1);
    summaryLayout->addWidget(new QLabel(QString::fromUtf8("中位估值"), this), 0, 2);
    summaryLayout->addWidget(medianValueLabel_, 0, 3);
    summaryLayout->addWidget(new QLabel(QString::fromUtf8("区间"), this), 1, 0);
    summaryLayout->addWidget(rangeLabel_, 1, 1);
    summaryLayout->addWidget(new QLabel(QString::fromUtf8("置信度"), this), 1, 2);
    summaryLayout->addWidget(confidenceLabel_, 1, 3);
    summaryLayout->addWidget(new QLabel(QString::fromUtf8("候选组合"), this), 2, 0);
    summaryLayout->addWidget(candidateCountLabel_, 2, 1);

    auto* adviceBox = new QGroupBox(QString::fromUtf8("出价建议"), this);
    auto* adviceLayout = new QGridLayout(adviceBox);
    adviceLayout->setSpacing(10);

    conservativeBidLabel_ = new QLabel("-", this);
    balancedBidLabel_ = new QLabel("-", this);
    aggressiveBidLabel_ = new QLabel("-", this);
    maxSecondPriceLabel_ = new QLabel("-", this);

    adviceLayout->addWidget(new QLabel(QString::fromUtf8("保守出价"), this), 0, 0);
    adviceLayout->addWidget(conservativeBidLabel_, 0, 1);
    adviceLayout->addWidget(new QLabel(QString::fromUtf8("均衡出价"), this), 0, 2);
    adviceLayout->addWidget(balancedBidLabel_, 0, 3);
    adviceLayout->addWidget(new QLabel(QString::fromUtf8("激进出价"), this), 1, 0);
    adviceLayout->addWidget(aggressiveBidLabel_, 1, 1);
    adviceLayout->addWidget(new QLabel(QString::fromUtf8("可接受二价"), this), 1, 2);
    adviceLayout->addWidget(maxSecondPriceLabel_, 1, 3);

    breakdownTable_ = new QTableWidget(this);
    breakdownTable_->setColumnCount(7);
    breakdownTable_->setHorizontalHeaderLabels({
        QString::fromUtf8("金色"),
        QString::fromUtf8("紫色"),
        QString::fromUtf8("红色"),
        QString::fromUtf8("最低"),
        QString::fromUtf8("中位"),
        QString::fromUtf8("最高"),
        QString::fromUtf8("权重")
    });
    breakdownTable_->horizontalHeader()->setStretchLastSection(true);
    breakdownTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    breakdownTable_->verticalHeader()->setVisible(false);
    breakdownTable_->setAlternatingRowColors(true);
    breakdownTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    breakdownTable_->setSelectionBehavior(QAbstractItemView::SelectRows);

    root->addWidget(summaryBox);
    root->addWidget(adviceBox);
    root->addWidget(breakdownTable_, 1);
}

void ResultPanel::showEmptyMessage(const QString& message)
{
    expectedValueLabel_->setText(message);
    medianValueLabel_->setText("-");
    rangeLabel_->setText("-");
    confidenceLabel_->setText("-");
    candidateCountLabel_->setText("-");
    conservativeBidLabel_->setText("-");
    balancedBidLabel_->setText("-");
    aggressiveBidLabel_->setText("-");
    maxSecondPriceLabel_->setText("-");
    breakdownTable_->setRowCount(0);
}

void ResultPanel::showResult(const EstimateResult& estimate, const AuctionAdvice& advice)
{
    if (!estimate.valid) {
        showEmptyMessage(QString::fromStdString(estimate.message));
        return;
    }

    expectedValueLabel_->setText(moneyWan(estimate.expectedValue));
    medianValueLabel_->setText(moneyWan(estimate.medianValue));
    rangeLabel_->setText(QString("%1 - %2 / P10 %3 / P90 %4")
                             .arg(moneyWan(estimate.minValue), moneyWan(estimate.maxValue),
                                  moneyWan(estimate.p10Value), moneyWan(estimate.p90Value)));
    confidenceLabel_->setText(QString("%1%").arg(estimate.confidence * 100.0, 0, 'f', 1));
    candidateCountLabel_->setText(countText(estimate.candidateCount));

    conservativeBidLabel_->setText(moneyWan(advice.conservativeBid));
    balancedBidLabel_->setText(moneyWan(advice.balancedBid));
    aggressiveBidLabel_->setText(moneyWan(advice.aggressiveBid));
    maxSecondPriceLabel_->setText(moneyWan(advice.maxAcceptableSecondPrice));

    breakdownTable_->setRowCount(static_cast<int>(estimate.breakdown.size()));
    for (int row = 0; row < static_cast<int>(estimate.breakdown.size()); ++row) {
        const auto& item = estimate.breakdown[static_cast<std::size_t>(row)];
        breakdownTable_->setItem(row, 0, new QTableWidgetItem(QString::number(item.goldCount)));
        breakdownTable_->setItem(row, 1, new QTableWidgetItem(QString::number(item.purpleCount)));
        breakdownTable_->setItem(row, 2, new QTableWidgetItem(QString::number(item.redCount)));
        breakdownTable_->setItem(row, 3, new QTableWidgetItem(moneyWan(item.minValue)));
        breakdownTable_->setItem(row, 4, new QTableWidgetItem(moneyWan(item.medianValue)));
        breakdownTable_->setItem(row, 5, new QTableWidgetItem(moneyWan(item.maxValue)));
        breakdownTable_->setItem(row, 6, new QTableWidgetItem(QString::number(item.weight, 'g', 3)));
    }
}

QString ResultPanel::moneyWan(double value)
{
    return QString("%1w").arg(value / 10000.0, 0, 'f', 2);
}

QString ResultPanel::countText(unsigned long long value)
{
    return QLocale().toString(static_cast<qulonglong>(value));
}

} // namespace bbae
