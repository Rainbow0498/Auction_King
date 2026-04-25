#include "ui/MainWindow.h"

#include "data/ConfigLoader.h"
#include "ui/ResultPanel.h"

#include <QApplication>
#include <QDoubleValidator>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <exception>

namespace bbae {
namespace {

QLineEdit* makeLineEdit(const QString& placeholder, QWidget* parent)
{
    auto* edit = new QLineEdit(parent);
    edit->setPlaceholderText(placeholder);
    edit->setMinimumHeight(34);
    return edit;
}

QLabel* sectionTitle(const QString& text, QWidget* parent)
{
    auto* label = new QLabel(text, parent);
    QFont font = label->font();
    font.setBold(true);
    label->setFont(font);
    return label;
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , estimator_(itemDatabase_)
{
    loadData();
    buildUi();
}

void MainWindow::loadData()
{
    const auto dir = dataDirectory();
    try {
        itemDatabase_.loadFromFile(dir / "items.json");
        auctionRules_ = ConfigLoader::loadAuctionRules(dir / "auction_rules.json");
        roleDatabase_.loadFromFile(dir / "roles.json");
        ConfigLoader::loadJson(dir / "vision_config.json");
    } catch (const std::exception& ex) {
        QMessageBox::critical(this, QString::fromUtf8("数据加载失败"), QString::fromStdString(ex.what()));
    }
}

void MainWindow::buildUi()
{
    setWindowTitle(QString::fromUtf8("盲盒拍卖估值辅助工具"));
    resize(1180, 720);

    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(22, 18, 22, 22);
    root->setSpacing(16);

    auto* header = new QHBoxLayout();
    auto* title = new QLabel(QString::fromUtf8("B站UP竞拍之王计算器"), central);
    QFont titleFont = title->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);

    roleCombo_ = new QComboBox(central);
    for (const auto& role : roleDatabase_.roles()) {
        if (role.enabled) {
            roleCombo_->addItem(QString::fromStdString(role.name), QString::fromStdString(role.id));
        }
    }
    if (roleCombo_->count() == 0) {
        roleCombo_->addItem(QString::fromUtf8("维克多"), "victor");
    }

    roundCombo_ = new QComboBox(central);
    for (const auto& rule : auctionRules_) {
        roundCombo_->addItem(QString::fromUtf8("第 %1 轮 · 倍率 %2").arg(rule.round).arg(rule.multiplier), rule.round);
    }
    if (roundCombo_->count() == 0) {
        roundCombo_->addItem(QString::fromUtf8("第 1 轮 · 倍率 1.0"), 1);
    }

    header->addWidget(title, 1);
    header->addWidget(new QLabel(QString::fromUtf8("角色"), central));
    header->addWidget(roleCombo_);
    header->addSpacing(10);
    header->addWidget(new QLabel(QString::fromUtf8("拍卖轮次"), central));
    header->addWidget(roundCombo_);

    auto* content = new QHBoxLayout();
    content->setSpacing(18);

    inputStack_ = new QStackedWidget(central);
    inputStack_->setMinimumWidth(360);
    inputStack_->setMaximumWidth(420);
    inputStack_->addWidget(buildVictorPanel());

    resultPanel_ = new ResultPanel(central);
    resultPanel_->showEmptyMessage(QString::fromUtf8("请选择角色并填写参数，然后点击智能推理。"));

    content->addWidget(inputStack_);
    content->addWidget(resultPanel_, 1);

    root->addLayout(header);
    root->addLayout(content, 1);
    setCentralWidget(central);

    connect(roleCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::roleChanged);

    setStyleSheet(
        "QMainWindow { background: #dcefe8; }"
        "QGroupBox { background: #fffdf8; border: 1px solid #e5dfd2; border-radius: 8px; margin-top: 12px; padding: 12px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 4px; font-weight: 600; }"
        "QLineEdit, QComboBox { background: white; border: 1px solid #d9d5ca; border-radius: 6px; padding: 6px 8px; }"
        "QPushButton { border: 0; border-radius: 7px; padding: 10px 14px; font-weight: 600; }"
        "QPushButton#primaryButton { background: #227b52; color: white; }"
        "QPushButton#secondaryButton { background: #eef1ed; color: #34423a; border: 1px solid #d4dbd4; }"
        "QTableWidget { background: #fffdf8; border: 1px solid #e5dfd2; border-radius: 8px; gridline-color: #eee8dc; }"
        "QHeaderView::section { background: #eee6d3; border: 0; padding: 7px; font-weight: 600; }"
    );
}

QWidget* MainWindow::buildVictorPanel()
{
    auto* panel = new QWidget(this);
    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    auto* group = new QGroupBox(QString::fromUtf8("维克多参数 · 金色 + 紫色 → 红色"), panel);
    auto* form = new QVBoxLayout(group);
    form->setSpacing(12);

    totalCountEdit_ = makeLineEdit(QString::fromUtf8("例 12"), group);
    totalCountEdit_->setValidator(new QIntValidator(1, 80, totalCountEdit_));
    auto* totalForm = new QFormLayout();
    totalForm->addRow(QString::fromUtf8("总件数"), totalCountEdit_);
    form->addLayout(totalForm);

    auto addQualityGrid = [&](const QString& title,
                              QLineEdit*& average,
                              QLineEdit*& count,
                              QLineEdit*& totalSize) {
        form->addWidget(sectionTitle(title, group));
        auto* grid = new QGridLayout();
        average = makeLineEdit(QString::fromUtf8("例 3.55"), group);
        count = makeLineEdit(QString::fromUtf8("例 4"), group);
        totalSize = makeLineEdit(QString::fromUtf8("例 12"), group);
        average->setValidator(new QDoubleValidator(0.0, 100000.0, 3, average));
        count->setValidator(new QIntValidator(0, 80, count));
        totalSize->setValidator(new QIntValidator(0, 999, totalSize));
        grid->addWidget(new QLabel(QString::fromUtf8("均值(w)"), group), 0, 0);
        grid->addWidget(new QLabel(QString::fromUtf8("存量(件数)"), group), 0, 1);
        grid->addWidget(new QLabel(QString::fromUtf8("总格数"), group), 0, 2);
        grid->addWidget(average, 1, 0);
        grid->addWidget(count, 1, 1);
        grid->addWidget(totalSize, 1, 2);
        form->addLayout(grid);
    };

    addQualityGrid(QString::fromUtf8("金色"), goldAverageEdit_, goldCountEdit_, goldTotalSizeEdit_);
    addQualityGrid(QString::fromUtf8("紫色"), purpleAverageEdit_, purpleCountEdit_, purpleTotalSizeEdit_);

    auto* hint = new QLabel(QString::fromUtf8("提示：均值单位为 w；空字段会自动枚举可能组合。"), group);
    hint->setWordWrap(true);
    form->addWidget(hint);

    auto* primary = new QPushButton(QString::fromUtf8("智能推理 & 估价"), group);
    primary->setObjectName("primaryButton");
    auto* reset = new QPushButton(QString::fromUtf8("重置所有输入"), group);
    reset->setObjectName("secondaryButton");
    form->addWidget(primary);
    form->addWidget(reset);
    form->addStretch(1);

    connect(primary, &QPushButton::clicked, this, &MainWindow::estimateCurrentRole);
    connect(reset, &QPushButton::clicked, this, &MainWindow::resetInputs);

    layout->addWidget(group);
    return panel;
}

void MainWindow::estimateCurrentRole()
{
    const QString roleId = roleCombo_->currentData().toString();
    if (roleId != "victor") {
        resultPanel_->showEmptyMessage(QString::fromUtf8("该角色的输入面板尚未实现。"));
        return;
    }

    const auto estimate = estimator_.estimateVictor(readVictorClue());
    const auto advice = advisor_.advise(estimate, auctionRules_, currentRound());
    resultPanel_->showResult(estimate, advice);
}

void MainWindow::resetInputs()
{
    for (auto* edit : {totalCountEdit_, goldAverageEdit_, goldCountEdit_, goldTotalSizeEdit_,
                       purpleAverageEdit_, purpleCountEdit_, purpleTotalSizeEdit_}) {
        if (edit) {
            edit->clear();
        }
    }
    resultPanel_->showEmptyMessage(QString::fromUtf8("输入已重置。"));
}

void MainWindow::roleChanged(int index)
{
    inputStack_->setCurrentIndex(index >= 0 ? 0 : 0);
    resultPanel_->showEmptyMessage(QString::fromUtf8("请选择角色并填写参数，然后点击智能推理。"));
}

VictorClue MainWindow::readVictorClue() const
{
    VictorClue clue;
    clue.totalHighQualityCount = optionalInt(totalCountEdit_);
    clue.gold.averagePriceWan = optionalDouble(goldAverageEdit_);
    clue.gold.count = optionalInt(goldCountEdit_);
    clue.gold.totalSize = optionalInt(goldTotalSizeEdit_);
    clue.purple.averagePriceWan = optionalDouble(purpleAverageEdit_);
    clue.purple.count = optionalInt(purpleCountEdit_);
    clue.purple.totalSize = optionalInt(purpleTotalSizeEdit_);
    return clue;
}

int MainWindow::currentRound() const
{
    return roundCombo_->currentData().toInt();
}

std::filesystem::path MainWindow::dataDirectory()
{
    const auto appDir = std::filesystem::path(QApplication::applicationDirPath().toStdString()) / "data";
    if (std::filesystem::exists(appDir)) {
        return appDir;
    }
    return std::filesystem::current_path() / "data";
}

std::optional<int> MainWindow::optionalInt(const QLineEdit* edit)
{
    const auto text = edit->text().trimmed();
    if (text.isEmpty()) {
        return std::nullopt;
    }
    bool ok = false;
    const int value = text.toInt(&ok);
    return ok ? std::optional<int>(value) : std::nullopt;
}

std::optional<double> MainWindow::optionalDouble(const QLineEdit* edit)
{
    const auto text = edit->text().trimmed();
    if (text.isEmpty()) {
        return std::nullopt;
    }
    bool ok = false;
    const double value = text.toDouble(&ok);
    return ok ? std::optional<double>(value) : std::nullopt;
}

} // namespace bbae
