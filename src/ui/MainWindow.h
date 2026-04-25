#pragma once

#include "core/AuctionAdvisor.h"
#include "core/AuctionRule.h"
#include "core/Estimator.h"
#include "data/ItemDatabase.h"
#include "data/RoleDatabase.h"

#include <QComboBox>
#include <QLineEdit>
#include <QMainWindow>
#include <QStackedWidget>

#include <filesystem>
#include <optional>

namespace bbae {

class ResultPanel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void estimateCurrentRole();
    void resetInputs();
    void roleChanged(int index);

private:
    ItemDatabase itemDatabase_;
    RoleDatabase roleDatabase_;
    AuctionRules auctionRules_;
    Estimator estimator_;
    AuctionAdvisor advisor_;

    QComboBox* roleCombo_ = nullptr;
    QComboBox* roundCombo_ = nullptr;
    QStackedWidget* inputStack_ = nullptr;
    ResultPanel* resultPanel_ = nullptr;

    QLineEdit* totalCountEdit_ = nullptr;
    QLineEdit* goldAverageEdit_ = nullptr;
    QLineEdit* goldCountEdit_ = nullptr;
    QLineEdit* goldTotalSizeEdit_ = nullptr;
    QLineEdit* purpleAverageEdit_ = nullptr;
    QLineEdit* purpleCountEdit_ = nullptr;
    QLineEdit* purpleTotalSizeEdit_ = nullptr;

    void loadData();
    void buildUi();
    QWidget* buildVictorPanel();
    VictorClue readVictorClue() const;
    int currentRound() const;

    static std::filesystem::path dataDirectory();
    static std::optional<int> optionalInt(const QLineEdit* edit);
    static std::optional<double> optionalDouble(const QLineEdit* edit);
};

} // namespace bbae

