#pragma once

#include <QTreeWidget>
#include <filesystem>
#include <unordered_map>

class CustomTreeWidgetItem : public QTreeWidgetItem {
public:
	CustomTreeWidgetItem(const std::string& indexString,
						 QTreeWidget* tree = nullptr)
		: m_IndexString(indexString), QTreeWidgetItem(tree) {}

	bool operator<(const QTreeWidgetItem& other) const {
		bool thisIsFolder = std::filesystem::is_directory(
			std::filesystem::u8path(this->m_IndexString));

		auto otherItem = dynamic_cast<const CustomTreeWidgetItem*>(&other);
		bool otherIsFolder = std::filesystem::is_directory(
			std::filesystem::u8path(otherItem->m_IndexString));

		if (thisIsFolder != otherIsFolder) {
			// 如果一个是文件夹，另一个是文件，那么文件夹应该在前面
			return thisIsFolder;
		}

		// 否则，按照字母顺序排序
		return text(0) < other.text(0);
	}

	std::string m_IndexString;
};

class TreeWidget : public QWidget {
	Q_OBJECT

public:
	TreeWidget(QWidget* parent);

	CustomTreeWidgetItem* addTopItem(const std::string& indexString,
									 const std::string& itemName, QIcon icon,
									 QVariant data, int opColumn = 0);

	void removeTopItem(const std::string& indexString, int opColumn = 0);

	bool updateTopItem(const std::string& indexString,
					   const std::string& itemName, QIcon icon, QVariant data,
					   int opColumn = 0);

	CustomTreeWidgetItem* addItem(const std::string& parentIndexString,
								  const std::string& indexString,
								  const std::string& itemName, QIcon icon,
								  QVariant data, int opColumn = 0);

	void removeItem(const std::string& indexString);

	bool updateItem(const std::string& indexString, const std::string& itemName,
					int opColumn = 0);

	bool updateItem(const std::string& indexString, QIcon icon,
					int opColumn = 0);

	bool updateItem(const std::string& indexString, QVariant data,
					int opColumn = 0);

	bool updateItem(const std::string& indexString, const std::string& itemName,
					QIcon icon, QVariant data, int opColumn = 0);

	CustomTreeWidgetItem* findItem(const std::string& indexString,
								   int opColumn = 0);

	void sort(int opColumn);

	QTreeWidget* getQTreeWidget();

	std::unordered_map<std::string, CustomTreeWidgetItem*>& getIndexMap();

private:
	QTreeWidget* m_QTreeWidget;
	std::unordered_map<std::string, CustomTreeWidgetItem*> m_IndexMap;

	void removeItemImpl(CustomTreeWidgetItem* item);
};
