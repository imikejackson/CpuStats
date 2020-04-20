#include "PowerGui.h"

#include <ncurses.h>
#include <unistd.h>

#include <cstdint>
#include <iostream>

#include <IntelPowerGadget/PowerGadgetLib.h>

PowerGui::PowerGui() = default;

PowerGui::~PowerGui() = default;

// -----------------------------------------------------------------------------
bool PowerGui::init()
{
  // Init Graph Data Structure
  for(size_t i = 0; i < m_GraphWidth; i++)
  {
    m_GraphData.push_back(0.0);
  }

  // init ncurses
  initscr();
  // Init Intel Power Gadget
  return PG_Initialize();
}

// -----------------------------------------------------------------------------
void PowerGui::cleanup()
{
  // clean up Intel Power Gadget
  // PG_Shutdown();
  // Clean up ncurses
  endwin();
}

// -----------------------------------------------------------------------------
void PowerGui::getProcessorInfo()
{
  bool err = PG_GetNumPackages(&m_NumCpu);
  std::cout << "numPackages: " << m_NumCpu << std::endl;

  for(int32_t i = 0; i < m_NumCpu; i++)
  {
    int32_t packageCoreCount = 0;
    err = PG_GetNumCores(i, &packageCoreCount);
    m_NumCores += packageCoreCount;
  }
  std::cout << "Total Cores: " << m_NumCores << std::endl;

  err = PG_GetIABaseFrequency(0, &m_BaseClock);
  std::cout << "baseFreq: " << m_BaseClock << std::endl;

  err = PG_GetIAMaxFrequency(0, &m_MaxClock);
  std::cout << "maxFreq: " << m_MaxClock << std::endl;

  m_LastRow = m_NumCores + k_TopHeader + k_MidHeader;
}

// -----------------------------------------------------------------------------
void PowerGui::drawFreqBar(PGSampleID sample0, PGSampleID sample1)
{
  // Get the data
  double mean, min, max;
  bool err = false;

  int32_t startLine = k_TopHeader;
  mvprintw(k_TopHeader, k_UtilOffset, "Utilization ");

  mvprintw(k_TopHeader, k_BarOffset - 1, "|");
  for(int32_t col = k_BarOffset; col < k_BarOffset + k_BarWidth; col++)
  {
    mvprintw(k_TopHeader, col, "-");
  }
  mvprintw(k_TopHeader, k_BarOffset + k_BarWidth, "|");

  startLine++;

  for(int32_t line = startLine; line < startLine + m_NumCores; line++)
  {
    int32_t core = line - startLine;
    err = PGSample_GetIACoreFrequency(sample0, sample1, core, &mean, &min, &max);
    mvprintw(line, k_FreqOffset, "%1.3f GHz", mean / 1000.0);

    double percentOfMax = mean / m_MaxClock;
    int32_t chunks = percentOfMax * k_BarWidth;
    int32_t currentColor = k_Green;
    if(percentOfMax > 0.4)
    {
      currentColor = k_Yellow;
    }
    if(percentOfMax > 0.6)
    {
      currentColor = k_Red;
    }
    attron(COLOR_PAIR(currentColor));
    for(int32_t col = k_BarOffset; col < k_BarOffset + chunks; col++)
    {
      mvprintw(line, col, "|");
    }
    attroff(COLOR_PAIR(currentColor));
  }
}

// -----------------------------------------------------------------------------
void PowerGui::drawCpuFreqUtilTable(PGSampleID sample0, PGSampleID sample1)
{
  // Get the data
  double tempMean, tempMin, tempMax;
  bool err = false;

  int32_t startLine = k_TopHeader;
  mvprintw(k_TopHeader, k_CoreOffet, "----");
  mvprintw(k_TopHeader, k_FreqOffset, "Frequency ");
  mvprintw(k_TopHeader, k_TempOffset, " Temps");
  startLine++;
  double util = 0.0;

  for(int32_t line = startLine; line < startLine + m_NumCores; line++)
  {
    int32_t core = line - startLine;

    mvprintw(line, k_CoreOffet, "#%d", core);

    err = PGSample_GetIACoreTemperature(sample1, core, &tempMean, &tempMin, &tempMax);
    err = PGSample_GetIACoreUtilization(sample0, sample1, core, &util);

    mvprintw(line, k_UtilOffset, "  %03.2f%%", util);

    int32_t currentColor = k_Green;
    if(tempMax > 50)
    {
      currentColor = k_Yellow;
    }
    if(tempMax > 75)
    {
      currentColor = k_Red;
    }
    attron(COLOR_PAIR(currentColor));
    mvprintw(line, k_TempOffset, " %3.0f C", tempMax);
    attroff(COLOR_PAIR(currentColor));
  }
}


// -----------------------------------------------------------------------------
void PowerGui::drawPackageInfo(PGSampleID sample0, PGSampleID sample1)
{
  double powerWatts,energyJoules  = 0.0;
  int32_t offset = 0;
  bool err = PGSample_GetPackagePower(sample0, sample1, &powerWatts, &energyJoules);

  double temp = 0.0;
  err = PGSample_GetPackageTemperature( sample0, &temp);
  attron(COLOR_PAIR(k_Blue));
  mvprintw(m_LastRow + offset, 0, "PKG  | Power %3.2f W", powerWatts);
  mvprintw(m_LastRow + offset, 23, " | Temp %3.2f W", temp);

  offset++;
 
  double mean, min, max;
  err = PGSample_GetIATemperature( sample0, &mean, &min, &max);

  double util = 0.0f;
  err  = PGSample_GetIAUtilization( sample0,  sample1, &util);

  mvprintw(m_LastRow + offset, 0, "Core | Temperature MAX/MIN:%3.0f /%3.0f | Util: %3.2f%%", max, min, util);
  attroff(COLOR_PAIR(k_Blue));

  err = PGSample_GetIAPower(sample0, sample1, &powerWatts, &energyJoules);
  //mvprintw(m_LastRow + offset++, 0, "Core Power: %3.0f Watts  %3.3f Jouls", powerWatts, energyJoules);

  //add the PKG temperature to the Graph
  m_GraphData.pop_front(); // Pop the front one off the graph
  m_GraphData.push_back(temp); // Push a new one on the back
}

// -----------------------------------------------------------------------------
void PowerGui::drawGraph()
{

  std::cout << "Graph size: " << m_GraphData.size() << std::endl;
  int32_t col = 0;
  for(const double& data : m_GraphData)
  {
    int32_t level = static_cast<int32_t>(data/100.0 * 10);

    
    for(int32_t row = 0; row < m_GraphHeight; row++)
    {
      if(level > row) {
        mvprintw(m_LastRow + m_GraphHeight - row, col, "^");
      }
      if(level == row) {
        mvprintw(m_LastRow + m_GraphHeight - row, col, "%1d", level);
      }
    }
    col++;
  }

}


// -----------------------------------------------------------------------------
void PowerGui::run()
{

  bool err = init();

  getProcessorInfo();

  start_color();
  init_pair(k_Green, COLOR_GREEN, COLOR_BLACK);
  init_pair(k_Yellow, COLOR_YELLOW, COLOR_BLACK);
  init_pair(k_Red, COLOR_RED, COLOR_BLACK);
  init_pair(k_Blue, COLOR_CYAN, COLOR_BLACK);

  for(int32_t i = 0; i < 100; i++)
  {
    // Clear the ncurses buffer
    clear();

    mvprintw(0, 0, "Base Clock: %4.2f GHz | Max Clock: %4.2f GHz | Cpu(s): %d | Core(s): %d", m_BaseClock / 1000.0, m_MaxClock / 1000.0, m_NumCpu, m_NumCores);

    // std::cout << "SAMPLE " << i << std::endl;
    PGSampleID sample0;
    PGSampleID sample1;
    err = PG_ReadSample(0, &sample0);
    usleep(100000);
    err = PG_ReadSample(0, &sample1);

    drawCpuFreqUtilTable(sample0, sample1);
    drawFreqBar(sample0, sample1);
    drawPackageInfo(sample0, sample1);
    drawGraph();


    refresh(); // Refresh the screen

    PGSample_Release(sample0);
    PGSample_Release(sample1);

    usleep(1000000);
  }

  cleanup();
}
