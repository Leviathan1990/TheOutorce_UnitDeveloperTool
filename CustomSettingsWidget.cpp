#include "CustomSettingsWidget.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QDebug>

// =============================================================================
// Complete list of CustomSettings names from IDA reverse engineering
// =============================================================================
// Sources: COutforceObject (sub_47E060), CGridMemberTemplate (sub_470400),
//          CUnitTemplate (sub_46C2A0), CUnitWeaponTemplate (sub_4721C0)
// Updated: December 2025 - Final version with full IDA analysis
// Total: ~125 settings
// =============================================================================
const QStringList CustomSettingsWidget::s_knownSettingNames = {

// =========================================================================
// COutforceObject (Base Object Settings - sub_47E060)
// Base class for all game objects
// =========================================================================

// --- Identity ---
    "Name",                      // offset 800 - Object name (string)
    "Race",                      // offset 720 - Race template reference (string, default "Default")

    // --- Resources ---
    "ResourceType",              // offset 808 - Resource type ID (int)
    "ResourceHeld",              // offset 812 - Stored resource amount (float)
    "ResourceLayers",            // offset 816 - Number of resource layers (int)

    // --- Physics ---
    "SpreadingSpeed",            // offset 880 - Spreading speed (float)
    "ExplosionMass",             // offset 884 - Explosion mass, input * 0.001 (float)
    "OnScreen",                  // offset 889 - Stay on screen (bool)
    "Power",                     // offset 828 - Battle power (float, default 1.0)
    "CollisionDamage",           // offset 824 - Collision damage (float, default 1.0)
    "Density",                   // offset 936 - Object density (float, default 1.0)
    "UneffectedByPhysics",       // offset 917 - Unaffected by physics (bool)

    // --- Auto Destruction ---
    "AutoDestructionTime",       // offset 832 - Auto destruction time, input * 0.001 (float)
    "AutoDestructionRandom",     // offset 836 - Auto destruction random variance, input * 0.001 (float)

    // --- Animation ---
    "AnimationLoopType",         // offset 524 - Values: "Stop"=0, "NormalLoop"=2
    "Animate",                   // Values: "OnBuildComplete", "OnFire", or always
    "AnimationTime",             // Animation duration, input ms * 1193.182 (int)
    "AnimationStart",            // Animation start offset, input ms * 1193.182 (int)
    "DisableAnimation",          // offset 865 - Disable animation (bool)

    // --- Auto Rotation ---
    "AutoRotationH",             // offset 768 - Auto rotation heading/yaw (float)
    "AutoRotationP",             // offset 772 - Auto rotation pitch (float)
    "AutoRotationB",             // offset 776 - Auto rotation bank/roll (float)

    // --- Auto Scale ---
    "AutoScaleX",                // offset 780 - Auto scale X (float)
    "AutoScaleY",                // offset 784 - Auto scale Y (float)
    "AutoScaleZ",                // offset 788 - Auto scale Z (float)

    // --- Destruction ---
    "DestructionMethod",         // offset 852 - "None"=0, "AutoExplosion"=10, or object name=64
    "DisableOnDestruction",      // offset 864 - Disable on destroy (bool)
    "DebrisChance",              // offset 868 - Debris spawn chance (float)

    // --- Rendering ---
    "DisableBoundRadius",        // offset 569 bit3 - Disable bounding radius
    "DisableClipping",           // offset 888 - Disable view clipping (bool)
    "Billboard",                 // offset 422 bit1 - Billboard mode, always face camera (bool)
    "FlatRadiusScaling",         // offset 684 - Flat radius scaling (float, default 1.0)

    // --- Morphing ---
    "MorphStepTexture",          // offset 568 bit6 - Morph step texture (bool)
    "MorphStepColor",            // offset 568 bit5 - Morph step color (bool)
    "MorphStepVertex",           // offset 568 bit4 - Morph step vertex (bool)

    // --- Emitter ---
    "UseInternalEmitter",        // offset 872/876 - Internal particle emitter ID (string->int)

    // --- Behavior ---
    "UpdateObject",              // offset 920 - Force object update every frame (bool)
    "MoveWhenParentDies",        // offset 924 - Movement when parent destroyed (float)
    "RandomRotation",            // offset 928 - Random initial rotation (bool)
    "ForceRadius",               // offset 792/796 - Force effect radius (bool/float)
    "NoTolerance",               // offset 741 - No tolerance (bool)

    // --- Projectile/Homing ---
    "HomingStrength",            // offset 840/844 - Homing missile strength (bool/float)
    "DamagePower",               // offset 712 - Pressure wave damage power (float)
    "DamageRadius",              // offset 716 - Pressure wave damage radius (float)

    // --- Object Sounds ---
    "SoundCreated",              // offset 892/896 - Sound when object created
    "SoundLife",                 // offset 900/904 - Sound during object life (looping)
    "SoundDeath",                // offset 908/912 - Sound when object destroyed

    // =========================================================================
    // CGridMemberTemplate (Grid/Building Base - sub_470400)
    // Extends COutforceObject for grid-based structures
    // =========================================================================

    "TileSize",                  // Grid tile size (parsed but not stored directly)
    "InvisibleToPathfinder",     // offset 964 - Pathfinder ignores this object (bool)
    "BuildTime",                 // offset 972 - Construction time (float, default 1.0)
    "Expense",                   // offset 976 - Resource cost to build (float)
    "SplashDamageModifier",      // offset 980 - Splash damage multiplier (float, default 1.0)
    "NoDebris",                  // offset 985 - No debris when destroyed (bool)
    "NoYCollision",              // offset 986 - No Y-axis collision (bool)
    "AbsoluteStill",             // offset 987 - Object never moves (bool)

    // =========================================================================
    // CUnitTemplate (Unit Settings - sub_46C2A0)
    // Extends CGridMemberTemplate for mobile units and buildings
    // =========================================================================

    // --- Build System ---
    "CanBuildUnit",              // Reference to buildable unit template (string, multiple allowed)
    "ConstructedBy",             // Reference to constructor unit template (string)
    "HasBuildCaps",              // offset 1010 - Has build capability (bool)
    "BuildMethod",               // offset 1012 - "Internal"=1, "External"=2
    "BuildPowerAdd",             // offset 1028 - Build power/speed addition (float)
    "BuildListPriority",         // offset 1016 - Priority in build list UI (int)

    // --- Resource Production/Consumption ---
    "Storage",                   // offset 1052 - Resource storage capacity (float)
    "GivesEnergy",               // offset 1120 - Energy production per tick (float)
    "GiveEnergy",                // offset 1120 - Alternative spelling
    "TakesEnergy",               // offset 1124 - Energy consumption per tick (float)
    "TakeEnergy",                // offset 1124 - Alternative spelling
    "GivesResources",            // offset 1132 - Resource production per tick (float)
    "GiveResources",             // offset 1132 - Alternative spelling
    "TakesResources",            // offset 1136 - Resource consumption per tick (float)
    "TakeResources",             // offset 1136 - Alternative spelling

    // --- Harvesting ---
    "CanHarvestType",            // offset 1040 - Harvestable resource type ID (int)
    "HarvestSpeed",              // offset 1044 - Harvest speed (float)
    "MaxResourcesHeld",          // offset 1048 - Max resources unit can carry (float)
    "HasReloadBar",              // offset 1193 - Show reload/progress bar (bool)

    // --- Child Units & Docking ---
    "AddChildUnit",              // offset 1056 - Child unit template to spawn (string)
    "ChildUnitOffsetX",          // offset 1060 - Child unit local X offset (float)
    "ChildUnitOffsetZ",          // offset 1064 - Child unit local Z offset (float)
    "DockPositionX",             // offset 1068 - Dock position X (float)
    "DockPositionZ",             // offset 1072 - Dock position Z (float)
    "DockingAngle",              // offset 1076 - Docking angle (float)

    // --- Combat ---
    "NumWeapons",                // offset 1164 - Number of weapon slots (int)
    "HasKamikazeCaps",           // offset 1216 - Kamikaze/suicide attack capability (bool)
    "SpaceAttack",               // offset 1009 - Can perform space/orbital attack (bool)
    "DontScanForAttack",         // offset 1192 - Don't auto-acquire targets (bool)

    // --- Movement ---
    "Speed",                     // offset 1004 - Movement speed (float)
    "EnergyAcceleration",        // offset 1196 - Energy-based acceleration (float)

    // --- Visibility & Radar ---
    "VisibleRange",              // offset 1176 - Vision/sight range (float), squared stored at 1180
    "HasRadarCaps",              // offset 1033 - Has radar capability (bool)
    "RadarRange",                // offset 1036 - Radar detection range (float)
    "AntiRadar",                 // offset 1036/1201 - Anti-radar/stealth range (float) + flag

    // --- Special: Warp ---
    "HasWarpCaps",               // offset 1140 - Has warp/teleport capability (bool)
    "WarpMaxRange",              // offset 1144 - Maximum warp distance (float)
    "HasAntiWarpCaps",           // offset 1148 - Has anti-warp field (bool)
    "HasWarpNukeCaps",           // offset 1213 - Has warp nuke capability (bool)

    // --- Special: Towing ---
    "HasTowCaps",                // offset 1168 - Can tow other units (bool)
    "TowMaxCableLength",         // offset 1172 - Maximum tow cable length (float)

    // --- Special: Spotting & Repair ---
    "HasSpotCaps",               // offset 1080/1084 - Spotting/scouting capability (bool/float)
    "HasRepairCaps",             // offset 1088 - Repair capability type (int)
    "PowerTransform",            // offset 1092 - Power/energy transform rate (float)

    // --- Special: Other ---
    "HasPhasingCaps",            // offset 1212 - Can phase through objects (bool)
    "LaserFence",                // offset 1202/1204 - Creates laser fence (bool/float range)
    "LaserFenceDamage",          // offset 1208 - Laser fence damage (float, default 1.0)

    // --- Flags ---
    "OnlySinglePlayer",          // offset 1032 - Only available in single player (bool)
    "Friendly",                  // offset 1200 - Friendly to all players (bool)
    "DisableCUnitUpdate",        // offset 1214 - Disable CUnit update logic (bool)

    // --- Unit Sounds ---
    "SoundClicked",              // offset 1224/1228 - Sound when unit selected
    "SoundAttack",               // offset 1232/1236 - Sound when attacking
    "SoundActive",               // offset 1272/1276 - Sound when activated
    "SoundDeactive",             // offset 1264/1268 - Sound when deactivated
    "SoundMove",                 // offset 1240/1244 - Sound when moving
    "SoundCancel",               // offset 1248/1252 - Sound when order cancelled
    "SoundDamage",               // offset 1256/1260 - Sound when taking damage

    // =========================================================================
    // CUnitWeaponTemplate (Weapon Settings - sub_4721C0)
    // Weapon definitions attached to units
    // =========================================================================

    "DefineWeapon",              // offset 1028 - Weapon slot definition ID (int)
    "AngleMovement",             // offset 964 - Turret rotation speed (float, default ~PI)
    "VisibleRangeWeapon",        // offset 968 - Weapon's visible/firing range (float)
    "UpdatesReloadBar",          // offset 1026 - Updates unit's reload bar (bool)
    "NoTargeting",               // offset 972 - Weapon doesn't auto-target (bool)

    "WeaponMethod",              // offset 984 - Weapon firing method:
    //   "Disabled"=0, "ObjectThrower"=1, "Laser"=2,
    //   "DirectHit"=3, "Thunder"=4, "ScaleObject"=5

    // --- Projectile Objects ---
    "ThrowObject",               // offset 988 - Projectile object template (string)
    "ThrowSpeed",                // offset 1008 - Projectile speed (float)
    "ScaleObject",               // offset 988 - Scale effect object (string)
    "FlameObject",               // offset 992 - Muzzle flame effect (string, flag at 1024)
    "LaserObject",               // offset 1004 - Laser beam object (string)
    "HitObject",                 // offset 996 - Impact effect object (string, flag at 1025)
    "TraceObject",               // offset 1000 - Bullet trace/trail object (string, DirectHit only)

    // --- Damage & Timing ---
    "HitDamage",                 // offset 1016 - Damage per hit (float, default 10.0)
    "LaserBurnTime",             // offset 976 - Laser burn duration, input ms * 1193.182 (int)
    "LaserWidth",                // offset 980 - Laser beam width (float)
    "ReloadTime",                // offset 1012 - Time between shots, input ms * 0.001 (float, default 0.25)

    // --- Weapon Flags ---
    "LockedRotation",            // offset 1032 - Turret cannot rotate (bool)
    "ImmediateDestruction",      // offset 1033 - Target destroyed immediately on hit (bool)

    // =========================================================================
    // Runtime/Spawn Settings (sub_4C25D0 console command, sub_44B220 binary)
    // Settings applied when objects are placed in the world
    // =========================================================================

    "InitialRotation",           // offset 796 - Initial rotation in radians (float)
    "InitialScaling",            // offset 800 - Initial scale modifier (float)
    "CustomValueX",              // offset 608+0 - Custom value X for special objects (float)
    "CustomValueY",              // offset 608+4 - Custom value Y for special objects (float)
    "CustomValueZ"               // offset 608+8 - Custom value Z for special objects (float)
};

CustomSettingsWidget::CustomSettingsWidget(QWidget* parent) : QWidget(parent), m_currentObject(nullptr)
{
    setupUI();
}

void CustomSettingsWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel("Custom Settings (Game Logic)", this);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels({"Name", "Value"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    m_table->setColumnWidth(0, 180);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);

    connect(m_table, &QTableWidget::cellChanged, this, &CustomSettingsWidget::onCellChanged);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &CustomSettingsWidget::onSelectionChanged);

    mainLayout->addWidget(m_table);

    QGroupBox* addGroup = new QGroupBox("Add New Setting", this);
    QHBoxLayout* addLayout = new QHBoxLayout(addGroup);

    m_nameCombo = new QComboBox(this);
    m_nameCombo->setEditable(true);
    m_nameCombo->addItems(s_knownSettingNames);
    m_nameCombo->setCurrentText("");
    m_nameCombo->setMinimumWidth(180);
    addLayout->addWidget(new QLabel("Name:", this));
    addLayout->addWidget(m_nameCombo);

    m_valueEdit = new QLineEdit(this);
    m_valueEdit->setPlaceholderText("Enter value...");
    addLayout->addWidget(new QLabel("Value:", this));
    addLayout->addWidget(m_valueEdit, 1);

    m_addButton = new QPushButton("Add", this);
    connect(m_addButton, &QPushButton::clicked, this, &CustomSettingsWidget::onAddSetting);
    addLayout->addWidget(m_addButton);

    mainLayout->addWidget(addGroup);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_removeButton = new QPushButton("Remove Selected", this);
    m_removeButton->setEnabled(false);
    connect(m_removeButton, &QPushButton::clicked, this, &CustomSettingsWidget::onRemoveSetting);
    buttonLayout->addWidget(m_removeButton);

    mainLayout->addLayout(buttonLayout);

    updateButtonStates();
}

void CustomSettingsWidget::setObject(Opf::Object* object)
{
    m_currentObject = object;
    refreshTable();
    updateButtonStates();
}

void CustomSettingsWidget::clear()
{
    m_currentObject = nullptr;
    m_table->setRowCount(0);
    m_nameCombo->setCurrentText("");
    m_valueEdit->clear();
    updateButtonStates();
}

QStringList CustomSettingsWidget::getKnownSettingNames()
{
    return s_knownSettingNames;
}

void CustomSettingsWidget::refreshTable()
{
    m_table->blockSignals(true);
    m_table->setRowCount(0);

    if (!m_currentObject)
    {
        m_table->blockSignals(false);
        return;
    }

    const auto& settings = m_currentObject->customSettings;
    m_table->setRowCount(settings.size());

    for (int i = 0; i < settings.size(); i++)
    {
        QTableWidgetItem* nameItem = new QTableWidgetItem(settings[i].name);
        QTableWidgetItem* valueItem = new QTableWidgetItem(settings[i].value);

        // Highlight important settings with colors
        if (settings[i].name == "CanBuildUnit")
        {
            nameItem->setBackground(QColor(100, 150, 100));   // Green - build capability
            valueItem->setBackground(QColor(100, 150, 100));
        }

        else if (settings[i].name.startsWith("HasWarp") || settings[i].name.startsWith("HasRadar") || settings[i].name.startsWith("HasRepair") || settings[i].name.startsWith("HasTow") || settings[i].name.startsWith("HasBuild") ||
                 settings[i].name.startsWith("HasKamikaze") ||settings[i].name.startsWith("HasPhasing") || settings[i].name.startsWith("HasSpot") || settings[i].name.startsWith("HasAntiWarp"))
        {
            nameItem->setBackground(QColor(100, 100, 180));   // Blue - special abilities
            valueItem->setBackground(QColor(100, 100, 180));
        }

        else if (settings[i].name.startsWith("Weapon") || settings[i].name == "DefineWeapon" || settings[i].name == "HitDamage" || settings[i].name == "ThrowObject" || settings[i].name == "LaserObject")
        {
            nameItem->setBackground(QColor(180, 100, 100));   // Red - weapons
            valueItem->setBackground(QColor(180, 100, 100));
        }

        m_table->setItem(i, 0, nameItem);
        m_table->setItem(i, 1, valueItem);
    }

    m_table->blockSignals(false);
}

void CustomSettingsWidget::updateButtonStates()
{
    bool hasObject = (m_currentObject != nullptr);
    bool hasSelection = !m_table->selectedItems().isEmpty();

    m_addButton->setEnabled(hasObject);
    m_removeButton->setEnabled(hasObject && hasSelection);
    m_nameCombo->setEnabled(hasObject);
    m_valueEdit->setEnabled(hasObject);
}

void CustomSettingsWidget::onAddSetting()
{
    if (!m_currentObject) return;

    QString name = m_nameCombo->currentText().trimmed();
    QString value = m_valueEdit->text().trimmed();

    if (name.isEmpty())
    {
        QMessageBox::warning(this, "Warning", "Please enter a setting name.");
        return;
    }

    m_currentObject->customSettings.append(Opf::CustomSetting(name, value));

    refreshTable();
    m_nameCombo->setCurrentText("");
    m_valueEdit->clear();

    emit settingsModified();
}

void CustomSettingsWidget::onRemoveSetting()
{
    if (!m_currentObject) return;

    int row = m_table->currentRow();
    if (row < 0 || row >= m_currentObject->customSettings.size()) return;

    QString name = m_currentObject->customSettings[row].name;
    QString value = m_currentObject->customSettings[row].value;

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Removal",QString("Remove setting '%1' = '%2'?").arg(name, value),QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    m_currentObject->customSettings.removeAt(row);

    refreshTable();
    emit settingsModified();
}

void CustomSettingsWidget::onCellChanged(int row, int column)
{
    if (!m_currentObject) return;
    if (row < 0 || row >= m_currentObject->customSettings.size()) return;

    QTableWidgetItem* item = m_table->item(row, column);
    if (!item) return;

    QString newValue = item->text();

    if (column == 0)
    {
        m_currentObject->customSettings[row].name = newValue;
    }

    else
    {
        m_currentObject->customSettings[row].value = newValue;
    }

    emit settingsModified();
}

void CustomSettingsWidget::onSelectionChanged()
{
    updateButtonStates();
}

// CanBuildUnitWidget

CanBuildUnitWidget::CanBuildUnitWidget(QWidget* parent) : QWidget(parent), m_currentObject(nullptr)
{
    setupUI();
}

void CanBuildUnitWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel("Units This Object Can Build", this);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    m_unitList = new QListWidget(this);
    m_unitList->setAlternatingRowColors(true);
    connect(m_unitList, &QListWidget::itemSelectionChanged, this, &CanBuildUnitWidget::onSelectionChanged);
    mainLayout->addWidget(m_unitList);

    QHBoxLayout* addLayout = new QHBoxLayout();

    m_unitCombo = new QComboBox(this);
    m_unitCombo->setEditable(true);
    m_unitCombo->setMinimumWidth(200);
    addLayout->addWidget(new QLabel("Unit:", this));
    addLayout->addWidget(m_unitCombo, 1);

    m_addButton = new QPushButton("Add", this);
    connect(m_addButton, &QPushButton::clicked, this, &CanBuildUnitWidget::onAddUnit);
    addLayout->addWidget(m_addButton);

    m_removeButton = new QPushButton("Remove", this);
    m_removeButton->setEnabled(false);
    connect(m_removeButton, &QPushButton::clicked, this, &CanBuildUnitWidget::onRemoveUnit);
    addLayout->addWidget(m_removeButton);

    mainLayout->addLayout(addLayout);

    updateButtonStates();
}

void CanBuildUnitWidget::setObject(Opf::Object* object)
{
    m_currentObject = object;
    refreshList();
    updateButtonStates();
}

void CanBuildUnitWidget::setAvailableUnits(const QStringList& units)
{
    m_availableUnits = units;
    m_unitCombo->clear();
    m_unitCombo->addItems(units);
}

void CanBuildUnitWidget::clear()
{
    m_currentObject = nullptr;
    m_unitList->clear();
    updateButtonStates();
}

void CanBuildUnitWidget::refreshList()
{
    m_unitList->clear();

    if (!m_currentObject) return;

    QStringList buildableUnits = m_currentObject->getCanBuildUnits();
    for (const QString& unit : buildableUnits)
    {
        m_unitList->addItem(unit);
    }
}

void CanBuildUnitWidget::updateButtonStates()
{
    bool hasObject = (m_currentObject != nullptr);
    bool hasSelection = !m_unitList->selectedItems().isEmpty();

    m_addButton->setEnabled(hasObject);
    m_removeButton->setEnabled(hasObject && hasSelection);
    m_unitCombo->setEnabled(hasObject);
}

void CanBuildUnitWidget::onAddUnit()
{
    if (!m_currentObject) return;

    QString unitName = m_unitCombo->currentText().trimmed();
    if (unitName.isEmpty())
    {
        QMessageBox::warning(this, "Warning", "Please enter a unit name.");
        return;
    }

    QStringList existing = m_currentObject->getCanBuildUnits();
    if (existing.contains(unitName))
    {
        QMessageBox::information(this, "Info",QString("Unit '%1' is already in the build list.").arg(unitName));
        return;
    }

    m_currentObject->addCanBuildUnit(unitName);

    refreshList();
    m_unitCombo->setCurrentText("");

    emit settingsModified();
}

void CanBuildUnitWidget::onRemoveUnit()
{
    if (!m_currentObject) return;

    QListWidgetItem* item = m_unitList->currentItem();
    if (!item) return;

    QString unitName = item->text();

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Removal",QString("Remove '%1' from build list?").arg(unitName),QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    m_currentObject->removeCanBuildUnit(unitName);

    refreshList();
    emit settingsModified();
}

void CanBuildUnitWidget::onSelectionChanged()
{
    updateButtonStates();
}
