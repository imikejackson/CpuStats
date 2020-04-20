#pragma once

#include <cstdint>

#include <list>
#include <vector>


class PowerGui
{
public:
  PowerGui();
  ~PowerGui();

  void run();

public:
  PowerGui(const PowerGui&) = delete;            // Copy Constructor Not Implemented
  PowerGui(PowerGui&&) = delete;                 // Move Constructor Not Implemented
  PowerGui& operator=(const PowerGui&) = delete; // Copy Assignment Not Implemented
  PowerGui& operator=(PowerGui&&) = delete;      // Move Assignment Not Implemented

private:
  const int32_t k_FrameWidth = 100;
  const int32_t k_TopHeader = 1;
  const int32_t k_MidHeader = 1;
  const int32_t k_BottomHeader = 1;

  const int32_t k_CoreOffet = 0;
  const int32_t k_FreqOffset = 6;
  const int32_t k_TempOffset = 16;
  const int32_t k_UtilOffset = 24;
  const int32_t k_BarOffset = 38;
  const int32_t k_BarWidth = 41;

  int32_t m_LastRow = 0;

  int32_t m_NumCpu = 0;
  int32_t m_NumCores = 0;
  double m_BaseClock = 0.0;
  double m_MaxClock = 0.0;

  const int32_t k_Green = 1;
  const int32_t k_Yellow = 2;
  const int32_t k_Red = 3;
  const int32_t k_Blue = 4;

  std::list<double> m_GraphData;
  int32_t m_GraphHeight = 11;
  int32_t m_GraphWidth = 80;

  bool init();

  void cleanup();

  void drawFreqBar(uint64_t sample0, uint64_t sample1);

  void drawCpuFreqUtilTable(uint64_t sample0, uint64_t sample1);

  void drawPackageInfo(uint64_t sample0, uint64_t sample1);

  void getProcessorInfo();

  void drawGraph();
};
