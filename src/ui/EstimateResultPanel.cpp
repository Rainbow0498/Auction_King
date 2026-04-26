#include "ui/EstimateResultPanel.h"

#include <QAbstractItemView>
#include <QGridLayout>
#include <QHeaderView>
#include <QStringList>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace bbae {

EstimateResultPanel::EstimateResultPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16, 14, 16, 16);
    root->setSpacing(12);

    auto* title = new QLabel(QString::fromUtf8("估价结果区"), this);
    title->setObjectName("sectionTitle");
    root->addWidget(title);

    auto* grid = new QGridLayout();
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(10);

    lowestLabel_ = new QLabel("-", this);
    guaranteedLabel_ = new QLabel("-", this);
    referenceLabel_ = new QLabel("-", this);
    aggressiveLabel_ = new QLabel("-", this);
    highestLabel_ = new QLabel("-", this);

    grid->addWidget(new QLabel(QString::fromUtf8("最低价格"), this), 0, 0);
    grid->addWidget(lowestLabel_, 0, 1);
    grid->addWidget(new QLabel(QString::fromUtf8("保底价格"), this), 0, 2);
    grid->addWidget(guaranteedLabel_, 0, 3);
    grid->addWidget(new QLabel(QString::fromUtf8("参考价格"), this), 1, 0);
    grid->addWidget(referenceLabel_, 1, 1);
    grid->addWidget(new QLabel(QString::fromUtf8("激进价格"), this), 1, 2);
    grid->addWidget(aggressiveLabel_, 1, 3);
    grid->addWidget(new QLabel(QString::fromUtf8("最高价格"), this), 2, 0);
    grid->addWidget(highestLabel_, 2, 1);
    root->addLayout(grid);

    riskLabel_ = new QLabel(QString::fromUtf8("风险提示：等待估价。"), this);
    riskLabel_->setWordWrap(true);
    riskLabel_->setObjectName("riskLabel");
    root->addWidget(riskLabel_);

    table_ = new QTableWidget(this);
    table_->setColumnCount(8);
    table_->setHorizontalHeaderLabels({
        QString::fromUtf8("紫色"),
        QString::fromUtf8("橙色"),
        QString::fromUtf8("红色"),
        QString::fromUtf8("最低"),
        QString::fromUtf8("保底"),
        QString::fromUtf8("参考"),
        QString::fromUtf8("激进"),
        QString::fromUtf8("最高")
    });
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table_->verticalHeader()->setVisible(false);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionMode(QAbstractItemView::NoSelection);
    table_->setAlternatingRowColors(true);
    root->addWidget(table_, 1);
}

void EstimateResultPanel::showResult(const PriceEstimate& estimate)
{
    if (!estimate.valid) {
        clear();
        riskLabel_->setText(riskText(estimate.risks));
        return;
    }

    lowestLabel_->setText(moneyText(estimate.lowestPrice));
    guaranteedLabel_->setText(moneyText(estimate.guaranteedPrice));
    referenceLabel_->setText(moneyText(estimate.referencePrice));
    aggressiveLabel_->setText(moneyText(estimate.aggressivePrice));
    highestLabel_->setText(moneyText(estimate.highestPrice));
    riskLabel_->setText(riskText(estimate.risks));

    table_->setRowCount(static_cast<int>(estimate.rows.size()));
    for (int row = 0; row < static_cast<int>(estimate.rows.size()); ++row) {
        const auto& item = estimate.rows[static_cast<std::size_t>(row)];
        table_->setItem(row, 0, new QTableWidgetItem(QString::number(item.counts.count("purple") ? item.counts.at("purple") : 0)));
        table_->setItem(row, 1, new QTableWidgetItem(QString::number(item.counts.count("gold") ? item.counts.at("gold") : 0)));
        table_->setItem(row, 2, new QTableWidgetItem(QString::number(item.counts.count("red") ? item.counts.at("red") : 0)));
        table_->setItem(row, 3, new QTableWidgetItem(moneyText(item.lowestPrice)));
        table_->setItem(row, 4, new QTableWidgetItem(moneyText(item.guaranteedPrice)));
        table_->setItem(row, 5, new QTableWidgetItem(moneyText(item.referencePrice)));
        table_->setItem(row, 6, new QTableWidgetItem(moneyText(item.aggressivePrice)));
        table_->setItem(row, 7, new QTableWidgetItem(moneyText(item.highestPrice)));
    }
}

void EstimateResultPanel::clear()
{
    lowestLabel_->setText("-");
    guaranteedLabel_->setText("-");
    referenceLabel_->setText("-");
    aggressiveLabel_->setText("-");
    highestLabel_->setText("-");
    riskLabel_->setText(QString::fromUtf8("风险提示：等待估价。"));
    table_->setRowCount(0);
}

QString EstimateResultPanel::moneyText(double value)
{
    return QString("%1w").arg(value / 10000.0, 0, 'f', 2);
}

QString EstimateResultPanel::riskText(const std::vector<std::string>& risks)
{
    if (risks.empty()) {
        return QString::fromUtf8("风险提示：线索较完整，当前估价可作为参考。");
    }

    QStringList lines;
    for (const auto& risk : risks) {
        lines << QString::fromUtf8("风险提示：%1").arg(QString::fromStdString(risk));
    }
    return lines.join("\n");
}

} // namespace bbae
