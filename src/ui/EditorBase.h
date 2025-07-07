#pragma once
#include "src/framework/defination/TypeDef.h"

class EditorBase {
public:
	EditorBase() = default;
	virtual ~EditorBase() = default;

	virtual MetaInfoType getMetaInfoType() { return MetaInfoType::eUnknown; }

	virtual std::string getName() { return ""; }

	virtual std::string getUuid() = 0;

	virtual bool save() = 0;
};
