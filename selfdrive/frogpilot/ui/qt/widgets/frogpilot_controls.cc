#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include "selfdrive/ui/ui.h"

void updateFrogPilotToggles() {
  static Params params_memory{"/dev/shm/params"};
  params_memory.putBool("FrogPilotTogglesUpdated", true);
}

QColor loadThemeColors(const QString &colorKey, bool clearCache) {
  static QJsonObject cachedColorData;

  if (clearCache) {
    cachedColorData = QJsonObject();
    QFile file("../frogpilot/assets/active_theme/colors/colors.json");

    while (!file.exists()) {
      util::sleep_for(100);
    }

    file.open(QIODevice::ReadOnly);

    QByteArray fileData = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(fileData);

    cachedColorData = doc.object();
  }

  QJsonObject colorObj = cachedColorData.value(colorKey).toObject();
  return QColor(
    colorObj.value("red").toInt(0),
    colorObj.value("green").toInt(0),
    colorObj.value("blue").toInt(0),
    colorObj.value("alpha").toInt(255)
  );
}

bool FrogPilotConfirmationDialog::toggle(const QString &prompt_text, const QString &confirm_text, QWidget *parent, const bool isLong) {
  ConfirmationDialog d(prompt_text, confirm_text, tr("Reboot Later"), false, parent, isLong);
  return d.exec();
}

bool FrogPilotConfirmationDialog::toggleAlert(const QString &prompt_text, const QString &button_text, QWidget *parent, const bool isLong) {
  ConfirmationDialog d(prompt_text, button_text, "", false, parent, isLong);
  return d.exec();
}

bool FrogPilotConfirmationDialog::yesorno(const QString &prompt_text, QWidget *parent, const bool isLong) {
  ConfirmationDialog d(prompt_text, tr("Yes"), tr("No"), false, parent, isLong);
  return d.exec();
}
