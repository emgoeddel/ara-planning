#include "ArmControlPanel.h"

ArmControlPanel::ArmControlPanel(Arm& arm, QWidget* parent) :
    QWidget(parent),
    arm(arm)
{
    QGridLayout* layout = new QGridLayout(this);

    for(int i= 0; i < arm.get_num_joints(); i++)
    {
        QLabel* label =  new QLabel(tr("joint"), this);
        QSpinBox* box = new QSpinBox(this);
        box->setRange(arm.get_joint_min(i),
                      arm.get_joint_max(i));
        box->setSingleStep(10);
        layout->addWidget(label, i, 0);
        layout->addWidget(box, i, 1);
        connect(box, SIGNAL(valueChanged(int)), this,
                SLOT(updateArm()));
        jointMap[i] = box;
    }
}

void ArmControlPanel::synchronize()
{
    Arm values = arm;
    for (int i = 0; i < values.get_num_joints(); i++)
    {
        jointMap[i]->setValue(values.get_joint(i));
    }
}

void ArmControlPanel::updateArm()
{
    pose angles;
    for(int i = 0; i < arm.get_num_joints(); i++)
    {
        angles.push_back(float(jointMap[i]->value()));
    }
    arm.set_joints(angles);
    emit(redrawArm());
}