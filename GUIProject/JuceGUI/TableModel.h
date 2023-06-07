#pragma once

#include <JuceHeader.h>

#include "NamedVector3.h"
#include "CommonCode.h"

class TableModel : public juce::TableListBoxModel
{
public:
	TableModel();

	int getNumRows() override;
	void paintRowBackground(Graphics&, int rowNumber, int width, int height, bool rowIsSelected) override;
	void paintCell(
		Graphics&,
		int 	rowNumber,
		int 	columnId,
		int 	width,
		int 	height,
		bool 	rowIsSelected
	) override;

	void SetData(const std::vector<commonCode::Block>& data)
	{
		m_Data.clear();
		for (const commonCode::Block& block : data)
			m_Data.push_back(block);
	}

private:
	std::vector<commonCode::Block> m_Data;
};
