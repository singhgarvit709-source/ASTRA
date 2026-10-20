#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QStackedWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QPainter>
#include <QTimer>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QMessageBox>
#include <QScrollArea>
#include <QProgressBar>
#include <QTextEdit>
#include <QRandomGenerator>
#include <QSizePolicy>
#include <QBrush>
#include <QPen>
#include <QColor>
#include <QFont>
#include <QVector>
#include <cmath>

struct SpaceObject {
    QString name;
    QString type;
    double altitude;
    double risk;
    double phase;
};

static QString appStyle()
{
    return QStringLiteral(R"(
        QWidget { background: #0b1220; color: #e6edf7; font-family: "Segoe UI"; font-size: 10pt; }
        QMainWindow { background: #0b1220; }
        QFrame#Sidebar { background: #080e19; border-right: 1px solid #1e293b; }
        QFrame#Topbar { background: #0f1728; border-bottom: 1px solid #1e293b; }
        QLabel#Brand { color: #f8fafc; font-size: 19pt; font-weight: 700; }
        QLabel#Muted { color: #8da1bd; }
        QLabel#PageTitle { font-size: 22pt; font-weight: 700; color: #f8fafc; }
        QLabel#CardTitle { color: #91a4c0; font-size: 9pt; }
        QLabel#Metric { color: #f8fafc; font-size: 23pt; font-weight: 700; }
        QFrame#Panel, QWidget#Panel { background: #111c2e; border: 1px solid #22324a; border-radius: 12px; }
        QPushButton { background: #17243a; border: 1px solid #2b3d58; border-radius: 8px; padding: 9px 12px; color: #dce8f8; text-align: left; }
        QPushButton:hover { background: #20314b; border-color: #3b82f6; }
        QPushButton#NavButton { background: transparent; border: 0; padding: 11px 12px; color: #9fb0c8; text-align: left; }
        QPushButton#NavButton:checked, QPushButton#NavButton:hover { background: #142641; color: #67d7ff; border-left: 3px solid #38bdf8; }
        QPushButton#Primary { background: #087ea4; border: 1px solid #0891b2; color: white; font-weight: 600; text-align: center; }
        QPushButton#Primary:hover { background: #0891b2; }
        QTableWidget { background: #0e192a; alternate-background-color: #132138; gridline-color: #24354d; border: 1px solid #263750; border-radius: 8px; selection-background-color: #164e63; }
        QHeaderView::section { background: #17243a; color: #a9bdd7; border: 0; padding: 9px; font-weight: 600; }
        QTableWidget::item { padding: 6px; }
        QLineEdit, QComboBox, QDoubleSpinBox, QTextEdit { background: #0b1525; border: 1px solid #2b3d58; border-radius: 7px; padding: 8px; }
        QProgressBar { background: #0b1525; border: 0; border-radius: 4px; height: 8px; text-align: center; }
        QProgressBar::chunk { background: #22d3ee; border-radius: 4px; }
        QScrollArea { border: none; }
    )");
}

class OrbitWidget : public QWidget
{
public:
    explicit OrbitWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setMinimumSize(420, 340);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    void setObjects(const QVector<SpaceObject> *objects) { objects_ = objects; update(); }
    void setRunning(bool running) { running_ = running; update(); }
    void advance() { if (running_) { angle_ += 0.012; update(); } }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), QColor("#0b1525"));

        // Subtle star field
        for (int i = 0; i < 75; ++i) {
            const int x = (i * 97 + 31) % qMax(1, width());
            const int y = (i * 53 + 19) % qMax(1, height());
            p.setPen(QPen(QColor(116, 158, 202, (i % 3 + 1) * 35), (i % 4 == 0) ? 2 : 1));
            p.drawPoint(x, y);
        }

        const QPointF c(width() * 0.5, height() * 0.53);
        const qreal earthR = qMin(width(), height()) * 0.19;
        for (int i = 0; i < 3; ++i) {
            p.setPen(QPen(QColor(55, 157, 205, 80 - i * 18), 1));
            p.setBrush(Qt::NoBrush);
            p.drawEllipse(c, earthR + 35 + i * 37, earthR + 16 + i * 27);
        }

        QRadialGradient earth(c, earthR * 1.2);
        earth.setColorAt(0.0, QColor("#54c9e8"));
        earth.setColorAt(0.55, QColor("#1675a8"));
        earth.setColorAt(1.0, QColor("#10294c"));
        p.setPen(QPen(QColor("#4dd8f5"), 1.2));
        p.setBrush(earth);
        p.drawEllipse(c, earthR, earthR);

        // Stylized continent shapes
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(72, 190, 151, 180));
        p.drawEllipse(QRectF(c.x() - earthR * .55, c.y() - earthR * .52, earthR * .55, earthR * .32));
        p.drawEllipse(QRectF(c.x() + earthR * .05, c.y() - earthR * .12, earthR * .43, earthR * .62));
        p.drawEllipse(QRectF(c.x() - earthR * .4, c.y() + earthR * .25, earthR * .26, earthR * .27));

        if (objects_) {
            for (int i = 0; i < objects_->size(); ++i) {
                const SpaceObject &o = objects_->at(i);
                const qreal rx = earthR + 35 + (i % 3) * 37;
                const qreal ry = earthR + 16 + (i % 3) * 27;
                const qreal a = o.phase + angle_ * (o.type == "Debris" ? 1.25 : 0.75);
                const QPointF pt(c.x() + std::cos(a) * rx, c.y() + std::sin(a) * ry);
                const QColor col = o.type == "Debris" ? QColor("#fb7185") : QColor("#67e8f9");
                p.setPen(QPen(col, 1));
                p.setBrush(col);
                p.drawEllipse(pt, o.type == "Debris" ? 4.0 : 5.5, o.type == "Debris" ? 4.0 : 5.5);
                if (i < 5) {
                    p.setPen(QColor("#c8d8ec"));
                    p.drawText(pt + QPointF(8, -7), o.name);
                }
            }
        }

        p.setPen(QColor("#8da1bd"));
        p.drawText(16, 25, "ORBITAL VIEW  /  SIMULATED OBJECTS");
        p.setPen(QColor("#4bd8f2"));
        p.drawText(16, height() - 16, running_ ? "SIMULATION RUNNING" : "SIMULATION PAUSED");
    }

private:
    const QVector<SpaceObject> *objects_ = nullptr;
    qreal angle_ = 0.0;
    bool running_ = false;
};

static QFrame *makePanel(const QString &title, QVBoxLayout *&layoutOut)
{
    auto *panel = new QFrame;
    panel->setObjectName("Panel");
    layoutOut = new QVBoxLayout(panel);
    layoutOut->setContentsMargins(16, 14, 16, 16);
    layoutOut->setSpacing(10);
    auto *heading = new QLabel(title);
    heading->setStyleSheet("font-weight: 700; color: #eaf2ff; font-size: 11pt;");
    layoutOut->addWidget(heading);
    return panel;
}

static QFrame *metricCard(const QString &title, const QString &value, const QString &caption)
{
    auto *card = new QFrame;
    card->setObjectName("Panel");
    auto *l = new QVBoxLayout(card);
    l->setContentsMargins(16, 14, 16, 14);
    l->setSpacing(5);
    auto *t = new QLabel(title);
    t->setObjectName("CardTitle");
    auto *v = new QLabel(value);
    v->setObjectName("Metric");
    auto *c = new QLabel(caption);
    c->setObjectName("Muted");
    l->addWidget(t);
    l->addWidget(v);
    l->addWidget(c);
    l->addStretch();
    return card;
}

class MainWindow : public QMainWindow
{
public:
    MainWindow()
    {
        setWindowTitle("ASTRA | Space Traffic & Risk Avoidance");
        resize(1440, 900);
        setMinimumSize(1080, 700);
        setStyleSheet(appStyle());

        objects_ = {
            {"ORION-01", "Satellite", 550, 12, 0.3},
            {"ASTRA-EYE", "Satellite", 620, 28, 1.7},
            {"DEBRIS-17", "Debris", 570, 81, 2.8},
            {"NAVSTAR-04", "Satellite", 720, 9, 4.1},
            {"FRAG-203", "Debris", 610, 64, 5.2},
            {"COMMS-12", "Satellite", 680, 21, 0.9}
        };

        auto *root = new QWidget;
        auto *rootLayout = new QHBoxLayout(root);
        rootLayout->setContentsMargins(0, 0, 0, 0);
        rootLayout->setSpacing(0);

        auto *sidebar = new QFrame;
        sidebar->setObjectName("Sidebar");
        sidebar->setFixedWidth(225);
        auto *side = new QVBoxLayout(sidebar);
        side->setContentsMargins(14, 20, 14, 16);
        side->setSpacing(8);

        auto *brand = new QLabel("✦ ASTRA");
        brand->setObjectName("Brand");
        side->addWidget(brand);
        auto *sub = new QLabel("SPACE TRAFFIC INTELLIGENCE");
        sub->setObjectName("Muted");
        sub->setStyleSheet("font-size: 8pt; letter-spacing: 1px;");
        side->addWidget(sub);
        side->addSpacing(24);
        auto *navLabel = new QLabel("WORKSPACE");
        navLabel->setObjectName("Muted");
        navLabel->setStyleSheet("font-size: 8pt; font-weight: 700;");
        side->addWidget(navLabel);

        stack_ = new QStackedWidget;
        const QStringList names = {"Mission Overview", "Orbit Monitor", "Collision Risk",
                                   "Maneuver Planner", "Analytics", "Learn ASTRA"};
        const QStringList symbols = {"⌂", "◎", "⚠", "➤", "▥", "ⓘ"};
        for (int i = 0; i < names.size(); ++i) {
            auto *b = new QPushButton(symbols[i] + "   " + names[i]);
            b->setObjectName("NavButton");
            b->setCheckable(true);
            b->setChecked(i == 0);
            side->addWidget(b);
            navButtons_.append(b);
            connect(b, &QPushButton::clicked, this, [this, i]() { showPage(i); });
        }
        side->addStretch();
        auto *status = new QLabel("●  SYSTEM ONLINE\n\nPrototype mode\nSynthetic data only");
        status->setStyleSheet("color: #7dd3a7; background: #10251f; border: 1px solid #1e4a3a; border-radius: 8px; padding: 12px;");
        side->addWidget(status);

        auto *right = new QWidget;
        auto *rightLayout = new QVBoxLayout(right);
        rightLayout->setContentsMargins(0, 0, 0, 0);
        rightLayout->setSpacing(0);

        auto *top = new QFrame;
        top->setObjectName("Topbar");
        auto *topLayout = new QHBoxLayout(top);
        topLayout->setContentsMargins(22, 12, 22, 12);
        auto *crumb = new QLabel("ASTRA  /  <span style='color:#eaf2ff'>Mission Overview</span>");
        crumb->setTextFormat(Qt::RichText);
        crumb->setObjectName("Muted");
        pageCrumb_ = crumb;
        topLayout->addWidget(crumb);
        topLayout->addStretch();
        auto *utc = new QLabel;
        utc->setObjectName("Muted");
        topLayout->addWidget(utc);
        auto *utcTimer = new QTimer(this);
        connect(utcTimer, &QTimer::timeout, this, [utc]() {
            utc->setText("UTC  " + QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd  HH:mm:ss"));
        });
        utcTimer->start(1000);
        utc->setText("UTC  " + QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd  HH:mm:ss"));
        rightLayout->addWidget(top);

        stack_->addWidget(makeOverview());
        stack_->addWidget(makeOrbitPage());
        stack_->addWidget(makeRiskPage());
        stack_->addWidget(makeManeuverPage());
        stack_->addWidget(makeAnalyticsPage());
        stack_->addWidget(makeLearnPage());
        rightLayout->addWidget(stack_, 1);

        rootLayout->addWidget(sidebar);
        rootLayout->addWidget(right, 1);
        setCentralWidget(root);

        auto *orbitTimer = new QTimer(this);
        connect(orbitTimer, &QTimer::timeout, this, [this]() {
            if (orbit_) orbit_->advance();
        });
        orbitTimer->start(40);
        updateObjectTable();
    }

private:
    QVector<SpaceObject> objects_;
    QStackedWidget *stack_ = nullptr;
    QLabel *pageCrumb_ = nullptr;
    QVector<QPushButton *> navButtons_;
    OrbitWidget *orbit_ = nullptr;
    QTableWidget *objectTable_ = nullptr;
    QTableWidget *riskTable_ = nullptr;
    QLabel *simulationState_ = nullptr;
    bool running_ = true;

    void showPage(int index)
    {
        stack_->setCurrentIndex(index);
        for (int i = 0; i < navButtons_.size(); ++i) navButtons_[i]->setChecked(i == index);
        const QStringList names = {"Mission Overview", "Orbit Monitor", "Collision Risk",
                                   "Maneuver Planner", "Analytics", "Learn ASTRA"};
        pageCrumb_->setText("ASTRA  /  <span style='color:#eaf2ff'>" + names.value(index) + "</span>");
        if (orbit_) orbit_->setRunning(running_);
    }

    QWidget *pageBase(const QString &title, const QString &subtitle, QVBoxLayout **layoutOut)
    {
        auto *page = new QWidget;
        auto *outer = new QVBoxLayout(page);
        outer->setContentsMargins(24, 22, 24, 24);
        outer->setSpacing(16);
        auto *titleLabel = new QLabel(title);
        titleLabel->setObjectName("PageTitle");
        auto *subLabel = new QLabel(subtitle);
        subLabel->setObjectName("Muted");
        outer->addWidget(titleLabel);
        outer->addWidget(subLabel);
        *layoutOut = outer;
        return page;
    }

    QWidget *makeOverview()
    {
        QVBoxLayout *outer = nullptr;
        QWidget *page = pageBase("Mission Overview", "Live situational awareness for a simulated orbital environment.", &outer);
        auto *metrics = new QGridLayout;
        metrics->setSpacing(12);
        metrics->addWidget(metricCard("TRACKED OBJECTS", QString::number(objects_.size()), "Catalogued satellites + debris"), 0, 0);
        metrics->addWidget(metricCard("ELEVATED RISKS", "02", "Synthetic risk threshold > 60"), 0, 1);
        metrics->addWidget(metricCard("MANEUVERS PLANNED", "01", "Demo proposal available"), 0, 2);
        metrics->addWidget(metricCard("SYSTEM STATUS", "ONLINE", "Prototype simulation"), 0, 3);
        outer->addLayout(metrics);

        auto *row = new QHBoxLayout;
        row->setSpacing(14);
        QVBoxLayout *orbitLayout = nullptr;
        auto *orbitPanel = makePanel("Orbital environment", orbitLayout);
        orbit_ = new OrbitWidget;
        orbit_->setObjects(&objects_);
        orbit_->setRunning(running_);
        orbitLayout->addWidget(orbit_, 1);
        auto *orbitControls = new QHBoxLayout;
        auto *toggle = new QPushButton("Pause simulation");
        toggle->setObjectName("Primary");
        connect(toggle, &QPushButton::clicked, this, [this, toggle]() {
            running_ = !running_;
            if (orbit_) orbit_->setRunning(running_);
            toggle->setText(running_ ? "Pause simulation" : "Start simulation");
            if (simulationState_) simulationState_->setText(running_ ? "RUNNING" : "PAUSED");
        });
        simulationState_ = new QLabel("RUNNING");
        simulationState_->setStyleSheet("color:#67e8a5; font-weight:700;");
        orbitControls->addWidget(toggle);
        orbitControls->addStretch();
        orbitControls->addWidget(simulationState_);
        orbitLayout->addLayout(orbitControls);
        row->addWidget(orbitPanel, 2);

        QVBoxLayout *activityLayout = nullptr;
        auto *activityPanel = makePanel("Mission activity", activityLayout);
        const QStringList activities = {
            "●  Object catalogue initialized",
            "●  Synthetic risk scan complete",
            "●  Orbit monitor connected",
            "●  Demo maneuver ready"
        };
        for (const QString &a : activities) {
            auto *l = new QLabel(a);
            l->setWordWrap(true);
            l->setStyleSheet("padding: 10px 2px; color:#b8c8de; border-bottom:1px solid #22324a;");
            activityLayout->addWidget(l);
        }
        activityLayout->addStretch();
        auto *hint = new QLabel("Prototype uses illustrative data. Connect validated algorithms before operational use.");
        hint->setWordWrap(true);
        hint->setObjectName("Muted");
        activityLayout->addWidget(hint);
        row->addWidget(activityPanel, 1);
        outer->addLayout(row, 1);
        return page;
    }

    QWidget *makeOrbitPage()
    {
        QVBoxLayout *outer = nullptr;
        QWidget *page = pageBase("Orbit Monitor", "Inspect the simulated satellite and debris catalogue.", &outer);
        QVBoxLayout *orbitLayout = nullptr;
        auto *panel = makePanel("Orbital visualization", orbitLayout);
        // Reuse the same widget only on the overview; this page has its own view.
        auto *view = new OrbitWidget;
        view->setObjects(&objects_);
        view->setRunning(running_);
        auto *timer = new QTimer(view);
        connect(timer, &QTimer::timeout, view, [view]() { view->advance(); });
        timer->start(40);
        orbitLayout->addWidget(view, 1);
        auto *note = new QLabel("Cyan markers represent satellites; coral markers represent debris. The illustration is not an ephemeris or a physically accurate orbit.");
        note->setObjectName("Muted");
        note->setWordWrap(true);
        orbitLayout->addWidget(note);
        outer->addWidget(panel, 1);
        return page;
    }

    QWidget *makeRiskPage()
    {
        QVBoxLayout *outer = nullptr;
        QWidget *page = pageBase("Collision Risk", "Review synthetic object-level risk scores; these are not real collision probabilities.", &outer);
        QVBoxLayout *tableLayout = nullptr;
        auto *panel = makePanel("Risk register", tableLayout);
        riskTable_ = new QTableWidget(0, 4);
        riskTable_->setHorizontalHeaderLabels({"Object ID", "Type", "Risk score", "Classification"});
        riskTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        riskTable_->verticalHeader()->setVisible(false);
        riskTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        riskTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableLayout->addWidget(riskTable_);
        auto *refresh = new QPushButton("Refresh demo risk values");
        refresh->setObjectName("Primary");
        connect(refresh, &QPushButton::clicked, this, [this]() {
            for (auto &o : objects_) {
                const int delta = QRandomGenerator::global()->bounded(-8, 9);
                o.risk = qBound(1.0, o.risk + delta, 99.0);
            }
            updateObjectTable();
            if (orbit_) orbit_->update();
        });
        tableLayout->addWidget(refresh, 0, Qt::AlignLeft);
        outer->addWidget(panel, 1);
        updateObjectTable();
        return page;
    }

    QWidget *makeManeuverPage()
    {
        QVBoxLayout *outer = nullptr;
        QWidget *page = pageBase("Maneuver Planner", "Create an illustrative proposal for a selected object.", &outer);
        auto *row = new QHBoxLayout;
        QVBoxLayout *formLayout = nullptr;
        auto *formPanel = makePanel("Proposal settings", formLayout);
        auto *form = new QFormLayout;
        auto *target = new QComboBox;
        for (const auto &o : objects_) target->addItem(o.name);
        auto *strategy = new QComboBox;
        strategy->addItems({"Small along-track adjustment", "Radial adjustment", "Review only — no maneuver"});
        auto *horizon = new QComboBox;
        horizon->addItems({"Next 6 hours", "Next 24 hours", "Next 72 hours"});
        form->addRow("Target object", target);
        form->addRow("Strategy", strategy);
        form->addRow("Planning horizon", horizon);
        formLayout->addLayout(form);
        auto *generate = new QPushButton("Generate demo proposal");
        generate->setObjectName("Primary");
        formLayout->addWidget(generate);
        row->addWidget(formPanel, 1);

        QVBoxLayout *resultLayout = nullptr;
        auto *resultPanel = makePanel("Proposal output", resultLayout);
        auto *result = new QTextEdit;
        result->setReadOnly(true);
        result->setPlainText("No proposal generated yet.\n\nChoose a target and strategy, then select Generate demo proposal.");
        resultLayout->addWidget(result, 1);
        connect(generate, &QPushButton::clicked, this, [target, strategy, horizon, result]() {
            result->setPlainText(
                "ASTRA — DEMO MANEUVER PROPOSAL\n\n"
                "Target: " + target->currentText() +
                "\nStrategy: " + strategy->currentText() +
                "\nHorizon: " + horizon->currentText() +
                "\n\nStatus: illustrative proposal generated\n"
                "Validation: NOT PERFORMED\n"
                "Fuel / delta-v: NOT CALCULATED\n"
                "Secondary collision check: NOT CONNECTED\n\n"
                "This output is a GUI demonstration only. It is not an actionable spacecraft command.");
        });
        row->addWidget(resultPanel, 1);
        outer->addLayout(row, 1);
        return page;
    }

    QWidget *makeAnalyticsPage()
    {
        QVBoxLayout *outer = nullptr;
        QWidget *page = pageBase("Analytics", "High-level prototype indicators for the simulated catalogue.", &outer);
        auto *grid = new QGridLayout;
        grid->setSpacing(14);
        grid->addWidget(metricCard("CATALOGUE SIZE", QString::number(objects_.size()), "Objects in local demo list"), 0, 0);
        grid->addWidget(metricCard("DEBRIS OBJECTS", QString::number(countType("Debris")), "Synthetic objects"), 0, 1);
        grid->addWidget(metricCard("SATELLITES", QString::number(countType("Satellite")), "Synthetic objects"), 0, 2);
        grid->addWidget(metricCard("MEAN RISK SCORE", QString::number(meanRisk(), 'f', 1), "Arbitrary 0–100 scale"), 0, 3);
        outer->addLayout(grid);
        QVBoxLayout *chartLayout = nullptr;
        auto *panel = makePanel("Risk score distribution", chartLayout);
        for (const auto &o : objects_) {
            auto *line = new QHBoxLayout;
            auto *name = new QLabel(o.name);
            name->setMinimumWidth(110);
            auto *bar = new QProgressBar;
            bar->setRange(0, 100);
            bar->setValue(static_cast<int>(o.risk));
            bar->setFormat(QString::number(static_cast<int>(o.risk)) + " / 100");
            line->addWidget(name);
            line->addWidget(bar, 1);
            chartLayout->addLayout(line);
        }
        auto *note = new QLabel("These bars visualize synthetic demo scores, not probability of collision.");
        note->setObjectName("Muted");
        chartLayout->addWidget(note);
        outer->addWidget(panel, 1);
        return page;
    }

    QWidget *makeLearnPage()
    {
        QVBoxLayout *outer = nullptr;
        QWidget *page = pageBase("Learn ASTRA", "A quick guide to the components of the project.", &outer);
        const QStringList titles = {
            "01  Object catalogue",
            "02  Dynamic graph",
            "03  Collision-risk estimation",
            "04  Priority queue",
            "05  Maneuver planning",
            "06  Machine-learning extension"
        };
        const QStringList descriptions = {
            "Stores simulated satellites and debris with identifiers and illustrative attributes.",
            "Represents objects or encounters as graph nodes and relationships as edges; actual graph design depends on the backend.",
            "A placeholder score is shown in this prototype. A real model needs validated inputs, units, and ground-truth evaluation.",
            "Can rank candidate encounters by a defined risk metric so the most urgent items are reviewed first.",
            "A future algorithm can compare candidate maneuvers against constraints, cost, and secondary-risk checks.",
            "A trained model may estimate risk from prepared data; model accuracy and limits must be documented."
        };
        for (int i = 0; i < titles.size(); ++i) {
            QVBoxLayout *cardLayout = nullptr;
            auto *card = makePanel(titles[i], cardLayout);
            auto *desc = new QLabel(descriptions[i]);
            desc->setWordWrap(true);
            desc->setObjectName("Muted");
            cardLayout->addWidget(desc);
            outer->addWidget(card);
        }
        outer->addStretch();
        return page;
    }

    int countType(const QString &type) const
    {
        int n = 0;
        for (const auto &o : objects_) if (o.type == type) ++n;
        return n;
    }

    double meanRisk() const
    {
        if (objects_.isEmpty()) return 0.0;
        double total = 0.0;
        for (const auto &o : objects_) total += o.risk;
        return total / objects_.size();
    }

    void updateObjectTable()
    {
        if (riskTable_) {
            riskTable_->setRowCount(objects_.size());
            for (int i = 0; i < objects_.size(); ++i) {
                const auto &o = objects_[i];
                const QString classification = o.risk >= 70 ? "CRITICAL" : (o.risk >= 40 ? "ELEVATED" : "NOMINAL");
                const QStringList values = {o.name, o.type, QString::number(static_cast<int>(o.risk)), classification};
                for (int col = 0; col < values.size(); ++col) {
                    auto *item = new QTableWidgetItem(values[col]);
                    if (col == 3) {
                        item->setForeground(o.risk >= 70 ? QColor("#fb7185") : (o.risk >= 40 ? QColor("#fbbf24") : QColor("#6ee7b7")));
                    }
                    riskTable_->setItem(i, col, item);
                }
            }
        }
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("ASTRA");
    QApplication::setOrganizationName("ASTRA Project");
    MainWindow window;
    window.show();
    return app.exec();
}
