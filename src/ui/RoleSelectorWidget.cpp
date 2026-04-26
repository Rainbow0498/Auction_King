#include "ui/RoleSelectorWidget.h"

#include <QAbstractButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace bbae {

RoleSelectorWidget::RoleSelectorWidget(QWidget* parent)
    : QWidget(parent)
    , buttonGroup_(new QButtonGroup(this))
{
    buttonGroup_->setExclusive(true);
    connect(buttonGroup_, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, [this](QAbstractButton*) {
        emit roleChanged();
    });
}

void RoleSelectorWidget::setRoles(const std::vector<RoleConfig>& roles)
{
    roles_ = roles;

    if (layout()) {
        delete layout();
    }

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(18, 16, 18, 16);
    root->setSpacing(10);

    auto* title = new QLabel(QString::fromUtf8("角色选择区"), this);
    title->setObjectName("sectionTitle");
    root->addWidget(title);

    auto* row = new QHBoxLayout();
    row->setSpacing(10);

    for (auto* button : buttonGroup_->buttons()) {
        buttonGroup_->removeButton(button);
        button->deleteLater();
    }

    for (int i = 0; i < static_cast<int>(roles_.size()); ++i) {
        const auto& role = roles_[static_cast<std::size_t>(i)];
        auto* button = new QPushButton(QString::fromStdString(role.name), this);
        button->setCheckable(true);
        button->setMinimumHeight(42);
        button->setToolTip(QString::fromStdString(role.description));
        buttonGroup_->addButton(button, i);
        row->addWidget(button);
        if (i == 0) {
            button->setChecked(true);
        }
    }

    row->addStretch(1);
    root->addLayout(row);
}

const RoleConfig* RoleSelectorWidget::currentRole() const
{
    const int id = buttonGroup_->checkedId();
    if (id < 0 || id >= static_cast<int>(roles_.size())) {
        return nullptr;
    }
    return &roles_[static_cast<std::size_t>(id)];
}

} // namespace bbae
