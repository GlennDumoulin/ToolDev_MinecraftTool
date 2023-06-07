#pragma once

#include <JuceHeader.h>
#include "TableModel.h"

#include "CommonCode.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::Component
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    void SelectFile();
    void SelectFolder();
    void CheckConversionBtnState();
    void ConvertFile();
    void ResetInputs();

private:
    //==============================================================================
    // Your private member variables go here...
    juce::TextButton m_InputBtn;
    juce::TextButton m_OutputBtn;
    juce::TextButton m_ConversionBtn;
    juce::TextButton m_ResetBtn;

    std::unique_ptr<juce::FileChooser> m_pFileChooser;
    juce::File m_InputFile;
    juce::File m_OutputFile;

    juce::ComboBox m_ReportType;

    juce::TextEditor m_OutputFilename;
    const juce::String m_OutputExtenstion{ ".obj" };

    juce::String m_ConversionMsg;

    juce::TableListBox m_DataTable;
    TableModel* m_pTableModel{ nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
