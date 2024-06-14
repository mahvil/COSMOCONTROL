#include "identify_skin_type.h"
#include <QDialog>
#include <QFormLayout>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <vector>
#include <QStringList>

using namespace std;

void identifySkinType(QWidget *parent) {
    QDialog dialog(parent);
    dialog.setWindowTitle(QObject::tr("Identify Skin Type"));
    QFormLayout form(&dialog);

    // Set the background color to light red
    dialog.setStyleSheet("background-color: #FFCDD2;");

    QStringList questions = {
        "Do you notice that your skin tends to appear shiny or greasy, especially in T-Zone area (forehead, nose, and chin)?",
        "Do your skin feel slick or oily to the touch, even shortly after washing your face?",
        "Have you experienced frequent breakouts, particularly in areas where your skin is oilier?",
        "Does your skin often feel tight or rough, especially after washing your face or showering?",
        "Do you experience flakiness or noticeable dry patches on your skin, particularly on your cheeks or forehead?",
        "Is your skin prone to sensitivity or irritation, and does it easily become red or inflamed?",
        "Do you experience oiliness in specific areas of your face, such as the T-zone (forehead, nose, and chin), while other areas feel dry or normal?",
        "Does your skin tend to break out or develop blackheads in the oilier areas, while the drier areas may have a tendency to feel tight or show signs of flakiness?",
        "Have you noticed that certain skincare products work well on certain parts of your face, while others may exacerbate oiliness or dryness in different areas?",
        "Do you frequently experience skin reactions such as redness, itching, burning, or stinging when using certain skincare products or cosmetics?",
        "Does your skin tend to become easily irritated by environmental factors such as sun exposure, wind, or extreme temperatures?",
        "Have you noticed that your skin reacts negatively to certain fabrics, detergents, or fragrances?"
    };

    vector<QCheckBox*> checkBoxes;
    for (const QString& question : questions) {
        QHBoxLayout* layout = new QHBoxLayout;
        QCheckBox* yesBox = new QCheckBox("Yes");
        QCheckBox* noBox = new QCheckBox("No");
        layout->addWidget(yesBox);
        layout->addWidget(noBox);
        checkBoxes.push_back(yesBox);
        checkBoxes.push_back(noBox);
        form.addRow(new QLabel(question), layout);
    }

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        int countO = 0, countD = 0, countN = 0, countS = 0;

        for (size_t i = 0; i < checkBoxes.size(); i += 2) {
            if (checkBoxes[i]->isChecked()) {
                if (i < 3 * 2) countO++;
                else if (i < 6 * 2) countD++;
                else if (i < 9 * 2) countN++;
                else countS++;
            }
        }

        QString result;
        if (countO > countD && countO > countN && countO > countS) {
            result = "You have Oily Skin.";
        } else if (countD > countO && countD > countN && countD > countS) {
            result = "You have Dry Skin.";
        } else if (countN > countO && countN > countD && countN > countS) {
            result = "You have Combination Skin.";
        } else if (countS > countO && countS > countD && countS > countN) {
            result = "You have Sensitive Skin.";
        } else {
            result = "You have a mix of skin types. Please consult a dermatologist for a more accurate assessment.";
        }

        QMessageBox::information(parent, QObject::tr("Identify Skin Type"), result);
    }
}
