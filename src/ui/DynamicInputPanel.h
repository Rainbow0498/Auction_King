#pragma once

#include "core/ColorInput.h"
#include "core/RoleConfig.h"
#include "ui/ColorInputCard.h"

#include <QLineEdit>
#include <QWidget>

#include <map>
#include <optional>
#include <vector>

namespace bbae {

class DynamicInputPanel : public QWidget {
    Q_OBJECT

public:
    explicit DynamicInputPanel(QWidget* parent = nullptr);

    void setRole(const RoleConfig& role);
    std::vector<ColorInputValues> values() const;
    std::optional<int> globalIntValue(const std::string& key) const;

signals:
    void calculateRequested();
    void valuesChanged();

public slots:
    void clearInputs();

private:
    std::map<std::string, QLineEdit*> globalEdits_;
    std::vector<ColorInputCard*> cards_;
    QWidget* cardContainer_ = nullptr;
    QWidget* globalContainer_ = nullptr;

    static std::optional<int> optionalInt(const QLineEdit* edit);
};

} // namespace bbae
