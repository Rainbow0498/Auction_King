#include "ui/ColorInputCard.h"

#include <QDoubleValidator>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QVBoxLayout>

namespace bbae {

ColorInputCard::ColorInputCard(const ColorGroupConfig& config, QWidget* parent)
    : QFrame(parent)
    , config_(config)
{
    setObjectName("colorInputCard");

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(14, 12, 14, 14);
    root->setSpacing(12);

    auto* header = new QHBoxLayout();
    auto* swatch = new QLabel(this);
    swatch->setFixedSize(12, 12);
    swatch->setStyleSheet(QString("background:%1; border-radius:6px;").arg(QString::fromStdString(config_.colorHex)));
    auto* title = new QLabel(QString::fromStdString(config_.label), this);
    title->setObjectName("cardTitle");
    header->addWidget(swatch);
    header->addWidget(title);
    header->addStretch(1);
    root->addLayout(header);

    auto* grid = new QGridLayout();
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(8);

    int column = 0;
    int row = 0;
    for (const auto& field : config_.fields) {
        auto* label = new QLabel(fieldLabel(field), this);
        auto* edit = addField(field, this);
        grid->addWidget(label, row, column);
        grid->addWidget(edit, row + 1, column);
        ++column;
        if (column == 3) {
            column = 0;
            row += 2;
        }
    }

    root->addLayout(grid);
}

ColorInputValues ColorInputCard::values() const
{
    ColorInputValues values;
    values.key = config_.key;
    values.label = config_.label;

    const auto lookup = [this](const std::string& field) -> QLineEdit* {
        const auto it = edits_.find(field);
        return it == edits_.end() ? nullptr : it->second;
    };

    values.unitGridPrice = optionalDouble(lookup("unit_grid_price"));
    values.totalCount = optionalInt(lookup("total_count"));
    values.totalGrid = optionalDouble(lookup("total_grid"));
    values.avgPrice = optionalDouble(lookup("avg_price"));
    values.totalPrice = optionalDouble(lookup("total_price"));
    return values;
}

void ColorInputCard::clear()
{
    for (const auto& entry : edits_) {
        entry.second->clear();
    }
}

QLineEdit* ColorInputCard::addField(const std::string& field, QWidget* parent)
{
    auto* edit = new QLineEdit(parent);
    edit->setPlaceholderText(placeholderFor(field));
    edit->setMinimumHeight(34);
    if (field == "total_count") {
        edit->setValidator(new QIntValidator(0, 999, edit));
    } else {
        edit->setValidator(new QDoubleValidator(0.0, 999999999.0, 3, edit));
    }
    connect(edit, &QLineEdit::textChanged, this, &ColorInputCard::valuesChanged);
    edits_[field] = edit;
    return edit;
}

QString ColorInputCard::fieldLabel(const std::string& field)
{
    if (field == "unit_grid_price") {
        return QString::fromUtf8("均格价值");
    }
    if (field == "total_count") {
        return QString::fromUtf8("件数");
    }
    if (field == "total_grid") {
        return QString::fromUtf8("总格数");
    }
    if (field == "avg_price") {
        return QString::fromUtf8("平均价格");
    }
    if (field == "total_price") {
        return QString::fromUtf8("总价");
    }
    return QString::fromStdString(field);
}

QString ColorInputCard::placeholderFor(const std::string& field)
{
    if (field == "unit_grid_price") {
        return QString::fromUtf8("例 2230");
    }
    if (field == "total_count") {
        return QString::fromUtf8("例 4");
    }
    if (field == "total_grid") {
        return QString::fromUtf8("例 14");
    }
    if (field == "avg_price") {
        return QString::fromUtf8("例 58000");
    }
    if (field == "total_price") {
        return QString::fromUtf8("例 232000");
    }
    return {};
}

std::optional<double> ColorInputCard::optionalDouble(const QLineEdit* edit)
{
    if (!edit || edit->text().trimmed().isEmpty()) {
        return std::nullopt;
    }
    bool ok = false;
    const double value = edit->text().trimmed().toDouble(&ok);
    return ok ? std::optional<double>(value) : std::nullopt;
}

std::optional<int> ColorInputCard::optionalInt(const QLineEdit* edit)
{
    if (!edit || edit->text().trimmed().isEmpty()) {
        return std::nullopt;
    }
    bool ok = false;
    const int value = edit->text().trimmed().toInt(&ok);
    return ok ? std::optional<int>(value) : std::nullopt;
}

} // namespace bbae
