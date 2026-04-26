#pragma once

#include "core/RoleConfig.h"

#include <QButtonGroup>
#include <QWidget>

#include <vector>

namespace bbae {

class RoleSelectorWidget : public QWidget {
    Q_OBJECT

public:
    explicit RoleSelectorWidget(QWidget* parent = nullptr);

    void setRoles(const std::vector<RoleConfig>& roles);
    const RoleConfig* currentRole() const;

signals:
    void roleChanged();

private:
    std::vector<RoleConfig> roles_;
    QButtonGroup* buttonGroup_ = nullptr;
};

} // namespace bbae

