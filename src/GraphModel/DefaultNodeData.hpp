#pragma once

#include <QtNodes/NodeData>

class DefaultNodeData : public QtNodes::NodeData {
public:
    DefaultNodeData()
            : _number(0.0)
    {}

    DefaultNodeData(double const number)
            : _number(number)
    {}

    QtNodes::NodeDataType type() const override { return {"decimal", "Decimal"}; }

    double number() const { return _number; }

    QString numberAsText() const { return QString::number(_number, 'f'); }

private:
    double _number;
};
