// Copyright © 2019 Brad Ackerman.
// Licensed under the MIT License (LICENSE.txt in this repository).

#include "psi2nix.hpp"
#include "ui_psi2nix.h"

#include <QDebug>
#include <QSerialPortInfo>

Psi2Nix::Psi2Nix(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Psi2Nix)
{
    ui->setupUi(this);
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    auto comboBox = ui->serialPort;
    for (auto port : ports) {
        QVariant metadata(port.portName());
        QString descr = port.portName();
        descr.append(" (");
        descr.append(port.description());
        descr.append(")");
        comboBox->addItem(descr, metadata);
    }
    if (ports.length() == 0) {
        comboBox->setEnabled(false);
    }

}

Psi2Nix::~Psi2Nix()
{
    delete ui;
}

void Psi2Nix::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void Psi2Nix::on_action_Quit_triggered()
{
    // Quit menu item selected.
    QCoreApplication::quit();
}

void Psi2Nix::on_serialPort_currentIndexChanged(int index)
{
    if (index >= 0) {
        qDebug() << "Selected port: "
                 << ui->serialPort->itemData(index).toString();
    }
}
