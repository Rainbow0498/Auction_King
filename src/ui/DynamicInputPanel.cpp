#include "ui/DynamicInputPanel.h"

#include <QFrame>
#include <QFormLayout>
#include <QIntValidator>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

namespace bbae {

DynamicInputPanel::DynamicInputPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(12);

    auto* title = new QLabel(QString::fromUtf8("角色动态输入区"), this);
    title->setObjectName("sectionTitle");
    root->addWidget(title);

    globalContainer_ = new QWidget(this);
    auto* globalLayout = new QVBoxLayout(globalContainer_);
    globalLayout->setContentsMargins(0, 0, 0, 0);
    globalLayout->setSpacing(12);
    root->addWidget(globalContainer_);

    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    cardContainer_ = new QWidget(scroll);
    auto* cardLayout = new QVBoxLayout(cardContainer_);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(12);
    cardLayout->addStretch(1);
    scroll->setWidget(cardContainer_);
    root->addWidget(scroll, 1);

    auto* button = new QPushButton(QString::fromUtf8("生成组合与估价"), this);
    button->setObjectName("primaryButton");
    button->setMinimumHeight(42);
    connect(button, &QPushButton::clicked, this, &DynamicInputPanel::calculateRequested);
    root->addWidget(button);
}

void DynamicInputPanel::setRole(const RoleConfig& role)
{
    auto* globalLayout = static_cast<QVBoxLayout*>(globalContainer_->layout());
    QLayoutItem* item = nullptr;
    while ((item = globalLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    globalEdits_.clear();

    if (!role.globalFields.empty()) {
        auto* frame = new QFrame(globalContainer_);
        frame->setObjectName("colorInputCard");
        auto* frameLayout = new QVBoxLayout(frame);
        frameLayout->setContentsMargins(14, 12, 14, 14);
        frameLayout->setSpacing(10);

        auto* header = new QLabel(QString::fromUtf8("全局参数"), frame);
        header->setObjectName("cardTitle");
        frameLayout->addWidget(header);

        auto* form = new QFormLayout();
        form->setContentsMargins(0, 0, 0, 0);
        form->setHorizontalSpacing(12);
        form->setVerticalSpacing(8);

        for (const auto& field : role.globalFields) {
            auto* edit = new QLineEdit(frame);
            edit->setPlaceholderText(QString::fromStdString(field.placeholder));
            edit->setMinimumHeight(34);
            edit->setValidator(new QIntValidator(0, 999, edit));
            connect(edit, &QLineEdit::textChanged, this, &DynamicInputPanel::valuesChanged);
            globalEdits_[field.key] = edit;
            form->addRow(QString::fromStdString(field.label), edit);

            if (!field.description.empty()) {
                auto* hint = new QLabel(QString::fromStdString(field.description), frame);
                hint->setWordWrap(true);
                hint->setStyleSheet("color:#6f7b72; font-size:12px;");
                frameLayout->addLayout(form);
                frameLayout->addWidget(hint);
                form = new QFormLayout();
                form->setContentsMargins(0, 0, 0, 0);
                form->setHorizontalSpacing(12);
                form->setVerticalSpacing(8);
            }
        }

        if (form->rowCount() > 0) {
            frameLayout->addLayout(form);
        } else {
            delete form;
        }

        globalLayout->addWidget(frame);
    }

    auto* cardLayout = static_cast<QVBoxLayout*>(cardContainer_->layout());
    while (!cards_.empty()) {
        auto* card = cards_.back();
        cards_.pop_back();
        cardLayout->removeWidget(card);
        card->deleteLater();
    }

    for (const auto& group : role.inputGroups) {
        auto* card = new ColorInputCard(group, cardContainer_);
        connect(card, &ColorInputCard::valuesChanged, this, &DynamicInputPanel::valuesChanged);
        cards_.push_back(card);
        cardLayout->insertWidget(cardLayout->count() - 1, card);
    }
}

std::vector<ColorInputValues> DynamicInputPanel::values() const
{
    std::vector<ColorInputValues> result;
    result.reserve(cards_.size());
    for (const auto* card : cards_) {
        result.push_back(card->values());
    }
    return result;
}

std::optional<int> DynamicInputPanel::globalIntValue(const std::string& key) const
{
    const auto it = globalEdits_.find(key);
    return it == globalEdits_.end() ? std::nullopt : optionalInt(it->second);
}

void DynamicInputPanel::clearInputs()
{
    for (const auto& entry : globalEdits_) {
        entry.second->clear();
    }
    for (auto* card : cards_) {
        card->clear();
    }
}

std::optional<int> DynamicInputPanel::optionalInt(const QLineEdit* edit)
{
    if (!edit || edit->text().trimmed().isEmpty()) {
        return std::nullopt;
    }
    bool ok = false;
    const int value = edit->text().trimmed().toInt(&ok);
    return ok ? std::optional<int>(value) : std::nullopt;
}

} // namespace bbae
