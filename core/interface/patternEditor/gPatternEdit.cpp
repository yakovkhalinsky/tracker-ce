#include "config.h"
#include "core/interfacePopups.h"
#include "fileManager.h"
#include "mtAudioExport.h"
#include "mtGenerators.h"
#include "mtStructs.h"
#include "patternEditor/patternEditor.h"
#include "performanceMode/performanceMode.h"
#include "scales.h"

#ifdef GUI_HANDLE_SYNTH
#include "Synth.h"
#endif

static const uint16_t framesPlaces[8][4] = {
    { 0 + 1, 423, 800 / 8 - 1, 55 },
    { (800 / 8) * 1 + 1, 423, 800 / 8 - 1, 55 },
    { (800 / 8) * 2 + 1, 423, 800 / 8 - 1, 55 },
    { (800 / 8) * 3 + 1, 423, 800 / 8 - 1, 55 },
    { (800 / 8) * 4 + 1, 423, 800 / 8 - 1, 55 },
    { (800 / 8) * 5 + 1, 423, 800 / 8 - 1, 55 },
    { (800 / 8) * 6 + 1, 423, 800 / 8 - 1, 55 },
    { (800 / 8) * 7 + 1, 423, 800 / 8 - 1, 55 },
};

static const uint16_t framesPopupPlaces[9][4] = {
    { 0 + 1, 58, 800 / 8 - 3, 364 },
    { (800 / 8) * 1 + 1, 58, 800 / 8 - 3, 364 },
    { (800 / 8) * 2 + 1, 58, 800 / 8 - 3, 364 },
    { (800 / 8) * 3 + 1, 58, 800 / 8 - 3, 364 },
    { (800 / 8) * 4 + 1, 58, 800 / 8 - 3, 364 },
    { (800 / 8) * 5 + 1, 58, 800 / 8 - 3, 364 },
    { (800 / 8) * 6 + 1, 58, 800 / 8 - 3, 364 },
    { (800 / 8) * 7 + 1, 58, 800 / 8 - 3, 364 },
    { (800 / 8) * 1 + 1, 58, 800 / 4 - 3, 364 },
};

static const uint8_t fillTypeListCount = 8;
const char fillTypeListLabels[fillTypeListCount][11] = {
    "Constant", "From-To", "Random", "Walk", "Turing", "Markov", "Prob", "Ratchet",
};
const char* ptrfillTypeListNames[fillTypeListCount] = {
    &fillTypeListLabels[0][0],
    &fillTypeListLabels[1][0],
    &fillTypeListLabels[2][0],
    &fillTypeListLabels[3][0],
    &fillTypeListLabels[4][0],
    &fillTypeListLabels[5][0],
    &fillTypeListLabels[6][0],
    &fillTypeListLabels[7][0],
};
// Names in the order valid for the column currently shown in the popup.
static const char* ptrFillTypeNamesForColumn[fillTypeListCount];
//------------------------------------------------------------
static const uint8_t fillFxSlotCount = 2;
const char fillFxSlotLabels[fillFxSlotCount][5] = {
    "FX1",
    "FX2",
};
const char* ptrfillFxSlotNames[fillFxSlotCount] = {
    &fillFxSlotLabels[0][0],
    &fillFxSlotLabels[1][0],
};
//------------------------------------------------------------
static const uint8_t fillScaleFilterCount = 2;
const char fillScaleFilterLabels[fillScaleFilterCount][5] = {
    "No",
    "Yes",
};
const char* ptrfillScaleFilterNames[fillScaleFilterCount] = {
    &fillScaleFilterLabels[0][0],
    &fillScaleFilterLabels[1][0],
};
//------------------------------------------------------------
static const uint8_t fillStepCount = 7;
const char fillStepLabels[fillStepCount][11] = {
    "Note", "No Note", "Fx", "No Fx", "Random", "Each", "Euclidean",
};

const char* ptrfillStepNames[fillStepCount] = {
    &fillStepLabels[0][0], &fillStepLabels[1][0], &fillStepLabels[2][0], &fillStepLabels[3][0],
    &fillStepLabels[4][0], &fillStepLabels[5][0], &fillStepLabels[6][0],
};
//------------------------------------------------------------

void cPatternEditor::initDisplayControls()
{
    strControlProperties prop;
    // ramka
    frameData.placesCount = 8;
    frameData.startPlace = 0;
    frameData.places[0] = &framesPlaces[0][0];
    frameData.places[1] = &framesPlaces[1][0];
    frameData.places[2] = &framesPlaces[2][0];
    frameData.places[3] = &framesPlaces[3][0];
    frameData.places[4] = &framesPlaces[4][0];
    frameData.places[5] = &framesPlaces[5][0];
    frameData.places[6] = &framesPlaces[6][0];
    frameData.places[7] = &framesPlaces[7][0];
    prop.x = 0;
    prop.y = 0;
    prop.w = 0;
    prop.h = 0;
    prop.style = 0;
    prop.value = 0;
    prop.data = &frameData;
    frameControl = display.createControl<cFrame>(&prop);

    strControlProperties prop2;

    for (uint8_t i = 0; i < 8; i++)
    {
        prop2.text = (char*)"";
        prop2.colors = display.getTheme().activeLabelsColors;

        prop2.style = (controlStyleCenterX | controlStyleFont3 | controlStyleCenterY);
        prop2.x = (800 / 8) * i + (800 / 16);
        prop2.w = 800 / 8 - 6;
        prop2.y = 424;
        prop2.h = 55;

        label[i] = display.createControl<cLabel>(&prop2);
    }

    prop2.text = nullptr;
    prop2.colors = nullptr;
    prop2.value = 255;
    prop2.style = controlStyleNoTransparency | controlStyleShow;
    prop2.x = 0;
    prop2.w = 800;
    prop2.y = 425;
    prop2.h = 54;

    bgLabel = display.createControl<cBgLabel>(&prop2);

    //=====================================================================================================
    // POPUP
    //=====================================================================================================
    labelDataRed.line.color = display.getTheme().red;
    labelDataRed.line.x1 = -40;
    labelDataRed.line.y1 = 50;
    labelDataRed.line.x2 = 40;
    labelDataRed.line.y2 = 50;

    labelDataGreen.line.color = display.getTheme().green;
    labelDataGreen.line.x1 = -40;
    labelDataGreen.line.y1 = 50;
    labelDataGreen.line.x2 = 40;
    labelDataGreen.line.y2 = 50;

    labelDataMore.dotIndicator.count = 3;
    labelDataMore.dotIndicator.current = 0;

    // lista 1
    prop.x = (800 / 8) * (1) + 1;
    prop.y = 58;
    prop.w = 800 / 8 - 3;
    prop.h = 140 + 8 * 28;
    prop.colors = nullptr;
    prop.style = controlStyleBackground |
                 (display.getTheme().bottomScreenShadow > 0 ? controlStyleBottomShadow : 0);
    prop.data = &fillTypeList;
    param1PopupListControl = display.createControl<cList>(&prop);

    // lista 2
    prop.x = (800 / 8) * (4) + 1;
    prop.data = &fillScaleFilterList;
    param2PopupListControl = display.createControl<cList>(&prop);

    // lista 3
    prop.x = (800 / 8) * (0) + 1;
    prop.data = &fillStepList;
    param3PopupListControl = display.createControl<cList>(&prop);

    // lista instrumentów
    prop.x = (800 / 8) * (4) + 1;
    prop.data = &fillInstrumentsFromList;
    intstrumentsFromPopupListControl = display.createControl<cList>(&prop);
    prop.x = (800 / 8) * (5) + 1;
    prop.data = &fillInstrumentsToList;
    intstrumentsToPopupListControl = display.createControl<cList>(&prop);

    // bar val1
    prop.data = nullptr;
    prop.x = (800 / 8) * (2) + 1;
    prop.style = controlStyleValue_0_100 | controlStyleBackground |
                 (display.getTheme().barShadow > 0 ? controlStyleGradient : 0) |
                 (display.getTheme().bottomScreenShadow > 0 ? controlStyleBottomShadow : 0);
    val1PopupBar = display.createControl<cBar>(&prop);

    // bar val2
    prop.x = (800 / 8) * (3) + 1;
    val2PopupBar = display.createControl<cBar>(&prop);

    // bar val3
    prop.x = (800 / 8) * (1) + 1;
    val3PopupBar = display.createControl<cBar>(&prop);

    prop.x = (800 / 8) * (6) + 1;
    prop.value = -1;
    valCancelEmptyPopupBar = display.createControl<cBar>(&prop);

    prop.x = (800 / 8) * (7) + 1;
    valFillEmptyPopupBar = display.createControl<cBar>(&prop);

    // label tytulowy popupu
    prop.value = 0;
    prop.style = (controlStyleCenterY | controlStyleFont4 | controlStyleBackground);
    prop.colors = display.getTheme().topBarLabelColors;
    prop.x = 0;
    prop.y = 28;
    prop.w = 799;
    prop.h = 27;
    patternPopupTitleLabel = display.createControl<cLabel>(&prop);

    // label tla
    prop.style = (controlStyleBackground | controlStyleCenterX | controlStyleNoTransparency);
    prop.colors = display.getTheme().patternFillBgLabelColors;
    prop.x = 400;
    prop.y = 28 + 4;
    prop.w = 800;
    prop.h = 193 + 7 * 28;
    patternPopupLabel = display.createControl<cLabel>(&prop);

    //=====================================================================================================
    //=====================================================================================================
    //=====================================================================================================

    uint32_t commonStyle =
      controlStyleFont4 |
      (mtConfig.general.numberingMode == NumberingMode::kHexadecimal ? controlStyleHexNumbers : 0) |
      (mtConfig.general.numberingMode == NumberingMode::kDecimalWithStepsFrom1
         ? controlStyleStartNumbersFrom1
         : 0) |
      (display.getTheme().bottomScreenShadow > 0 ? controlStyleBottomShadow : 0) |
      (display.getTheme().patternEditorPlayheadShadow > 0 ? controlStyleGradient : 0);

    prop.style = commonStyle;
    prop.x = 0;
    prop.y = mtConfig.general.patternInfo != 0 ? 0 : 28;
    prop.w = 800;
    prop.h = mtConfig.general.patternInfo != 0 ? 423 : 395;
    prop.colors = display.getTheme().patternTrackerColors;
    prop.data = &trackerPattern;
    prop.value = patternViewMode;
    verticalPatternControl = display.createControl<cTracker>(&prop);

    //	prop.style =  (mtConfig.general.hexMode ? controlStyleHexNumbers : 0);
    ////	prop.x = 0;
    ////	prop.y =  mtConfig.general.patternInfo != 1 ? 0 : 28;
    ////	prop.w = 800;
    ////	prop.h = mtConfig.general.patternInfo != 1 ? 423 : 395;
    //	prop.colors = (uint32_t*)patternTrackerColors;
    //	prop.data = &trackerPattern;
    //	prop.value = patternViewMode;
    //	arrangementControl = display.createControl<cArrangement>(&prop);

    prop.style = commonStyle;
    //	prop.x = 0;
    //	prop.y =  mtConfig.general.patternInfo != 1 ? 0 : 28;
    //	prop.w = 800;
    //	prop.h = mtConfig.general.patternInfo != 1 ? 423 : 395;
    prop.colors = display.getTheme().patternTrackerColors;
    prop.data = &trackerPattern;
    prop.value = patternViewMode;
    horizontalPatternControl = display.createControl<cHorizontalTracker>(&prop);

    switch (mtConfig.general.patternArrangement)
    {
        case PatternArrangement::kHorizontal:
        {
            patternControl = horizontalPatternControl;
            if (trackerPattern.middleVisibleStep < 5)
            {
                trackerPattern.middleVisibleStep = 5;
            }
            break;
        }
        // case 2: patternControl = arrangementControl; break;
        default: patternControl = verticalPatternControl; break;
    }
}

void cPatternEditor::showDefaultScreen()
{
    showTitleBar();

    showPattern(1);
    showLength(1);
    showStep();

    frameData.places[0] = &framesPlaces[0][0];
    frameData.places[1] = &framesPlaces[1][0];
    frameData.places[2] = &framesPlaces[2][0];
    frameData.places[3] = &framesPlaces[3][0];
    frameData.places[4] = &framesPlaces[4][0];
    frameData.places[5] = &framesPlaces[5][0];
    frameData.places[6] = &framesPlaces[6][0];
    frameData.places[7] = &framesPlaces[7][0];

    activateLabelsBorder();

    display.setControlShow(bgLabel);
    display.refreshControl(bgLabel);

    display.setControlShow(patternControl);
    display.refreshControl(patternControl);

    mtPopups.hideInfoPopup();

    display.synchronizeRefresh();
}

void cPatternEditor::showEditModeLabels()
{
    display.setControlColors(label[3], display.getTheme().activeLabelsColors);
    display.setControlColors(label[4], display.getTheme().activeLabelsColors);
    display.setControlColors(label[5], display.getTheme().activeLabelsColors);
    display.setControlColors(label[6], display.getTheme().activeButtonLabelsColors);
    display.setControlColors(label[7], display.getTheme().activeLabelsColors);

    display.refreshControl(label[3]);
    display.refreshControl(label[4]);
    display.refreshControl(label[5]);
    display.refreshControl(label[6]);
    display.refreshControl(label[7]);

    display.synchronizeRefresh();
}

void cPatternEditor::hideEditModeLabels()
{
    display.setControlColors(label[3], display.getTheme().inactiveLabelsColors);
    //	display.setControlColors(label[4], display.getTheme().inactiveLabelsColors);
    display.setControlColors(label[5], display.getTheme().inactiveLabelsColors);
    display.setControlColors(label[6], display.getTheme().inactiveButtonLabelsColors);
    display.setControlColors(label[7], display.getTheme().inactiveLabelsColors);

    display.refreshControl(label[3]);
    display.refreshControl(label[4]);
    display.refreshControl(label[5]);
    display.refreshControl(label[6]);
    display.refreshControl(label[7]);

    display.synchronizeRefresh();
}

static elapsedMillis lastShownPattern_blinkTimer;

void cPatternEditor::showPattern(uint8_t forceShow, uint8_t shiftPressed)
{
    uint16_t patternToShow = sequencer.isPerformanceMode() ? 4444 : mtProject.values.actualPattern;
    uint16_t nextSequential = sequencer.sequencialSwitch_GetNext();

    if (lastShownPatternOnTitleBar != patternToShow || forceShow)
    {
        lastShownPatternOnTitleBar = patternToShow;
        showTitleBarPattern();
    }

    if (fillState)
        return;
    if (shiftPressed)
        return;
    if (actualActionButtonsPage != 0)
        return;

    patternToShow = mtProject.values.actualPattern;

    // blink czy nie?
    if (nextSequential > 0)
    {
        patternToShow = nextSequential;

        if (lastShownPattern_blinkState == 0)
        {
            // start blinking, it will blink between states 1 and 2
            lastShownPattern_blinkState = 1;
        }
    }
    else
    {
        lastShownPattern_blinkState = 0;
    }

    // refresh value
    if (lastShownPattern != patternToShow || forceShow)
    {
        lastShownPattern = patternToShow;
        const char* numberFormat =
          mtConfig.general.numberingMode == NumberingMode::kHexadecimal ? "%02X" : "%d";

        snprintf(cPattern, 5, numberFormat, patternToShow);
        display.setControlText2(label[0], cPattern);
        display.refreshControl(label[0]);
    }

    // refresh colors
    if (lastShownPattern_blinkTimer >= 200)
    {
        lastShownPattern_blinkTimer = 0;

        if (lastShownPattern_blinkState == 1)
        {
            lastShownPattern_blinkState = 2;
            display.setControlColors(label[0], display.getTheme().redBlinkLabelColors);
        }
        else if (lastShownPattern_blinkState == 2)
        {
            lastShownPattern_blinkState = 1;
            display.setControlColors(label[0], display.getTheme().activeLabelsColors);
        }
        else
        {
            display.setControlColors(label[0], display.getTheme().activeLabelsColors);
        }
        display.refreshControl(label[0]);
    }
}

void cPatternEditor::showLength(uint8_t forceShow)
{
    if (fillState)
        return;
    if (actualActionButtonsPage != 0)
        return;

    const char* numberFormat =
      mtConfig.general.numberingMode == NumberingMode::kHexadecimal ? "%02X" : "%d";
    uint8_t localLength = sequencer.getPatternToUI()->track[0].length + 1;

    if (lastLength != localLength || forceShow)
    {
        lastLength = localLength;
        sprintf(cLength, numberFormat, localLength);
        display.setControlText2(label[1], cLength);
        display.refreshControl(label[1]);
    }
}

void cPatternEditor::showStep()
{
    if (actualActionButtonsPage != 0)
        return;

    sprintf(cStep, "%d", mtProject.values.patternEditStep);

    display.setControlText2(label[2], cStep);
    display.refreshControl(label[2]);
}

void cPatternEditor::refreshPatternParams()
{
    showPattern();
    showLength();
}

//##############################################################################################
//###############################   GENERATIVE FILL HELPERS    #################################
//##############################################################################################

bool cPatternEditor::fillTypeHasAlt(int16_t type) const
{
    return type == fillTypeWalk || type == fillTypeTuring || type == fillTypeMarkov;
}

bool cPatternEditor::fillStepUsesDensity(int16_t fillStep) const
{
    return fillStep >= Sequencer::fillStepRandom;
}

bool cPatternEditor::fillAltIsRotate(const strFill& fill) const
{
    return fillAltActive == fillAltRotate && fill.fillStep == Sequencer::fillStepEuclidean;
}

bool cPatternEditor::fillAltIsTypeParam(const strFill& fill) const
{
    if (!fillTypeHasAlt(fill.type))
        return false;

    // Without a density selector there is nothing to toggle to, so the type
    // parameter is always the value shown in this slot.
    return !fillStepUsesDensity(fill.fillStep) || fillAltActive == fillAltTypeParam;
}

bool cPatternEditor::fillAltSlotActive(const strFill& fill) const
{
    if (!fillStepUsesDensity(fill.fillStep))
        return fillTypeHasAlt(fill.type);
    return fillAltIsRotate(fill) || fillAltIsTypeParam(fill);
}

int16_t cPatternEditor::fillRotateMax() const
{
    // Use the editor's live selection: sequencer.selection is only pushed to
    // the sequencer when the fill is applied.
    int length = 1;
    if (trackerPattern.selectState == 2)
    {
        if (trackerPattern.selectColumn)
        {
            length = sequencer.getPatternLength();
        }
        else
        {
            int first = trackerPattern.selectStartStep;
            int last = trackerPattern.selectEndStep;
            if (first > last)
            {
                const int swap = first;
                first = last;
                last = swap;
            }
            length = last - first + 1;
        }
    }

    if (length < 1)
        length = 1;
    return (int16_t)(length - 1);
}

int16_t cPatternEditor::fillDensityMax(int16_t fillStep) const
{
    if (fillStep == Sequencer::fillStepEuclidean)
        return (int16_t)clamp<int>(fillRotateMax() + 1, 1, Sequencer::MAX_FILL_DENSITY);
    return Sequencer::MAX_FILL_DENSITY;
}

int16_t* cPatternEditor::fillAltValuePtr(strFill& fill, int16_t& min, int16_t& max) const
{
    if (fillAltIsRotate(fill))
    {
        min = 0;
        max = fillRotateMax();
        return &fill.rotate;
    }

    const int16_t type = fill.type;
    min = fillAltMin(type);
    max = fillAltMax(type);
    return &fill.genParam[type - fillTypeWalk][3];
}

void cPatternEditor::toggleFillAlt(const strFill& fill)
{
    if (!fillStepUsesDensity(fill.fillStep))
        return;

    // Cycle through Density/Events, Euclidean Rotate and the type's alternate
    // parameter, skipping the modes that are not available for this
    // combination.  The second column always shows the selected mode's name.
    for (uint8_t i = 1; i <= 3; i++)
    {
        const uint8_t mode = (uint8_t)((fillAltActive + i) % 3);
        if (mode == fillAltDensity ||
            (mode == fillAltRotate && fill.fillStep == Sequencer::fillStepEuclidean) ||
            (mode == fillAltTypeParam && fillTypeHasAlt(fill.type)))
        {
            fillAltActive = mode;
            return;
        }
    }
}

int16_t cPatternEditor::fillAltMin(int16_t type) const
{
    return type == fillTypeWalk ? 0 : 1;
}

int16_t cPatternEditor::fillAltMax(int16_t type) const
{
    return type == fillTypeWalk ? 24 : 48;
}

const char* cPatternEditor::fillAltLabel(int16_t type) const
{
    return type == fillTypeWalk ? "Max jump" : "Range";
}

const uint8_t* cPatternEditor::getFillTypesForColumn(uint8_t& count) const
{
    static const uint8_t noteTypes[] = { fillTypeConstant, fillTypeFromTo, fillTypeRandom,
                                         fillTypeWalk,     fillTypeTuring, fillTypeMarkov };
    static const uint8_t instrTypes[] = { fillTypeConstant, fillTypeFromTo, fillTypeRandom };
    static const uint8_t fxTypes[] = { fillTypeConstant, fillTypeFromTo, fillTypeRandom,
                                       fillTypeProb,     fillTypeRatchet };

    if (editParam == EditNote)
    {
        count = sizeof(noteTypes) / sizeof(noteTypes[0]);
        return noteTypes;
    }
    if (editParam == EditInstr || isMicVolView)
    {
        count = sizeof(instrTypes) / sizeof(instrTypes[0]);
        return instrTypes;
    }
    count = sizeof(fxTypes) / sizeof(fxTypes[0]);
    return fxTypes;
}

uint8_t cPatternEditor::fillTypeToDisplayIndex(int16_t type) const
{
    uint8_t count = 0;
    const uint8_t* types = getFillTypesForColumn(count);
    for (uint8_t i = 0; i < count; i++)
    {
        if (types[i] == type)
            return i;
    }
    return 0;
}

int16_t cPatternEditor::fillTypeFromDisplayIndex(uint8_t index) const
{
    uint8_t count = 0;
    const uint8_t* types = getFillTypesForColumn(count);
    if (index >= count)
        index = count - 1;
    return types[index];
}

int16_t cPatternEditor::fillValue(const strFill& fill, uint8_t slot) const
{
    if (isGenFillType(fill.type))
    {
        int typeIdx = fill.type - fillTypeWalk;
        int paramIdx = (slot == 2) ? 0 : (slot == 4) ? 1 : 2;
        return fill.genParam[typeIdx][paramIdx];
    }
    return (slot == 2) ? fill.param : (slot == 4) ? fill.from : fill.to;
}

int16_t* cPatternEditor::fillValuePtr(strFill& fill, uint8_t slot, int16_t& min, int16_t& max) const
{
    min = 0;
    max = 0;

    const int16_t type = fill.type;
    const bool gen = isGenFillType(type);
    const bool noteColumn = editParam == EditNote;
    const bool fxColumn = (editParam == EditFx1 || editParam == EditFx2) && !isMicVolView;

    int16_t* ptr = nullptr;
    if (gen)
    {
        int typeIdx = type - fillTypeWalk;
        int paramIdx = (slot == 2) ? 0 : (slot == 4) ? 1 : 2;
        ptr = &fill.genParam[typeIdx][paramIdx];
    }
    else
    {
        ptr = (slot == 2) ? &fill.param : (slot == 4) ? &fill.from : &fill.to;
    }

    if (noteColumn)
    {
        if (slot == 2)
        {
            min = 0;
            max = MAX_SCALE;
        }
        else if (slot == 4)
        {
            if (type == fillTypeTuring)
            {
                min = 1;
                max = mtGen::kMaxRegisterLength;
            }
            else if (type == fillTypeMarkov)
            {
                min = 0;
                max = 1;
            }
            else
            {
                min = 0;
                max = 127;
            }
        }
        else
        {
            if (type == fillTypeTuring)
            {
                min = 0;
                max = 100;
            }
            else if (type == fillTypeMarkov)
            {
                min = 1;
                max = mtGen::kMaxMarkovOrder;
            }
            else
            {
                min = 0;
                max = 127;
            }
        }
    }
    else if (fxColumn)
    {
        if (type == fillTypeProb || type == fillTypeRatchet)
        {
            if (slot == 2)
            {
                min = 0;
                max = 1; // FX1 / FX2 target slot
            }
            else if (slot == 4)
            {
                min = 0;
                max = mtGen::kChanceProbMax;
            }
            else
            {
                if (type == fillTypeRatchet)
                {
                    min = mtGen::kRollPeriodMin;
                    max = mtGen::kRollPeriodMax;
                }
                else
                {
                    min = 0;
                    max = Sequencer::MAXSTEP;
                }
            }
        }
        else
        {
            if (slot == 2)
            {
                // param == -1 means "random FX" in the legacy fill helpers.
                min = -1;
                max = FX_COUNT - 1 - FX_COUNT_HIDDEN_FXes;
            }
            else
            {
                int16_t fxId = interfaceGlobals.fxIDs[clamp<int>(fill.param, 0, FX_COUNT - 1)];
                min = sequencer.getFxMin(fxId);
                max = sequencer.getFxMax(fxId);
            }
        }
    }
    else if (editParam == EditInstr)
    {
        min = 0;
        max = kTotalInstrumentsCount - 1;
    }
    else
    {
        min = 0;
        max = 100;
    }

    return ptr;
}

const char* cPatternEditor::fillValueLabel(int16_t type, uint8_t slot) const
{
    if (slot == 4)
    {
        if (editParam == EditNote)
        {
            if (type == fillTypeTuring)
                return "Length";
            if (type == fillTypeMarkov)
                return "Source";
            if (type == fillTypeConstant)
                return "Note";
            return "From";
        }
        if (editParam == EditInstr)
            return type == fillTypeConstant ? "Instrument" : "From";
        if (editParam == EditFx1 || editParam == EditFx2)
        {
            if (type == fillTypeProb || type == fillTypeRatchet)
                return "Prob %";
            if (isMicVolView)
                return editParam == EditFx1 ? "Micro-move" : "Volume";
            return "Fx Value";
        }
    }
    else if (slot == 5)
    {
        if (editParam == EditNote)
        {
            if (type == fillTypeTuring)
                return "Lock";
            if (type == fillTypeMarkov)
                return "Order";
            return "To";
        }
        if (editParam == EditInstr)
            return "To";
        if (editParam == EditFx1 || editParam == EditFx2)
        {
            if (type == fillTypeProb || type == fillTypeRatchet)
                return "Count";
            if (isMicVolView)
                return editParam == EditFx1 ? "Micro-move" : "Volume";
            return "To";
        }
    }
    return "";
}

const char* cPatternEditor::fillTypeName(int16_t type) const
{
    if (type < 0 || type >= fillTypeCount)
        return "?";
    return &fillTypeListLabels[type][0];
}

void cPatternEditor::showFillAppliedStatus(int16_t type)
{
    char text[20];
    snprintf(text, sizeof(text), "Fill: %s", fillTypeName(type));

    display.setControlText(label[7], text);
    display.setControlText2(label[7], "undo");
    display.setControlValue(label[7], 1);
    display.refreshControl(label[7]);
    display.synchronizeRefresh();
}

//##############################################################################################
//###############################             FILL             #################################
//##############################################################################################

void cPatternEditor::showFillNote()
{
    uint8_t fillIdx = getFillDataIdx();

    // scale
    fillScaleFilterList.linesCount = 13;
    fillScaleFilterList.start = fillValue(fillData[fillIdx], 2);
    fillScaleFilterList.length = MAX_SCALE + 1;
    fillScaleFilterList.data = (char**)(&scaleNames);
    fillScaleFilterList.lineWidthMax = 800 / 8 - 30;
    fillScaleFilterList.lineTextLenMax = 15;

    display.setControlData(param2PopupListControl, &fillScaleFilterList);

    display.setControlValue(label[2], 0);
    display.setControlText(label[2], "Scale");
    display.setControlText2(label[2], "");
}

void cPatternEditor::fillInstrumentNames()
{
    for (int i = 0; i < kSampleInstrumentsCount; i++)
    {
        if (mtProject.instrument[i].isActive)
        {
            strncpy(&interfaceGlobals.intrumentsNames[i][0],
                    mtProject.instrument[i].sample.file_name,
                    SAMPLE_NAME_SIZE);
        }
        else
        {
            strcpy(&interfaceGlobals.intrumentsNames[i][0], "- empty -");
        }

        interfaceGlobals.ptrIntrumentsNames[i] = &interfaceGlobals.intrumentsNames[i][0];
    }
    for (int i = 0; i < kMidiInstrumentsCount; i++)
    {
        sprintf(
          &interfaceGlobals.intrumentsNames[kSampleInstrumentsCount + i][0], "MIDI %d", i + 1);
        interfaceGlobals.ptrIntrumentsNames[kSampleInstrumentsCount + i] =
          &interfaceGlobals.intrumentsNames[kSampleInstrumentsCount + i][0];
    }

#ifdef FEATURE_SYNTH
    for (int i = 0; i < kSynthInstrumentsCount; i++)
    {
        int listPos = i + kMidiInstrumentsCount + kSampleInstrumentsCount;

        strncpy(interfaceGlobals.intrumentsNames[listPos],
                synth->getSynthName(i).c_str(),
                SAMPLE_NAME_SIZE);

        interfaceGlobals.ptrIntrumentsNames[listPos] = interfaceGlobals.intrumentsNames[listPos];
    }
#endif
}

void cPatternEditor::showFillInstr()
{
    // display.setControlHide(param2PopupListControl);
    display.setControlHide(label[2]);

    fillInstrumentNames();

    fillInstrumentsFromList.linesCount = 13;
    fillInstrumentsFromList.start = fillData[editParam].from;
    fillInstrumentsFromList.length = kTotalInstrumentsCount;
    fillInstrumentsFromList.data = interfaceGlobals.ptrIntrumentsNames;
    fillInstrumentsFromList.lineWidthMax = 800 / 8 - 30;
    fillInstrumentsFromList.lineTextLenMax = 15;
    display.setControlPosition(intstrumentsFromPopupListControl, (800 / 8) * (4) + 1, -1);
    display.setControlData(intstrumentsFromPopupListControl, &fillInstrumentsFromList);

    fillInstrumentsToList.linesCount = 13;
    fillInstrumentsToList.start = fillData[editParam].to;
    fillInstrumentsToList.length = kTotalInstrumentsCount;
    fillInstrumentsToList.data = interfaceGlobals.ptrIntrumentsNames;
    fillInstrumentsToList.lineWidthMax = 800 / 8 - 30;
    fillInstrumentsToList.lineTextLenMax = 15;
    display.setControlPosition(intstrumentsToPopupListControl, (800 / 8) * (5) + 1, -1);
    display.setControlData(intstrumentsToPopupListControl, &fillInstrumentsToList);

    display.setControlShow(intstrumentsFromPopupListControl);
    display.setControlHide(val1PopupBar);

    if (fillPlace == 2)
    {
        fillPlace = 0;
    }
}

void cPatternEditor::showFillMicVol()
{
    display.setControlHide(label[2]);
    display.setControlHide(param2PopupListControl);

    if (fillPlace == 2)
    {
        fillPlace = 0;
    }

    refreshFillFrom();
    refreshFillTo();
}

void cPatternEditor::showFillFx()
{
    uint8_t fillIdx = getFillDataIdx();
    int16_t type = fillData[fillIdx].type;

    if (type == fillTypeProb || type == fillTypeRatchet)
    {
        // Prob / Ratchet write a known FX code; slot 2 chooses the target
        // FX column instead of an arbitrary FX type.
        fillFxSlotList.linesCount = fillFxSlotCount;
        fillFxSlotList.start = fillValue(fillData[fillIdx], 2);
        fillFxSlotList.length = fillFxSlotCount;
        fillFxSlotList.data = (char**)(&ptrfillFxSlotNames);
        fillFxSlotList.lineWidthMax = 800 / 8 - 30;
        fillFxSlotList.lineTextLenMax = 5;

        display.setControlData(param2PopupListControl, &fillFxSlotList);

        display.setControlValue(label[2], 0);
        display.setControlText(label[2], "FX");
        display.setControlText2(label[2], "");
    }
    else
    {
        // fx
        fillFxTypeList.linesCount = 13;
        fillFxTypeList.start = fillData[fillIdx].param + 1;
        fillFxTypeList.length = FX_COUNT + 1 - FX_COUNT_HIDDEN_FXes;
        fillFxTypeList.data = (char**)(&interfaceGlobals.ptrAllFxNamesShort);
        fillFxTypeList.lineWidthMax = 800 / 8 - 30;
        fillFxTypeList.lineTextLenMax = 15;

        display.setControlData(param2PopupListControl, &fillFxTypeList);
    }

    refreshFillParam();
    refreshFillFrom();
    refreshFillTo();
}

void cPatternEditor::showFillPopup()
{
    uint8_t fillIdx = getFillDataIdx();

    // Density selectors always open on Events; the alternate values (Rotate /
    // Max jump / Range) are one press of the second column away.
    if (fillStepUsesDensity(fillData[fillIdx].fillStep))
        fillAltActive = fillAltDensity;

    if (fillData[fillIdx].type == 0)
    {
        if (fillPlace > 4)
            fillPlace = 4;
    }
    else
    {
        if (fillPlace > 5)
            fillPlace = 5;
    }
    if (fillData[EditNote].from == 0) // first time open, init to defaults
    {
        fillData[EditNote].from = mtProject.values.padBoardRootNote;
        fillData[EditNote].to = mtProject.values.padBoardRootNote;
        fillData[EditNote].param = mtProject.values.padBoardScale;
    }

    frameData.places[0] = &framesPopupPlaces[0][0];
    frameData.places[1] = &framesPopupPlaces[1][0];
    frameData.places[2] = &framesPopupPlaces[2][0];
    frameData.places[3] = &framesPopupPlaces[3][0];
    frameData.places[4] = &framesPopupPlaces[4][0];
    frameData.places[5] = &framesPopupPlaces[5][0];
    frameData.places[6] = &framesPopupPlaces[6][0];
    frameData.places[7] = &framesPopupPlaces[7][0];

    // step
    fillStepList.linesCount = fillStepCount;
    fillStepList.start = fillData[fillIdx].fillStep;
    fillStepList.length = fillStepCount;
    fillStepList.data = (char**)(&ptrfillStepNames);
    fillStepList.lineWidthMax = 800 / 8 - 30;
    fillStepList.lineTextLenMax = 11;

    display.setControlPosition(param3PopupListControl, (800 / 8) * (0) + 1, -1);
    display.setControlData(param3PopupListControl, &fillStepList);
    display.setControlShow(param3PopupListControl);

    display.setControlValue(label[0], 0);
    display.setControlText(label[0], "Where?");
    display.setControlText2(label[0], "");

    // typ
    uint8_t fillTypeCountForColumn = 0;
    const uint8_t* fillTypeIds = getFillTypesForColumn(fillTypeCountForColumn);
    for (uint8_t i = 0; i < fillTypeCountForColumn; i++)
        ptrFillTypeNamesForColumn[i] = ptrfillTypeListNames[fillTypeIds[i]];

    fillTypeList.linesCount = fillTypeCountForColumn;
    fillTypeList.start = fillTypeToDisplayIndex(fillData[fillIdx].type);
    fillTypeList.length = fillTypeCountForColumn;
    fillTypeList.data = (char**)(&ptrFillTypeNamesForColumn);
    fillTypeList.lineWidthMax = 800 / 8 - 30;
    fillTypeList.lineTextLenMax = 11;

    display.setControlPosition(param1PopupListControl, (800 / 8) * (3) + 1, -1);
    display.setControlData(param1PopupListControl, &fillTypeList);
    display.setControlValue(param1PopupListControl, fillTypeToDisplayIndex(fillData[fillIdx].type));

    display.setControlText(label[3], "Fill Type");
    display.setControlValue(label[3], 0);

    // density
    display.setControlValue(label[1], 1);
    display.setControlText(label[1], "Density");
    display.setControlText2(label[1], "");
    display.setControlPosition(val3PopupBar, (800 / 8) * (1) + 1, -1);
    display.setControlValue(val3PopupBar,
                            fillData[fillIdx].density[fillData[fillIdx].fillStep]);

    // val to/from
    display.setControlPosition(val1PopupBar, (800 / 8) * (4) + 1, -1);
    display.setControlPosition(val2PopupBar, (800 / 8) * (5) + 1, -1);

    // param
    fillEmptyList.linesCount = 13;
    fillEmptyList.start = 0;
    fillEmptyList.length = 0;
    fillEmptyList.data = NULL;
    display.setControlPosition(param2PopupListControl, (800 / 8) * (2) + 1, -1);
    display.setControlData(param2PopupListControl, &fillEmptyList);
    display.setControlValue(param2PopupListControl, -1);

    for (uint8_t i = 0; i < 8; i++)
    {
        display.setControlStyle(label[i],
                                controlStyleCenterX | controlStyleCenterY | controlStyleFont3 |
                                  controlStyleShow);
        display.setControlStyle2(label[i],
                                 controlStyleCenterX | controlStyleCenterY | controlStyleFont2);
        display.setControlColors(label[i], display.getTheme().activeLabelsColors);
        display.refreshControl(label[i]);
    }

    display.setControlShow(val1PopupBar);
    display.setControlShow(val2PopupBar);
    display.setControlHide(intstrumentsFromPopupListControl);
    display.setControlHide(intstrumentsToPopupListControl);

    display.setControlShow(val3PopupBar);
    display.setControlShow(valCancelEmptyPopupBar);
    display.setControlShow(valFillEmptyPopupBar);
    display.setControlShow(param3PopupListControl);
    display.setControlShow(param2PopupListControl);
    display.setControlShow(param1PopupListControl);

    display.setControlValue(label[6], 0);
    display.setControlValue(label[7], 0);
    display.setControlText(label[6], "Cancel");
    display.setControlText2(label[6], "");
    display.setControlText(label[7], "Fill");
    display.setControlText2(label[7], "");

    // pozostale
    switch (editParam)
    {
        case 0:
            showFillNote();
            display.setControlText(patternPopupTitleLabel, " Fill Notes");
            break;
        case 1:
            showFillInstr();
            display.setControlText(patternPopupTitleLabel, " Fill Instruments");
            break;
        case 2:
            if (isMicVolView)
            {
                showFillMicVol();
                display.setControlText(patternPopupTitleLabel, " Fill Micro-move");
            }
            else
            {
                showFillFx();
                display.setControlText(patternPopupTitleLabel, " Fill Fx 1");
            }
            break;
        case 3:
            if (isMicVolView)
            {
                showFillMicVol();
                display.setControlText(patternPopupTitleLabel, " Fill Volume");
            }
            else
            {
                showFillFx();
                display.setControlText(patternPopupTitleLabel, " Fill Fx 2");
            }
            break;

        default: display.setControlText(patternPopupTitleLabel, " Fill"); break;
    }

    display.setControlValue(bgLabel, 255);

    display.refreshControl(val1PopupBar);
    display.refreshControl(val3PopupBar);
    display.refreshControl(valCancelEmptyPopupBar);
    display.refreshControl(valFillEmptyPopupBar);
    display.refreshControl(param3PopupListControl);
    display.refreshControl(param2PopupListControl);
    display.refreshControl(param1PopupListControl);

    display.setControlShow(patternPopupLabel);
    display.refreshControl(patternPopupLabel);
    display.setControlShow(patternPopupTitleLabel);
    display.refreshControl(patternPopupTitleLabel);

    display.refreshControl(bgLabel);

    display.setControlHide(patternControl);
    display.refreshControl(patternControl);

    refreshFillType();
    refreshFillPlace();
    refreshFillDensity();
    activateFillPopupBorder();

    display.synchronizeRefresh();
}

void cPatternEditor::refreshFillType()
{
    uint8_t fillIdx = getFillDataIdx();
    int16_t type = fillData[fillIdx].type;

    display.setControlValue(param1PopupListControl, fillTypeToDisplayIndex(type));
    display.refreshControl(param1PopupListControl);

    fillText1[0] = 0;
    fillText2[0] = 0;

    display.setControlText(label[4], fillValueLabel(type, 4));
    display.setControlText(label[5], fillValueLabel(type, 5));
    display.setControlValue(label[4], 1);
    display.setControlValue(label[5], 1);

    if (type == fillTypeConstant)
    {
        display.setControlValue(val2PopupBar, -1);
        display.setControlText(label[5], "");
        display.setControlText2(label[5], "");
    }

    refreshFillParam();
    refreshFillFrom();
    refreshFillTo();
    refreshFillDensity();

    display.refreshControl(val1PopupBar);
    display.refreshControl(val2PopupBar);

    display.refreshControl(label[2]);
    display.refreshControl(label[3]);
    display.refreshControl(label[4]);
    display.refreshControl(label[5]);
}

void cPatternEditor::refreshFillFrom()
{
    uint8_t fillIdx = getFillDataIdx();
    int16_t type = fillData[fillIdx].type;

    int16_t min = 0;
    int16_t max = 0;
    int16_t* stored = fillValuePtr(fillData[fillIdx], 4, min, max);
    int16_t value = clamp<int>(*stored, min, max);
    *stored = value;

    fillText1[0] = 0;
    display.setControlText(label[4], fillValueLabel(type, 4));

    if (editParam == EditNote)
    {
        if (type == fillTypeTuring)
        {
            sprintf(fillText1, "%d", value);
            display.setControlText2(label[4], fillText1);
            display.setControlValue(
              val1PopupBar, (value - 1) * 100 / (mtGen::kMaxRegisterLength - 1));
        }
        else if (type == fillTypeMarkov)
        {
            display.setControlText2(label[4], value ? "Track" : "Sel");
            display.setControlValue(val1PopupBar, value ? 100 : 0);
        }
        else
        {
            display.setControlText2(label[4], (char*)&mtNotes[value][0]);
            display.setControlValue(val1PopupBar, (value * 100) / (127));
        }
    }
    else if (editParam == EditInstr)
    {
        if (value < kSampleInstrumentsCount)
            sprintf(fillText1, "%d", (value + 1));
        else
            sprintf(fillText1, "%d", (value + 3));
        display.setControlText2(label[4], fillText1);
        display.refreshControl(label[4]);

        display.setControlHide(val1PopupBar);
        display.setControlShow(intstrumentsFromPopupListControl);
        display.setControlValue(intstrumentsFromPopupListControl, value);
        display.refreshControl(intstrumentsFromPopupListControl);
    }
    else if (editParam == EditFx1 || editParam == EditFx2)
    {
        if (type == fillTypeProb || type == fillTypeRatchet)
        {
            sprintf(fillText1, "%d", value);
            display.setControlText2(label[4], fillText1);
            display.setControlValue(val1PopupBar, (value * 100) / mtGen::kChanceProbMax);
        }
        else if (isMicVolView)
        {
            uint8_t fxType = editParam == EditFx1 ? sequencer.fx.FX_TYPE_MICROMOVE
                                                  : sequencer.fx.FX_TYPE_VOLUME;
            sequencer.makeFxValLabelWithAlgin(
              fillText1, fillTextLen, fxType, value, Sequencer::fxValLabelAlign_center);
            display.setControlText2(label[4], fillText1);
            display.setControlValue(val1PopupBar, value);
        }
        else
        {
            sequencer.makeFxValLabelWithAlgin(fillText1,
                                              fillTextLen,
                                              interfaceGlobals.fxIDs[fillData[fillIdx].param],
                                              value,
                                              Sequencer::fxValLabelAlign_center);

            display.setControlText2(label[4], fillText1);

            display.setControlValue(val1PopupBar, ((value - min) * 100) / (max - min));
        }
    }

    display.setControlValue(label[4], 1);
    display.refreshControl(label[4]);
    display.refreshControl(val1PopupBar);
}

void cPatternEditor::refreshFillTo()
{
    uint8_t fillIdx = getFillDataIdx();
    int16_t type = fillData[fillIdx].type;

    fillText2[0] = 0;
    display.setControlText(label[5], fillValueLabel(type, 5));

    if (type == fillTypeConstant)
    {
        display.setControlText(label[5], "");
        display.setControlText2(label[5], "");
        display.refreshControl(label[5]);

        display.setControlValue(val2PopupBar, -1);
        display.refreshControl(val2PopupBar);
        display.setControlData(intstrumentsToPopupListControl, &fillEmptyList);
        display.setControlValue(intstrumentsToPopupListControl, -1);
        display.refreshControl(intstrumentsToPopupListControl);
        return;
    }

    int16_t min = 0;
    int16_t max = 0;
    int16_t* stored = fillValuePtr(fillData[fillIdx], 5, min, max);
    int16_t value = clamp<int>(*stored, min, max);
    *stored = value;

    if (editParam == EditNote)
    {
        if (type == fillTypeTuring)
        {
            sprintf(fillText2, "%d", value);
            display.setControlText2(label[5], fillText2);
            display.setControlValue(val2PopupBar, value);
        }
        else if (type == fillTypeMarkov)
        {
            sprintf(fillText2, "%d", value);
            display.setControlText2(label[5], fillText2);
            display.setControlValue(val2PopupBar,
                                    (value - 1) * 100 / (mtGen::kMaxMarkovOrder - 1));
        }
        else
        {
            display.setControlText2(label[5], (char*)&mtNotes[value][0]);
            display.setControlValue(val2PopupBar, (value * 100) / (127));
        }
        display.refreshControl(label[5]);
    }
    else if (editParam == EditInstr)
    {
        if (value < kSampleInstrumentsCount)
            sprintf(fillText2, "%d", (value + 1));
        else
            sprintf(fillText2, "%d", (value + 3));
        display.setControlText2(label[5], fillText2);
        display.refreshControl(label[5]);

        display.setControlHide(val2PopupBar);
        display.setControlShow(intstrumentsToPopupListControl);
        display.setControlData(intstrumentsToPopupListControl, &fillInstrumentsToList);
        display.setControlValue(intstrumentsToPopupListControl, value);
        display.refreshControl(intstrumentsToPopupListControl);
    }
    else if (editParam == EditFx1 || editParam == EditFx2)
    {
        if (type == fillTypeProb || type == fillTypeRatchet)
        {
            sprintf(fillText2, "%d", value);
            display.setControlText2(label[5], fillText2);
            display.setControlValue(val2PopupBar,
                                    max > min ? ((value - min) * 100) / (max - min) : 0);
            display.refreshControl(label[5]);
        }
        else if (isMicVolView)
        {
            uint8_t fxType = editParam == EditFx1 ? sequencer.fx.FX_TYPE_MICROMOVE
                                                  : sequencer.fx.FX_TYPE_VOLUME;
            sequencer.makeFxValLabelWithAlgin(
              fillText2, fillTextLen, fxType, value, Sequencer::fxValLabelAlign_center);
            display.setControlText2(label[5], fillText2);
            display.setControlValue(val2PopupBar, value);
            display.refreshControl(label[5]);
        }
        else
        {
            sequencer.makeFxValLabelWithAlgin(fillText2,
                                              fillTextLen,
                                              interfaceGlobals.fxIDs[fillData[fillIdx].param],
                                              value,
                                              Sequencer::fxValLabelAlign_center);

            display.setControlText2(label[5], fillText2);

            display.setControlValue(val2PopupBar,
                                    max > min ? ((value - min) * 100) / (max - min) : 0);
            display.refreshControl(label[5]);
        }
    }

    display.setControlValue(label[5], 1);
    display.refreshControl(val2PopupBar);
}

void cPatternEditor::refreshFillParam()
{
    uint8_t fillIdx = getFillDataIdx();
    int16_t type = fillData[fillIdx].type;

    if (editParam == EditNote)
    {
        int16_t min = 0;
        int16_t max = 0;
        int16_t* stored = fillValuePtr(fillData[fillIdx], 2, min, max);
        *stored = clamp<int>(*stored, min, max);

        // Re-point the slot 2 list every time: the type (and therefore the
        // meaning of this slot) can change while the popup is open.
        fillScaleFilterList.linesCount = 13;
        fillScaleFilterList.start = *stored;
        fillScaleFilterList.length = MAX_SCALE + 1;
        fillScaleFilterList.data = (char**)(&scaleNames);
        fillScaleFilterList.lineWidthMax = 800 / 8 - 30;
        fillScaleFilterList.lineTextLenMax = 15;
        display.setControlData(param2PopupListControl, &fillScaleFilterList);

        display.setControlValue(param2PopupListControl, *stored);
        display.setControlShow(param2PopupListControl);
        display.setControlValue(label[2], 1);
        display.setControlText(label[2], "Scale");
        display.setControlText2(label[2], "");
        display.refreshControl(label[2]);
        display.refreshControl(param2PopupListControl);
    }
    else if ((editParam == EditFx1 || editParam == EditFx2) && !isMicVolView)
    {
        if (type == fillTypeProb || type == fillTypeRatchet)
        {
            int16_t min = 0;
            int16_t max = 0;
            int16_t* stored = fillValuePtr(fillData[fillIdx], 2, min, max);
            *stored = clamp<int>(*stored, min, max);

            fillFxSlotList.linesCount = fillFxSlotCount;
            fillFxSlotList.start = *stored;
            fillFxSlotList.length = fillFxSlotCount;
            fillFxSlotList.data = (char**)(&ptrfillFxSlotNames);
            fillFxSlotList.lineWidthMax = 800 / 8 - 30;
            fillFxSlotList.lineTextLenMax = 5;
            display.setControlData(param2PopupListControl, &fillFxSlotList);

            display.setControlValue(param2PopupListControl, *stored);
            display.setControlShow(param2PopupListControl);
            display.refreshControl(param2PopupListControl);

            display.setControlValue(label[2], 1);
            display.setControlText(label[2], "FX");
            display.setControlText2(label[2], *stored == 0 ? "FX1" : "FX2");
            display.refreshControl(label[2]);
        }
        else
        {
            fillFxTypeList.linesCount = 13;
            fillFxTypeList.start = fillData[fillIdx].param + 1;
            fillFxTypeList.length = FX_COUNT + 1 - FX_COUNT_HIDDEN_FXes;
            fillFxTypeList.data = (char**)(&interfaceGlobals.ptrAllFxNamesShort);
            fillFxTypeList.lineWidthMax = 800 / 8 - 30;
            fillFxTypeList.lineTextLenMax = 15;
            display.setControlData(param2PopupListControl, &fillFxTypeList);

            display.setControlValue(param2PopupListControl, fillData[fillIdx].param + 1);
            display.setControlShow(param2PopupListControl);
            display.refreshControl(param2PopupListControl);

            display.setControlValue(label[2], 1);
            display.setControlText(label[2], "Fx Type");
            fxTypeText[0] = interfaceGlobals.ptrAllFxNames[fillData[fillIdx].param + 1][0];
            fxTypeText[1] = 0;
            display.setControlText2(label[2], fxTypeText);
            display.refreshControl(label[2]);

            refreshFillFrom();
            refreshFillTo();
        }
    }
}

void cPatternEditor::refreshFillDensity()
{
    uint8_t fillIdx = getFillDataIdx();
    strFill& fill = fillData[fillIdx];

    if (fillAltSlotActive(fill))
    {
        int16_t min = 0;
        int16_t max = 0;
        int16_t* stored = fillAltValuePtr(fill, min, max);
        int16_t value = clamp<int>(*stored, min, max);
        *stored = value;

        sprintf(fillTextDensity, "%d", value);
        display.setControlText(label[1],
                               fillAltIsRotate(fill) ? "Rotate" : fillAltLabel(fill.type));
        display.setControlText2(label[1], fillTextDensity);
        display.refreshControl(label[1]);
        display.setControlValue(val3PopupBar, max > min ? (value - min) * 100 / (max - min) : 0);
        display.refreshControl(val3PopupBar);
        return;
    }

    const char* labelText = nullptr;
    int value = 0;
    switch (fill.fillStep)
    {
        case Sequencer::fillStepRandom:
            labelText = "Density";
            value = fill.density[fill.fillStep] * 100 / Sequencer::MAX_FILL_DENSITY;
            sprintf(fillTextDensity, "%d %%", value);
            break;
        case Sequencer::fillStepByStep:
            labelText = "Step";
            value = fill.density[fill.fillStep] * 100 / Sequencer::MAX_FILL_DENSITY;
            sprintf(fillTextDensity, "%d", fill.density[fill.fillStep]);
            break;
        case Sequencer::fillStepEuclidean:
        {
            labelText = "Events";
            const int maxEvents = fillDensityMax(fill.fillStep);
            fill.density[fill.fillStep] =
              clamp<int>(fill.density[fill.fillStep], 1, maxEvents);
            value = maxEvents > 1 ? (fill.density[fill.fillStep] - 1) * 100 / (maxEvents - 1)
                                  : 100;
            sprintf(fillTextDensity, "%d", fill.density[fill.fillStep]);
            break;
        }
        default: break;
    }
    if (labelText != nullptr)
    {
        display.setControlText(label[1], labelText);
        display.setControlText2(label[1], fillTextDensity);
        display.refreshControl(label[1]);
        display.setControlValue(val3PopupBar, value);
    }
    else
    {
        display.setControlText(label[1], "");
        display.setControlText2(label[1], "");
        display.refreshControl(label[1]);
        display.setControlValue(val3PopupBar, -1);
    }
    display.refreshControl(val3PopupBar);
}

void cPatternEditor::refreshFillStep()
{
    uint8_t fillIdx = getFillDataIdx();
    //------------------------------
    // step
    // Serial.println(fillStep+2);
    display.setControlValue(param3PopupListControl, fillData[fillIdx].fillStep);
    // display.setControlText(bottomLabel[5], "Step");

    display.refreshControl(param3PopupListControl);

    refreshFillDensity();
}

void cPatternEditor::hideFillPopup()
{
    display.setControlHide(param1PopupListControl);
    display.setControlHide(val1PopupBar);
    display.setControlHide(val2PopupBar);
    display.setControlHide(val3PopupBar);
    display.setControlHide(intstrumentsFromPopupListControl);
    display.setControlHide(intstrumentsToPopupListControl);
    display.setControlHide(valCancelEmptyPopupBar);
    display.setControlHide(valFillEmptyPopupBar);
    display.setControlHide(param3PopupListControl);
    display.setControlHide(param2PopupListControl);

    display.setControlValue(bgLabel, 255);

    display.setControlHide(patternPopupTitleLabel);
    display.setControlHide(patternPopupLabel);
}
//

//##############################################################################################
//###############################                              #################################
//##############################################################################################

void cPatternEditor::activateFillPopupBorder()
{
    if (fillPlace < 0 || fillPlace > frameData.placesCount - 1)
    {
        display.setControlHide(frameControl);
        display.refreshControl(frameControl);
        return;
    }

    display.setControlValue(frameControl, fillPlace);
    display.setControlShow(frameControl);
    display.refreshControl(frameControl);
}

void cPatternEditor::activateLabelsBorder()
{
    if (selectedPlace < 0 || selectedPlace > frameData.placesCount - 1)
    {
        display.setControlHide(frameControl);
        display.refreshControl(frameControl);
        return;
    }

    display.setControlValue(frameControl, selectedPlace);
    display.setControlShow(frameControl);
    display.refreshControl(frameControl);
}

//##############################################################################################
//###############################        MUTE BUTTONS          #################################
//##############################################################################################
void cPatternEditor::showMuteButtons(uint8_t state, bool midiTracks)
{
    if (state)
    {
        for (int i = 0; i < 8; i++)
        {

            mtProject.values.TrackNames[i][7] = 0;
            display.setControlValue(label[i], 1);
            display.setControlStyle(label[i],
                                    controlStyleCenterX | controlStyleCenterY | controlStyleFont3 |
                                      controlStyleShow);
            display.setControlStyle2(label[i],
                                     controlStyleCenterX | controlStyleCenterY | controlStyleFont3);

            char* topText = (char*)"";
            char* bottomText = (char*)"";
            auto colors = display.getTheme().inactiveLabelsColors;

            bool showButton = midiTracks ? i < kMidiTracks : true;
            if (showButton)
            {
                int mute =
                  midiTracks ? mtProject.values.midiTrackMute[i] : mtProject.values.trackMute[i];
                topText =
                  midiTracks ? mtProject.values.midiTrackNames[i] : mtProject.values.TrackNames[i];
                bottomText = mute ? (char*)"Unmute" : (char*)"Mute";
                colors = mute ? display.getTheme().inactiveLabelsColors
                              : display.getTheme().activeLabelsColors;
            }

            display.setControlText(label[i], topText);
            display.setControlText2(label[i], bottomText);
            display.setControlColors(label[i], colors);

            display.refreshControl(label[i]);
        }

        display.synchronizeRefresh();
    }
    else
    {
        showDefaultScreen();
    }
}

//##############################################################################################
//###############################      REC ARMING BUTTONS       #################################
//##############################################################################################
void cPatternEditor::showRecordArmingButtons(uint8_t state, bool midiTracks)
{

    if (state)
    {
        for (int buttonIdx = 0; buttonIdx < 8; buttonIdx++)
        {
            int trackIdx = midiTracks ? buttonIdx + 8 : buttonIdx;

            display.setControlValue(label[buttonIdx], 1);
            display.setControlStyle(label[buttonIdx],
                                    controlStyleCenterX | controlStyleCenterY | controlStyleFont3 |
                                      controlStyleShow);
            display.setControlStyle2(label[buttonIdx],
                                     controlStyleCenterX | controlStyleCenterY | controlStyleFont3);

            char* topText = (char*)"";
            char* bottomText = (char*)"";
            auto colors = display.getTheme().inactiveLabelsColors;

            bool showButton = midiTracks ? buttonIdx < kMidiTracks : true;
            if (showButton)
            {
                topText = (char*)"Live REC";
                bottomText =
                  sequencer.isRecArmedOnTrack(trackIdx) ? (char*)"Armed" : (char*)"Unarmed";

                colors = sequencer.isRecArmedOnTrack(trackIdx)
                           ? display.getTheme().activeLabelsColors
                           : display.getTheme().inactiveLabelsColors;
            }

            display.setControlText(label[buttonIdx], topText);
            display.setControlText2(label[buttonIdx], bottomText);
            display.setControlColors(label[buttonIdx], colors);

            display.refreshControl(label[buttonIdx]);
        }

        display.synchronizeRefresh();
    }
    else
    {
        showDefaultScreen();
    }
}

//##############################################################################################
//###############################                              #################################
//##############################################################################################
void cPatternEditor::activateGreenSelection() //
{
    trackerPattern.selectColor = 1;
}

void cPatternEditor::activateRedSelection()
{
    trackerPattern.selectColor = 0;
}

void cPatternEditor::deactivateSelection()
{
    trackerPattern.selectColor = 2;
}

void cPatternEditor::showKeyboardExport()
{
    titleBar->show(true);
    titleBar->setTitle("Render Selection");

    display.setControlHide(patternControl);
    display.refreshControl(patternControl);

    for (uint8_t i = 0; i < 8; i++)
    {
        display.setControlValue(label[i], 0);
        display.setControlStyle(label[i],
                                controlStyleCenterX | controlStyleCenterY | controlStyleFont3 |
                                  controlStyleShow);
        display.setControlStyle2(label[i],
                                 controlStyleCenterX | controlStyleCenterY | controlStyleFont3);
        display.setControlText(label[i], "");
        display.setControlText2(label[i], "");
        display.setControlColors(label[i], display.getTheme().activeButtonLabelsColors);
    }

    display.setControlText(label[0], "Enter");
    display.setControlText(label[1], "Auto Name");
    display.setControlText(label[5], "Cancel");
    display.setControlText(label[6], "Render");
    display.setControlText2(label[6], "& Load");
    display.setControlStyle2(label[6],
                             controlStyleCenterX | controlStyleCenterY | controlStyleFont3);
    display.setControlText(label[7], "Render");
    display.setControlText2(label[7], "Selection");
    display.setControlStyle2(label[7],
                             controlStyleCenterX | controlStyleCenterY | controlStyleFont3);

    display.setControlValue(label[6], 1);
    display.setControlValue(label[7], 1);

    for (uint8_t i = 0; i < 8; i++)
    {
        display.refreshControl(label[i]);
    }

    display.setControlValue(bgLabel, 255); // 242
    display.refreshControl(bgLabel);

    mtPopups.hideInfoPopup();

    display.synchronizeRefresh();
}

void cPatternEditor::showKeyboardNameEdit(const char* title, bool autoNameEnabled)
{
    titleBar->show(true);
    titleBar->setTitle(title);

    display.setControlHide(patternControl);

    for (uint8_t i = 0; i < 8; i++)
    {
        display.setControlStyle(label[i],
                                controlStyleCenterX | controlStyleFont3 | controlStyleShow);
        display.setControlText(label[i], "");
        display.setControlText2(label[i], "");
        display.setControlColors(label[i], display.getTheme().activeButtonLabelsColors);
    }

    display.setControlText(label[0], "Enter");

    if (autoNameEnabled)
    {
        display.setControlText(label[1], "Auto Name");
    }

    display.setControlText(label[6], "Cancel");
    display.setControlText(label[7], "Save");

    for (uint8_t i = 0; i < 8; i++)
    {
        display.refreshControl(label[i]);
    }

    display.setControlValue(bgLabel, 255);
    display.refreshControl(bgLabel);

    mtPopups.hideInfoPopup();

    display.synchronizeRefresh();
}

void cPatternEditor::hideKeyboardExport()
{
    display.setControlShow(patternControl);
    display.refreshControl(patternControl);
    showDefaultScreen();
    refreshEditState();
    display.synchronizeRefresh();
}

void cPatternEditor::showOverwriteExportDialog()
{
    for (uint8_t i = 0; i < 8; i++)
    {
        display.setControlText(label[i], "");
        display.setControlText2(label[i], "");
    }

    display.setControlValue(label[6], 0);
    display.setControlValue(label[7], 0);
    display.setControlText(label[6], "No");
    display.setControlText(label[7], "Yes");

    for (uint8_t i = 0; i < 8; i++)
    {
        display.refreshControl(label[i]);
    }

    display.setControlValue(bgLabel, 255); // 192
    display.refreshControl(bgLabel);

    mtPopups.showInfoPopup("This name already exists.", "Do you want to overwrite it?");

    display.synchronizeRefresh();
}

void cPatternEditor::hideOverwriteExportDialog()
{
    mtPopups.hideInfoPopup();
    showKeyboardExport();
    display.synchronizeRefresh();
}

void cPatternEditor::showExportProgress()
{
    mtPopups.showProgressPopup("Rendering...");
    keyboardManager.deactivateKeyboard();
    display.synchronizeRefresh();
}
void cPatternEditor::refreshExportProgress()
{
    mtPopups.changePopupProgress(exporter.getProgress());
}
void cPatternEditor::hideExportProgress()
{
    mtPopups.hideProgressPopup();
}
// komunikat ze za malo pamieci w banku zeby zaladowac render
void cPatternEditor::showFullMemoryInBank()
{
    mtPopups.show(2, "There is not enough memory", "in sample bank."); // todo: podzielic
}
// komunikat ze nie ma wolnych instrumentow dla rendera
void cPatternEditor::showFullInstrumentInBank()
{
    mtPopups.show(2, "There are not enough instruments"); // todo: podzielic
}

void cPatternEditor::showActionButtonPage1Labels()
{
    display.setControlText(label[0], "Pattern");
    display.setControlText(label[1], "Length");
    display.setControlText(label[2], "Step jump");
    display.setControlText(label[3], "Fill");
    display.setControlText(label[4], "Preview");
    display.setControlText(label[5], "Undo");
    display.setControlText(label[6], "Redo");
    display.setControlText(label[7], "More");

    display.setControlText2(label[3], "");
    display.setControlText2(label[4], "");
    display.setControlText2(label[5], "");
    display.setControlText2(label[6], "");
    display.setControlText2(label[7], "");

    showPattern(1);
    showLength(1);
    showStep();

    display.setControlColors(label[0], display.getTheme().activeLabelsColors);
    display.setControlColors(label[1], display.getTheme().activeLabelsColors);
    display.setControlColors(label[2], display.getTheme().activeLabelsColors);
    display.setControlColors(label[4], display.getTheme().activeLabelsColors);

    if (editMode)
    {
        display.setControlColors(label[3], display.getTheme().activeLabelsColors);
        display.setControlColors(label[5], display.getTheme().activeButtonLabelsColors);
        display.setControlColors(label[6], display.getTheme().activeLabelsColors);
    }
    else
    {
        display.setControlColors(label[3], display.getTheme().inactiveLabelsColors);
        display.setControlColors(label[5], display.getTheme().inactiveButtonLabelsColors);
        display.setControlColors(label[6], display.getTheme().inactiveLabelsColors);
    }
    display.setControlColors(label[7], display.getTheme().moreButtonColors);

    display.setControlData(label[0], &labelDataGreen);
    display.setControlData(label[1], &labelDataGreen);
    display.setControlData(label[2], &labelDataGreen);
    display.setControlData(label[3], &labelDataRed);
    display.setControlData(label[4], &labelDataGreen);
    display.setControlData(label[5], &labelDataRed);
    display.setControlData(label[6], &labelDataRed);
    display.setControlData(label[7], &labelDataMore);

    labelDataMore.dotIndicator.current = 0;

    for (uint8_t i = 0; i < 8; i++)
    {
        display.setControlValue(label[i], (i > 2 && i < 7) ? 0 : 1);
        display.setControlStyle(
          label[i],
          controlStyleCenterX | controlStyleCenterY | controlStyleFont3 | controlStyleShow |
            //  (i > 2 ? controlStyleUnderline : 0) | // underline
            (i == 7
               ? controlStyleSpecialFx2
               : (display.getTheme().patternEditorBottomButtonsLines ? controlStyleSpecialFx : 0)));
        display.setControlStyle2(label[i],
                                 controlStyleCenterX | controlStyleCenterY | controlStyleFont2);
        display.refreshControl(label[i]);
    }

    display.synchronizeRefresh();
}

void cPatternEditor::showActionButtonPage2Labels()
{
    display.setControlText(label[0], "Render");
    display.setControlText(label[1], "Invert");
    display.setControlText(label[2], "Duplicate");
    display.setControlText(label[3], "Expand");
    display.setControlText(label[4], "Shrink");
    display.setControlText(label[5], "Copy");
    display.setControlText(label[6], "Paste");
    display.setControlText(label[7], "More");

    display.setControlText2(label[0], "Selection");
    display.setControlText2(label[1], "");
    display.setControlText2(label[2], "Pattern");
    display.setControlText2(label[3], "Pattern");
    display.setControlText2(label[4], "Pattern");
    display.setControlText2(label[5], "Pattern");
    display.setControlText2(label[6], "Pattern");
    display.setControlText2(label[7], "");

    if (editMode)
    {
        display.setControlColors(label[0], display.getTheme().activeButtonLabelsColors);
        display.setControlColors(label[1], display.getTheme().activeButtonLabelsColors);
        display.setControlColors(label[2], display.getTheme().activeButtonLabelsColors);
        display.setControlColors(label[3], display.getTheme().activeButtonLabelsColors);
        display.setControlColors(label[4], display.getTheme().activeButtonLabelsColors);
        display.setControlColors(label[5], display.getTheme().activeButtonLabelsColors);
        display.setControlColors(label[6], display.getTheme().activeButtonLabelsColors);
    }
    else
    {
        display.setControlColors(label[0], display.getTheme().inactiveButtonLabelsColors);
        display.setControlColors(label[1], display.getTheme().inactiveButtonLabelsColors);
        display.setControlColors(label[2], display.getTheme().inactiveButtonLabelsColors);
        display.setControlColors(label[3], display.getTheme().inactiveButtonLabelsColors);
        display.setControlColors(label[4], display.getTheme().inactiveButtonLabelsColors);
        display.setControlColors(label[5], display.getTheme().inactiveButtonLabelsColors);
        display.setControlColors(label[6], display.getTheme().inactiveButtonLabelsColors);
    }
    display.setControlColors(label[7], display.getTheme().moreButtonColors);

    for (uint8_t i = 0; i < 8; i++)
    {
        display.setControlValue(label[i], 1);
        display.setControlData(label[i], i == 7 ? &labelDataMore : &labelDataRed);
        display.setControlStyle(
          label[i],
          controlStyleCenterX | controlStyleCenterY | controlStyleFont3 | controlStyleShow |
            //  controlStyleUnderline  | // underline
            (i == 7
               ? controlStyleSpecialFx2
               : (display.getTheme().patternEditorBottomButtonsLines ? controlStyleSpecialFx : 0)));
        display.setControlStyle2(label[i],
                                 controlStyleCenterX | controlStyleCenterY | controlStyleFont3);
        display.refreshControl(label[i]);
    }

    display.setControlValue(label[1], 0);

    labelDataMore.dotIndicator.current = 1;

    display.synchronizeRefresh();
}

void cPatternEditor::showActionButtonPage3Labels()
{
    display.setControlText(label[0], "Pattern");
    display.setControlText(label[1], "Track");
    display.setControlText(label[2], "");
    display.setControlText(label[3], "");
    display.setControlText(label[4], "");
    display.setControlText(label[5], "");
    display.setControlText(label[6], "");
    display.setControlText(label[7], "More");

    display.setControlText2(label[0], "Name");
    display.setControlText2(label[1], "Name");
    display.setControlText2(label[2], "");
    display.setControlText2(label[3], "");
    display.setControlText2(label[4], "");
    display.setControlText2(label[5], "");
    display.setControlText2(label[6], "");
    display.setControlText2(label[7], "");

    if (editMode)
    {
    }
    else
    {
    }

    display.setControlColors(label[0], display.getTheme().activeButtonLabelsColors);
    display.setControlColors(label[1], display.getTheme().activeButtonLabelsColors);
    display.setControlColors(label[7], display.getTheme().moreButtonColors);

    display.setControlData(label[0], &labelDataGreen);
    display.setControlData(label[1], &labelDataGreen);
    display.setControlData(label[7], &labelDataMore);

    for (uint8_t i = 0; i < 8; i++)
    {
        display.setControlValue(label[i], 1);
        display.setControlStyle(
          label[i],
          controlStyleCenterX | controlStyleCenterY | controlStyleFont3 | controlStyleShow |
            (i == 7 ? controlStyleSpecialFx2
                    : (i < 2 ? (display.getTheme().patternEditorBottomButtonsLines
                                  ? controlStyleSpecialFx
                                  : 0)
                             : 0)));
        display.setControlStyle2(label[i],
                                 controlStyleCenterX | controlStyleCenterY | controlStyleFont3);
        display.refreshControl(label[i]);
    }

    labelDataMore.dotIndicator.current = 2;

    display.synchronizeRefresh();
}

void cPatternEditor::showTitleBar()
{
    if (mtConfig.general.patternInfo != 0)
    {
        titleBar->hide();
        return;
    }

    titleBar->show(true);

    showTitleBarPattern();
    showActualParamOnBar(true);
}

void cPatternEditor::showTitleBarPattern()
{
    if (mtConfig.general.patternInfo != 0)
    {
        return;
    }

    char* patternName =
      newFileManager.getPatternsMetadata()->getPatternName(mtProject.values.actualPattern);

    if (patternName == nullptr || patternName[0] == 0)
    {
        snprintf(cTitlePattern,
                 sizeof(cTitlePattern),
                 mtConfig.general.numberingMode == NumberingMode::kHexadecimal ? "Pattern %02X"
                                                                               : "Pattern %d",
                 mtProject.values.actualPattern);
    }
    else
    {
        snprintf(cTitlePattern, sizeof(cTitlePattern), "%s", patternName);
    }

    if (sequencer.isPerformancePatternChanged())
    {
        strncpy(cTitlePattern, "Pattern PERF", 13);
        cTitlePattern[13] = 0;
    }

    titleBar->setTitle(cTitlePattern);
}

void cPatternEditor::showActualParamOnBar(bool forceShow)
{
    if (mtConfig.general.patternInfo != 0)
    {
        return;
    }

    switch (editParam)
    {
        case cPatternEditor::EditNote:
        case cPatternEditor::EditInstr:
        {
            uint8_t idx = mtProject.values.lastUsedInstrument;

            if (lastTitleBarParamValue == idx && lastTitleBarParam <= cPatternEditor::EditInstr &&
                !forceShow)
            {
                return;
            }

            lastTitleBarParamValue = idx;
            lastTitleBarParam = editParam;

            titleBar->showCurrentInstrumentOnRightLabel();
            return;
        }
        case cPatternEditor::EditFx1:
        case cPatternEditor::EditFx2:
        {
            if (isMicVolView)
            {
                uint8_t type_temp = (editParam == cPatternEditor::EditFx1) ? 0xFE : 0xFD;
                if (lastTitleBarParamValue == type_temp && (lastTitleBarParam == editParam) &&
                    !forceShow)
                {
                    return;
                }

                lastTitleBarParamValue = type_temp;
                strncpy(rightTitleBarLabel,
                        editParam == cPatternEditor::EditFx1 ? "Micro-move" : "Volume",
                        35);
                rightTitleBarLabel[35] = 0;
                break;
            }

            const auto& stepFx = sequencer.getPatternToUI()
                                   ->track[trackerPattern.actualTrack]
                                   .step[trackerPattern.actualStep]
                                   .fx[1 - (editParam - 2)];
            uint8_t type_temp = interfaceGlobals.fxIdToName(stepFx.type);
            if (type_temp == 0 || type_temp >= FX_MAX)
            {
                uint8_t lastUsedFx = getPreviousUsedFx();
                if (lastUsedFx < FX_MAX)
                {
                    type_temp = lastUsedFx;
                }
                else
                {
                    type_temp = 0;
                }
            }

            uint8_t fxVal_temp =
              (stepFx.type == sequencer.fx.FX_TYPE_CHANCE) ? stepFx.value : 0xFF;

            if (lastTitleBarParamValue == type_temp && lastTitleBarFx == fxVal_temp &&
                (lastTitleBarParam == editParam) && !forceShow)
            {
                return;
            }

            lastTitleBarParamValue = type_temp;
            lastTitleBarFx = fxVal_temp;

            if (stepFx.type == sequencer.fx.FX_TYPE_CHANCE)
            {
                uint8_t val = stepFx.value;
                if (val <= sequencer.fx.CHANCE_PROB_MAX)
                {
                    snprintf(rightTitleBarLabel, sizeof(rightTitleBarLabel), "Chance: %u%%", val);
                }
                else if (val == sequencer.fx.CHANCE_GROUP_WITH_LAST)
                {
                    strncpy(rightTitleBarLabel, "Chance: Group with Last", 35);
                }
                else if (val == sequencer.fx.CHANCE_NOT_LAST)
                {
                    strncpy(rightTitleBarLabel, "Chance: Not Last", 35);
                }
                else if (val >= sequencer.fx.CHANCE_PLAY_SKIP_START &&
                         val <= sequencer.fx.CHANCE_PLAY_SKIP_END)
                {
                    uint8_t idx = val - sequencer.fx.CHANCE_PLAY_SKIP_START;
                    uint8_t n = 1 + idx / sequencer.fx.CHANCE_CYCLE_STEPS;
                    uint8_t m = 1 + idx % sequencer.fx.CHANCE_CYCLE_STEPS;
                    snprintf(rightTitleBarLabel,
                             sizeof(rightTitleBarLabel),
                             "Chance: Play %u / Skip %u",
                             n,
                             m);
                }
                else if (val >= sequencer.fx.CHANCE_SKIP_PLAY_START &&
                         val <= sequencer.fx.CHANCE_SKIP_PLAY_END)
                {
                    uint8_t idx = val - sequencer.fx.CHANCE_SKIP_PLAY_START;
                    uint8_t n = 1 + idx / sequencer.fx.CHANCE_CYCLE_STEPS;
                    uint8_t m = 1 + idx % sequencer.fx.CHANCE_CYCLE_STEPS;
                    snprintf(rightTitleBarLabel,
                             sizeof(rightTitleBarLabel),
                             "Chance: Skip %u / Play %u",
                             n,
                             m);
                }
            }
            else
            {
                strncpy(rightTitleBarLabel, &interfaceGlobals.fxNames[type_temp][0], 35);
            }
            rightTitleBarLabel[35] = 0;
            break;
        }
    }

    lastTitleBarParam = editParam;

    titleBar->showRightLabel(true);
    titleBar->setRightLabel(rightTitleBarLabel);
}

void cPatternEditor::hideActualParamOnBar()
{
    titleBar->showRightLabel(false);
}

void cPatternEditor::showClockWarning()
{
    for (uint8_t i = 0; i < 8; i++)
    {
        display.setControlText(label[i], "");
        display.setControlText2(label[i], "");

        display.refreshControl(label[i]);
        display.setControlShow(label[i]);
    }

    display.setControlValue(label[6], 0);
    display.setControlValue(label[7], 0);
    display.setControlText(label[6], "Yes");
    display.setControlText(label[7], "Cancel");
    display.setControlColors(label[6], display.getTheme().activeLabelsColors);
    display.setControlColors(label[7], display.getTheme().activeLabelsColors);

    display.refreshControl(label[6]);
    display.refreshControl(label[6]);

    // char currentInfo[100];
    // sprintf(currentInfo,"Start rendering using internal clock source (%.0f BPM)?",
    // mtProject.values.globalTempo);
    mtPopups.showInfoPopup("External clock source is set.",
                           "Start rendering using internal clock source?");

    display.setControlValue(bgLabel, 255); // 192
    display.setControlShow(bgLabel);
    display.refreshControl(bgLabel);

    display.synchronizeRefresh();
}
