#include "ui/CombinationPanel.h"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QStringList>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace bbae {

CombinationPanel::CombinationPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16, 14, 16, 16);
    root->setSpacing(10);

    auto* title = new QLabel(QString::fromUtf8("件数组合信息区"), this);
    title->setObjectName("sectionTitle");
    summaryLabel_ = new QLabel(QString::fromUtf8("等待生成组合。"), this);
    summaryLabel_->setWordWrap(true);

    countTable_ = new QTableWidget(this);
    countTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    countTable_->verticalHeader()->setVisible(false);
    countTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    countTable_->setSelectionMode(QAbstractItemView::NoSelection);
    countTable_->setAlternatingRowColors(true);

    root->addWidget(title);
    root->addWidget(summaryLabel_);
    root->addWidget(countTable_, 1);
}

void CombinationPanel::showResult(const CombinationResult& result)
{
    QStringList parts;
    if (result.totalCollectionCount) {
        parts << QString::fromUtf8("总藏品 = %1").arg(*result.totalCollectionCount);
    }
    parts << QString::fromUtf8("合法组合数量 = %1").arg(result.legalCombinationCount);
    for (const auto& options : result.countOptions) {
        QStringList counts;
        for (int count : options.counts) {
            counts << QString::number(count);
        }
        parts << QString::fromUtf8("%1可取 %2").arg(QString::fromStdString(options.label), counts.join("/"));
    }
    summaryLabel_->setText(parts.join(QString::fromUtf8("   |   ")));

    QStringList headers;
    for (const auto& options : result.countOptions) {
        headers << QString::fromStdString(options.label);
    }
    if (result.totalCollectionCount) {
        headers << QString::fromUtf8("总件数");
    }

    countTable_->clearContents();
    countTable_->setColumnCount(headers.size());
    countTable_->setHorizontalHeaderLabels(headers);
    countTable_->setRowCount(static_cast<int>(result.combinations.size()));

    for (int row = 0; row < static_cast<int>(result.combinations.size()); ++row) {
        const auto& combination = result.combinations[static_cast<std::size_t>(row)];
        int total = 0;
        for (int column = 0; column < static_cast<int>(result.countOptions.size()); ++column) {
            const auto& options = result.countOptions[static_cast<std::size_t>(column)];
            const auto it = combination.counts.find(options.key);
            const int value = it == combination.counts.end() ? 0 : it->second;
            total += value;
            countTable_->setItem(row, column, new QTableWidgetItem(QString::number(value)));
        }
        if (result.totalCollectionCount) {
            countTable_->setItem(row, headers.size() - 1, new QTableWidgetItem(QString::number(total)));
        }
    }
}

void CombinationPanel::clear()
{
    summaryLabel_->setText(QString::fromUtf8("等待生成组合。"));
    countTable_->setRowCount(0);
}

} // namespace bbae
