#include "TableModel.h"

TableModel::TableModel()
{
	m_Vectors.push_back({ "vector1", 1, 2, 3 });
	m_Vectors.push_back({ "vector2", 4, 5, 6 });
}

int TableModel::getNumRows()
{
	return static_cast<int>(m_Vectors.size());
}

void TableModel::paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
	if (rowIsSelected)
	{
		g.setColour(Colour{ 210, 210, 210 });
		g.fillRoundedRectangle(0.f, 0.f, static_cast<float>(width), static_cast<float>(height), 3.f);
		g.drawRoundedRectangle(0.f, 0.f, static_cast<float>(width), static_cast<float>(height), 3.f, 2.f);
	}
	else
	{
		g.setColour(Colour{ 255, 128, 64 });
		g.drawRoundedRectangle(0.f, 0.f, static_cast<float>(width), static_cast<float>(height), 3.f, 2.f);
	}
}

void TableModel::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
	if (rowNumber < m_Vectors.size())
	{
		g.setColour(rowIsSelected ? Colour{ 64, 64, 64 } : Colour{ 243, 243, 243 });

		NamedVector3& vector = m_Vectors[rowNumber];
		switch (columnId)
		{
		case 1:
		{
			g.drawText(vector.name, 40, 0, width-10, height, Justification::centredLeft);
			break;
		}

		case 2:
		{
			g.drawText(String{ vector.x }, 0, 0, width, height, Justification::centredLeft);
			break;
		}

		case 3:
		{
			g.drawText(String{ vector.y }, 0, 0, width, height, Justification::centredLeft);
			break;
		}

		case 4:
		{
			g.drawText(String{ vector.z }, 0, 0, width, height, Justification::centredLeft);
			break;
		}

		/*case 5:
		{
			g.drawText(String{ vector.isOpaque ? "true" : "false" }, 0, 0, width, height, Justification::centredLeft);
			break;
		}*/
		}
	}
}
