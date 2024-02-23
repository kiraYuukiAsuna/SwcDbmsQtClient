#pragma once

#include <memory>
#include "NodeDelegateModelRegistry.hpp"
#include "SnapshotModel.h"
#include "IncrementOperationModel.h"

static std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registerDataModels() {
    auto ret = std::make_shared<QtNodes::NodeDelegateModelRegistry>();

    ret->registerModel<SnapshotDelegateModel>("SwcDbmsClientNodeModel");
    ret->registerModel<IncrementOperationDelegateModel>("SwcDbmsClientNodeModel");

    return ret;
}
