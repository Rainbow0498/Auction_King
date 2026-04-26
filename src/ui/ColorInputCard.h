#pragma once

#include "core/ColorInput.h"
#include "core/RoleConfig.h"

#include <QFrame>
#include <QLineEdit>

#include <map>
#include <optional>

namespace bbae {

class ColorInputCard : public QFrame {
    Q_OBJECT

public:
    explicit ColorInputCard(const ColorGroupConfig& config, QWidget* parent = nullptr);

    ColorInputValues values() const;
    void clear();

signals:
    void valuesChanged();

private:
    ColorGroupConfig config_;
    std::map<std::string, QLineEdit*> edits_;

    QLineEdit* addField(const std::string& field, QWidget* parent);
    static QString fieldLabel(const std::string& field);
    static QString placeholderFor(const std::string& field);
    static std::optional<double> optionalDouble(const QLineEdit* edit);
    static std::optional<int> optionalInt(const QLineEdit* edit);
};

} // namespace bbae
