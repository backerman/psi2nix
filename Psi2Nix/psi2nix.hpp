// Copyright © 2019 Brad Ackerman.
// Licensed under the MIT License (LICENSE.txt in this repository).

#pragma once

#include <QMainWindow>

namespace Ui {
class Psi2Nix;
}

class Psi2Nix : public QMainWindow
{
    Q_OBJECT

public:
    explicit Psi2Nix(QWidget *parent = nullptr);
    ~Psi2Nix();

protected:
    void changeEvent(QEvent *e);

private slots:
    void on_action_Quit_triggered();
    void on_serialPort_currentIndexChanged(int index);

private:
    Ui::Psi2Nix *ui;
};
