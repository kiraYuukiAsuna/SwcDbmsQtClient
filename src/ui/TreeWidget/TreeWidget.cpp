#include "TreeWidget.h"
#include <QVBoxLayout>

TreeWidget::TreeWidget(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout;
    m_QTreeWidget = new QTreeWidget(this);
    layout->addWidget(m_QTreeWidget);
    setLayout(layout);
}

CustomTreeWidgetItem*
TreeWidget::addTopItem(const std::string&indexString, const std::string&itemName, QIcon icon, QVariant data,
                       int opColumn) {
    if (findItem(indexString)) {
        return nullptr;
    }

    auto* item = new CustomTreeWidgetItem(indexString);
    item->setText(opColumn, QString::fromStdString(itemName));
    item->setIcon(opColumn, icon);
    item->setData(opColumn, Qt::UserRole + 1, data);

    // add to map
    m_IndexMap.insert({indexString, item});

    m_QTreeWidget->addTopLevelItem(item);

    return item;
}

void TreeWidget::removeTopItem(const std::string&indexString, int opColumn) {
    auto* item = findItem(indexString);
    if (item) {
        // remove from map
        m_IndexMap.erase(indexString);
        m_QTreeWidget->removeItemWidget(item, opColumn);
    }
}

bool
TreeWidget::updateTopItem(const std::string&indexString, const std::string&itemName, QIcon icon, QVariant data,
                          int opColumn) {
    auto iter = m_IndexMap.find(indexString);
    if (iter == m_IndexMap.end()) {
        return false;
    }

    iter->second->setText(opColumn, QString::fromStdString(itemName));
    iter->second->setIcon(opColumn, icon);
    iter->second->setData(opColumn, Qt::UserRole + 1, data);
    return true;
}

CustomTreeWidgetItem*
TreeWidget::addItem(const std::string&parentIndexString, const std::string&indexString, const std::string&itemName,
                    QIcon icon, QVariant data, int opColumn) {
    if (findItem(indexString)) {
        return nullptr;
    }

    auto* parentItem = findItem(parentIndexString);
    if (!parentItem) {
        return nullptr;
    }

    auto* item = new CustomTreeWidgetItem(indexString);
    item->setText(opColumn, QString::fromStdString(itemName));
    item->setIcon(opColumn, icon);
    item->setData(opColumn, Qt::UserRole + 1, data);

    // add to map
    m_IndexMap.insert({indexString, item});

    parentItem->addChild(item);

    return item;
}

void TreeWidget::removeItemImpl(CustomTreeWidgetItem* item) {
    int count = item->childCount();
    if (count == 0) //没有子节点，直接删除
    {
        delete item;
        return;
    }

    for (int i = 0; i < count; i++) {
        CustomTreeWidgetItem* childItem = dynamic_cast<CustomTreeWidgetItem *>(item->child(0)); //删除子节点
        removeItemImpl(childItem);
    }
    delete item; //最后将自己删除
}

void TreeWidget::removeItem(const std::string&indexString) {
    auto iter = m_IndexMap.find(indexString);
    if (iter != m_IndexMap.end()) {
        // remove from map
        removeItemImpl(iter->second);
        m_IndexMap.erase(iter);
    }
}

bool TreeWidget::updateItem(const std::string&indexString, const std::string&itemName, int opColumn) {
    auto iter = m_IndexMap.find(indexString);
    if (iter == m_IndexMap.end()) {
        return false;
    }

    iter->second->setText(opColumn, QString::fromStdString(itemName));
    return true;
}

bool TreeWidget::updateItem(const std::string&indexString, QIcon icon, int opColumn) {
    auto iter = m_IndexMap.find(indexString);
    if (iter == m_IndexMap.end()) {
        return false;
    }

    iter->second->setIcon(opColumn, icon);
    return true;
}

bool TreeWidget::updateItem(const std::string&indexString, QVariant data, int opColumn) {
    auto iter = m_IndexMap.find(indexString);
    if (iter == m_IndexMap.end()) {
        return false;
    }

    iter->second->setData(opColumn, Qt::UserRole + 1, data);
    return true;
}

bool TreeWidget::updateItem(const std::string&indexString, const std::string&itemName, QIcon icon, QVariant data,
                            int opColumn) {
    auto iter = m_IndexMap.find(indexString);
    if (iter == m_IndexMap.end()) {
        return false;
    }

    iter->second->setText(opColumn, QString::fromStdString(itemName));
    iter->second->setIcon(opColumn, icon);
    iter->second->setData(opColumn, Qt::UserRole + 1, data);
    return true;
}

CustomTreeWidgetItem* TreeWidget::findItem(const std::string&indexString, int opColumn) {
    auto iter = m_IndexMap.find(indexString);
    if (iter == m_IndexMap.end()) {
        return nullptr;
    }
    return iter->second;
}

void TreeWidget::sort(int opColumn) {
    m_QTreeWidget->sortItems(opColumn, Qt::AscendingOrder);
}

QTreeWidget* TreeWidget::getQTreeWidget() {
    return m_QTreeWidget;
}

std::unordered_map<std::string, CustomTreeWidgetItem *>& TreeWidget::getIndexMap() {
    return m_IndexMap;
}
