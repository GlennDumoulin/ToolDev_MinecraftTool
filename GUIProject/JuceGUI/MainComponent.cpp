#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize(800, 600);

    //Initialize input button
    m_InputBtn.setButtonText("No file selected");
    addAndMakeVisible(m_InputBtn);
    m_InputBtn.onClick = [this]()
    {
        SelectFile();
    };

    //Initialize output button
    m_OutputBtn.setButtonText("No folder selected");
    addAndMakeVisible(m_OutputBtn);
    m_OutputBtn.onClick = [this]()
    {
        SelectFolder();
    };

    //Initialize conversion button
    m_ConversionBtn.setButtonText("Convert JSON to OBJ");
    addAndMakeVisible(m_ConversionBtn);
    m_ConversionBtn.onClick = [this]()
    {
        ConvertFile();
    };
    CheckConversionBtnState();

    //Initialize reset button
    m_ResetBtn.setButtonText("Reset inputs");
    m_ResetBtn.setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(100, 100, 70));
    addAndMakeVisible(m_ResetBtn);
    m_ResetBtn.onClick = [this]()
    {
        ResetInputs();
    };

    //Initialize report type combo box
    addAndMakeVisible(m_ReportType);
    m_ReportType.addItem("None", static_cast<int>(commonCode::ReportStatus::UNDEFINED));
    m_ReportType.addItem("Blocks", static_cast<int>(commonCode::ReportStatus::BLOCKS));
    m_ReportType.addItem("Layers", static_cast<int>(commonCode::ReportStatus::LAYERS));
    m_ReportType.setSelectedId(static_cast<int>(commonCode::ReportStatus::BLOCKS));

    //Initialize output filename text editor
    m_OutputFilename.setJustification(juce::Justification::centred);
    m_OutputFilename.setInputRestrictions(30, "aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ1234567890-_ ");
    addAndMakeVisible(m_OutputFilename);

    //Initialize report data table
    addChildComponent(m_DataTable);
    m_pTableModel = new TableModel();
    m_DataTable.setModel(m_pTableModel);
}

MainComponent::~MainComponent()
{
    if (m_pTableModel)
        delete m_pTableModel;
}


//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    //Initialize texts on screen
    g.setFont(juce::Font(16.0f));
    g.setColour(juce::Colours::white);

    g.drawText("Select input file", juce::Rectangle<int>{ 10, 10, (getWidth() / 2) - 20, 20 }, juce::Justification::centred, true);
    g.drawText("Select output folder", juce::Rectangle<int>{ (getWidth() / 2) + 10, 10, (getWidth() / 2) - 20, 20 }, juce::Justification::centred, true);

    g.drawText("Report type", juce::Rectangle<int>{ 10, 70, 100, 30 }, juce::Justification::centred, true);

    g.drawText("Filename", juce::Rectangle<int>{ (getWidth() / 2) + 10, 70, 80, 30 }, juce::Justification::centred, true);
    g.drawText(m_OutputExtenstion, juce::Rectangle<int>{ getWidth() - 60, 70, 60, 30 }, juce::Justification::centred, true);
   
    g.drawText(m_ConversionMsg, juce::Rectangle<int>{ 10, getHeight() - 30, getWidth() - 20, 20 }, juce::Justification::centred, true);
}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    
    //Initialize bounds for buttons
    m_InputBtn.setBounds(10, 30, (getWidth() / 2) - 20, 30);
    m_OutputBtn.setBounds((getWidth() / 2) + 10, 30, (getWidth() / 2) - 20, 30);
    m_ConversionBtn.setBounds(10, 110, (getWidth() / 2) - 20, 30);
    m_ResetBtn.setBounds((getWidth() / 2) + 10, 110, (getWidth() / 2) - 20, 30);

    //Initialize bounds for combo box
    m_ReportType.setBounds(110, 70, (getWidth() / 2) - 120, 30);

    //Initialize bounds for text editor
    m_OutputFilename.setBounds((getWidth() / 2) + 90, 70, (getWidth() / 2) - 140, 30);

    //Initialize bounds for data table
    m_DataTable.setBounds(10, 150, getWidth() - 20, getHeight() - 190);
}

//==============================================================================
void MainComponent::SelectFile()
{
    using namespace juce;

    //Create file chooser
    m_pFileChooser = std::make_unique<FileChooser>(
        "Please select the json file you want to load...",
        m_InputFile.existsAsFile() ? m_InputFile : File::getSpecialLocation(File::userHomeDirectory),
        "*.json"
    );

    auto folderChooserFlags = FileBrowserComponent::openMode;
    m_pFileChooser->launchAsync(folderChooserFlags,
        [this](const FileChooser& chooser)
        {
            juce::Logger::getCurrentLogger()->writeToLog("JSON file selected");

            //Save the selected file
            m_InputFile = chooser.getResult();
            m_InputBtn.setButtonText(m_InputFile.getFileName());

            CheckConversionBtnState();

            //Set default text for output filename
            m_OutputFilename.setTextToShowWhenEmpty(m_InputFile.getFileNameWithoutExtension(), juce::Colours::white);
            m_OutputFilename.repaint();
        }
    );
}

void MainComponent::SelectFolder()
{
    using namespace juce;

    //Create folder chooser
    m_pFileChooser = std::make_unique<FileChooser>(
        "Please select the output location you want to use...",
        m_OutputFile.exists() ? m_OutputFile : File::getSpecialLocation(File::userHomeDirectory),
        ""
    );

    auto folderChooserFlags = FileBrowserComponent::canSelectDirectories;
    m_pFileChooser->launchAsync(folderChooserFlags,
        [this](const FileChooser& chooser)
        {
            juce::Logger::getCurrentLogger()->writeToLog("Output location selected");

            //Save the selected folder
            m_OutputFile = chooser.getResult();
            m_OutputBtn.setButtonText(m_OutputFile.getFileName());

            CheckConversionBtnState();
        }
    );
}

void MainComponent::CheckConversionBtnState()
{
    //Check if we have everything we need to handle file conversion
    if (m_InputFile.existsAsFile() && m_OutputFile.exists())
    {
        m_ConversionBtn.setEnabled(true);
        m_ConversionBtn.setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(70, 100, 70));
    }
    else
    {
        m_ConversionBtn.setEnabled(false);
        m_ConversionBtn.setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(100, 70, 70));
    }
}

void MainComponent::ConvertFile()
{
    //Create output path
    juce::String outputPath{ m_OutputFile.getFullPathName() };
    outputPath.append("\\", 2);
    if (m_OutputFilename.isEmpty())
    {
        outputPath.append(m_OutputFilename.getTextToShowWhenEmpty(), m_OutputFilename.getTextToShowWhenEmpty().length());
    }
    else
    {
        outputPath.append(m_OutputFilename.getText(), m_OutputFilename.getTotalNumChars());
    }
    outputPath.append(m_OutputExtenstion, m_OutputExtenstion.length());

    //Handle file conversion
    std::vector<commonCode::Block> blocks{};
    std::wstring conversionMsg{ L"" };
    commonCode::ConvertJsonToObj(std::wstring{ m_InputFile.getFullPathName().toWideCharPointer() }, std::wstring{ outputPath.toWideCharPointer() }, blocks, conversionMsg);

    m_ConversionMsg = juce::String{ conversionMsg.c_str() };
    repaint();

    //Handle reporting
    switch (static_cast<commonCode::ReportStatus>(m_ReportType.getSelectedId()))
    {
    case commonCode::ReportStatus::BLOCKS: //Report blocks
    {
        //Set report data
        static_cast<TableModel*>(m_DataTable.getModel())->SetData(blocks);

        //Setup data table columns
        m_DataTable.getHeader().removeAllColumns();
        m_DataTable.getHeader().addColumn("Layer", 1, 200, 100, 300);
        m_DataTable.getHeader().addColumn("X", 2, 50, 20, 60);
        m_DataTable.getHeader().addColumn("Y", 3, 50, 20, 60);
        m_DataTable.getHeader().addColumn("Z", 4, 50, 20, 60);
        m_DataTable.getHeader().addColumn("Is opaque", 5, 100, 50, 100);
        
        //Set data table visible
        m_DataTable.repaint();
        m_DataTable.setVisible(true);

        break;
    }

    case commonCode::ReportStatus::LAYERS: //Report layers
    {
        //Set report data
        std::map<const std::wstring, int> layersData{};
        for (const commonCode::Block& b : blocks)
        {
            const std::wstring layername{ b.layerName };

            if (layersData.find(layername) != layersData.end())
            {
                ++layersData[layername];
            }
            else
            {
                layersData[layername] = 1;
            }
        }
        
        std::vector<commonCode::Block> layers{};
        for (const auto& layerIt : layersData)
        {
            layers.push_back(commonCode::Block{ layerIt.first, false, commonCode::Vector3f{ layerIt.second, 0, 0 } });
        }
        static_cast<TableModel*>(m_DataTable.getModel())->SetData(layers);

        //Setup data table columns
        m_DataTable.getHeader().removeAllColumns();
        m_DataTable.getHeader().addColumn("Layer", 1, 200, 100, 300);
        m_DataTable.getHeader().addColumn("Nr of blocks", 2, 100, 50, 100);

        //Set data table visible
        m_DataTable.repaint();
        m_DataTable.setVisible(true);

        break;
    }

    case commonCode::ReportStatus::UNDEFINED: //Report nothing
    default:
    {
        //Set data table hidden
        m_DataTable.repaint();
        m_DataTable.setVisible(false);

        break;
    }
    }
}

void MainComponent::ResetInputs()
{
    //Reset input file
    if (m_InputFile.existsAsFile())
    {
        m_InputFile = juce::File{};
        m_InputBtn.setButtonText("No file selected");

        //Reset output filename default value
        m_OutputFilename.setTextToShowWhenEmpty("", juce::Colours::white);
        m_OutputFilename.repaint();
    }

    //Reset output folder
    if (m_OutputFile.exists())
    {
        m_OutputFile = juce::File{};
        m_OutputBtn.setButtonText("No folder selected");
    }

    //Reset report type
    m_ReportType.setSelectedId(static_cast<int>(commonCode::ReportStatus::BLOCKS));

    //Reset output filename
    if (!m_OutputFilename.isEmpty())
        m_OutputFilename.clear();

    //Set data table hidden
    m_DataTable.repaint();
    m_DataTable.setVisible(false);

    //Reset conversion button and message
    CheckConversionBtnState();

    m_ConversionMsg = juce::String{};
    repaint();
}
