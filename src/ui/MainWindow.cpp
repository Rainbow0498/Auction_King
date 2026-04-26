#include "ui/MainWindow.h"

#include "data/ConfigLoader.h"
#include "data/RoleConfigLoader.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

#include <exception>

namespace bbae {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , combinationSolver_(itemDatabase_)
    , priceCalculator_(itemDatabase_)
{
    loadData();
    buildUi();
    applyCurrentRole();
}

void MainWindow::loadData()
{
    const auto dir = dataDirectory();
    try {
        itemDatabase_.loadFromFile(dir / "items.json");
        roles_ = RoleConfigLoader().loadFromFile(dir / "roles.json");
        ConfigLoader::loadJson(dir / "vision_config.json");
    } catch (const std::exception& ex) {
        QMessageBox::critical(this, QString::fromUtf8("数据加载失败"), QString::fromStdString(ex.what()));
    }
}

void MainWindow::buildUi()
{
    setWindowTitle(QString::fromUtf8("盲盒拍卖估值辅助工具"));
    resize(1280, 820);

    auto* central = new QWidget(this);
    central->setObjectName("appRoot");
    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(28, 24, 28, 28);
    root->setSpacing(16);

    auto* title = new QLabel(QString::fromUtf8("B站UP竞拍之王计算器"), central);
    title->setObjectName("appTitle");
    root->addWidget(title);

    roleSelector_ = new RoleSelectorWidget(central);
    roleSelector_->setObjectName("surfacePanel");
    roleSelector_->setRoles(roles_);
    root->addWidget(roleSelector_);

    auto* middle = new QHBoxLayout();
    middle->setSpacing(16);

    inputPanel_ = new DynamicInputPanel(central);
    inputPanel_->setObjectName("surfacePanel");
    inputPanel_->setMinimumWidth(470);
    inputPanel_->setMaximumWidth(560);
    middle->addWidget(inputPanel_, 0);

    auto* rightColumn = new QVBoxLayout();
    rightColumn->setSpacing(16);

    combinationPanel_ = new CombinationPanel(central);
    combinationPanel_->setObjectName("surfacePanel");
    estimatePanel_ = new EstimateResultPanel(central);
    estimatePanel_->setObjectName("surfacePanel");

    rightColumn->addWidget(combinationPanel_, 1);
    rightColumn->addWidget(estimatePanel_, 1);
    middle->addLayout(rightColumn, 1);

    root->addLayout(middle, 1);
    setCentralWidget(central);

    connect(roleSelector_, &RoleSelectorWidget::roleChanged, this, &MainWindow::applyCurrentRole);
    connect(inputPanel_, &DynamicInputPanel::calculateRequested, this, &MainWindow::calculate);
    connect(inputPanel_, &DynamicInputPanel::valuesChanged, this, &MainWindow::markDirty);

    applyStyle();
}

void MainWindow::applyCurrentRole()
{
    const auto* role = roleSelector_->currentRole();
    if (!role) {
        return;
    }
    inputPanel_->setRole(*role);
    combinationPanel_->clear();
    estimatePanel_->clear();
}

void MainWindow::calculate()
{
    const auto* role = roleSelector_->currentRole();
    if (!role) {
        return;
    }

    const auto inputs = inputPanel_->values();
    const auto totalCollectionCount = inputPanel_->globalIntValue("total_collection_count");
    const auto combinations = combinationSolver_.solve(*role, inputs, totalCollectionCount);
    const auto estimate = priceCalculator_.calculate(*role, inputs, combinations);
    combinationPanel_->showResult(combinations);
    estimatePanel_->showResult(estimate);
}

void MainWindow::markDirty()
{
    combinationPanel_->clear();
    estimatePanel_->clear();
}

void MainWindow::applyStyle()
{
    setStyleSheet(
        "QWidget#appRoot { background: #edf4ef; color: #24332c; }"
        "QLabel#appTitle { font-size: 24px; font-weight: 800; color: #1f3d31; padding: 0 2px 4px 2px; }"
        "QWidget#surfacePanel { background: #fffdf8; border: 1px solid #e1ded6; border-radius: 12px; }"
        "QLabel#sectionTitle { font-size: 15px; font-weight: 700; color: #24332c; }"
        "QLabel#cardTitle { font-size: 14px; font-weight: 700; color: #2b2b2b; }"
        "QFrame#colorInputCard { background: #ffffff; border: 1px solid #e7e0d4; border-radius: 10px; }"
        "QLineEdit { background: #fbfbf8; border: 1px solid #d9d5ca; border-radius: 7px; padding: 7px 9px; min-width: 92px; }"
        "QLineEdit:focus { border: 1px solid #227b52; background: #ffffff; }"
        "QPushButton { border: 1px solid #d7d0c4; border-radius: 8px; padding: 9px 16px; font-weight: 700; background: #f6f4ef; color: #304238; }"
        "QPushButton:checked { background: #227b52; color: #ffffff; border-color: #227b52; }"
        "QPushButton#primaryButton { background: #227b52; color: #ffffff; border-color: #227b52; }"
        "QPushButton#primaryButton:hover { background: #1c6c48; }"
        "QTableWidget { background: #ffffff; border: 1px solid #e7e0d4; border-radius: 8px; gridline-color: #eee8dc; }"
        "QHeaderView::section { background: #ede6d8; border: 0; padding: 8px; font-weight: 700; color: #33443a; }"
        "QLabel#riskLabel { background: #fff6df; border: 1px solid #f0dbad; border-radius: 8px; padding: 10px; color: #6c4f13; }"
        "QScrollArea { background: transparent; }"
    );
}

std::filesystem::path MainWindow::dataDirectory()
{
    const auto appDir = std::filesystem::path(QApplication::applicationDirPath().toStdString()) / "data";
    if (std::filesystem::exists(appDir)) {
        return appDir;
    }
    return std::filesystem::current_path() / "data";
}

} // namespace bbae
