#pragma once

#include "core/CombinationSolver.h"
#include "core/PriceCalculator.h"
#include "core/RoleConfig.h"
#include "data/ItemDatabase.h"
#include "ui/CombinationPanel.h"
#include "ui/DynamicInputPanel.h"
#include "ui/EstimateResultPanel.h"
#include "ui/RoleSelectorWidget.h"

#include <QMainWindow>

#include <filesystem>
#include <vector>

namespace bbae {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void applyCurrentRole();
    void calculate();
    void markDirty();

private:
    ItemDatabase itemDatabase_;
    std::vector<RoleConfig> roles_;
    CombinationSolver combinationSolver_;
    PriceCalculator priceCalculator_;

    RoleSelectorWidget* roleSelector_ = nullptr;
    DynamicInputPanel* inputPanel_ = nullptr;
    CombinationPanel* combinationPanel_ = nullptr;
    EstimateResultPanel* estimatePanel_ = nullptr;

    void loadData();
    void buildUi();
    void applyStyle();

    static std::filesystem::path dataDirectory();
};

} // namespace bbae

